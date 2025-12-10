# Convoy Movement Refactor Concept (Updated: Manual Movement + Convoy Pivot Follows Leader)

Goal: align Overworld convoy movement with the working Camp explorer pattern (`AFC_ExplorerCharacter`) so walking animations are driven by **velocity from `AddMovementInput`** (no AI `MoveTo`), while keeping the existing Overworld systems intact—especially **world map revelation**, which relies on the convoy actor’s position.

This doc is updated to reflect current project realities:
- Convoy movement is currently issued via `AAIController::MoveToLocation` inside `AFCPlayerController`.
- Convoy members are currently spawned and **attached** to spawn point components in `AFCOverworldConvoy`.
- The OverworldConvoy Blueprint records visited map locations based on `GetActorLocation(self)`, so the **convoy actor transform is currently treated as the “true convoy position.”**

---

## 0. Beginner mental model (why we’re doing this)

There are two common movement approaches in Unreal:

### A) AI `MoveTo` (current convoy)
- You tell an AI controller “go there.”
- The AI system moves the character.
- It works, but animation and movement debugging can be less predictable because movement isn’t driven by explicit per-frame input.

### B) Manual input movement (Explorer pattern)
- You compute a direction and call `AddMovementInput`.
- CharacterMovement turns that into velocity.
- Animations that blend by velocity/speed behave consistently.

Explorer already works using (B). We want convoy to use the same movement pipeline.

---

## 1. Current setups (baseline)

### 1.1 Camp Explorer (`AFC_ExplorerCharacter`)
- Finds a nav path via `UNavigationSystemV1::FindPathToLocationSynchronously`.
- Stores path state (`PathPoints`, `CurrentPathIndex`, `bIsFollowingPath`).
- In Tick, steers along the path using `AddMovementInput`.

### 1.2 Overworld Convoy (`AFCOverworldConvoy` + `AFCConvoyMember`)

#### `AFCOverworldConvoy`
- Spawns leader + followers and currently attaches them to spawn point components.
- Has POI overlap handling and a “gate” (`bIsInteractingWithPOI`) to prevent duplicate overlaps.
- `StopAllMembers()` currently assumes AI controllers and calls `StopMovement()`.

#### `AFCConvoyMember`
- Inherits `ACharacter`.
- Currently auto-possesses AI (so an AI controller exists).
- Tick is currently not used for manual path following.

#### Important dependency (OverworldConvoy Blueprint Tick)
- The OverworldConvoy Event Graph records visited world locations using **`GetActorLocation(self)`** (with a distance threshold).
- This means many systems implicitly treat the convoy actor as the authoritative “convoy position” (world map, often camera pivot).

**Key gap:** if we move only the leader character and the convoy actor stays still, the world map recording (and any camera anchored to convoy actor) can break.

---

## 2. Target movement architecture

**Goal:** one predictable movement pipeline shared with Explorer, while keeping existing systems stable.

### 2.1 Leader (authoritative walker)
- Only the leader computes a nav path (`FindPathToLocationSynchronously`).
- Leader follows the path in Tick using `AddMovementInput(..., bForce=true)`.

### 2.2 Followers (cheap + animation-friendly)
- Followers do not compute nav paths.
- Followers follow the leader using a simple offset target: `LeaderLocation + FollowerOffset`.
- Followers move via `AddMovementInput(..., bForce=true)` so animations stay correct.

### 2.3 Convoy manager actor stays authoritative for “convoy position”
**Critical decision (Option 1):** `AFCOverworldConvoy` remains the single authoritative transform for:
- world map visited-location recording
- camera pivot / camera attach points
- any future systems that read convoy actor location

To achieve this:
- The leader walks.
- The convoy actor (pivot) follows the leader each tick.

---

## 3. Step-by-step refactor (pre-interaction milestone)

### Step 1 — Stop attaching convoy members to spawn point components
**Change**
- In `AFCOverworldConvoy::SpawnConvoyMembers`, remove `AttachToComponent(...)` for leader and followers.
- Still spawn them at the spawn point world transform.

**Why**
- Characters are meant to move as independent actors with CharacterMovement.
- Attachment can fight movement and makes debugging harder.
- References (`LeaderMember`, `ConvoyMembers`) already give ownership/control.

**Acceptance**
- Members spawn at correct locations.
- Members are not children of spawn point components in the Outliner.

---

### Step 2 — Make `AFCOverworldConvoy` a pivot that follows the center that encloses all convoy members (preserve world map + camera behavior)
**Change**
To position the convoy actor at the center that encloses all convoy members (front/back, left/right bounds), do this in AFCOverworldConvoy::Tick:

Gather all valid convoy members’ world locations.
Calculate their bounding box (FBox) or centroid (average position).
Set convoy actor location to the bounding box center (for full group coverage).
Optionally interpolate smoothly with FMath::VInterpTo.
Example C++ Tick (center based on bounding box):

```cpp
void AFCOverworldConvoy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
 
    if (ConvoyMembers.Num() == 0)
    {
        return;
    }
 
    TArray<FVector> MemberPositions;
    for (auto* Member : ConvoyMembers)
    {
        if (Member && !Member->IsPendingKill())
        {
            MemberPositions.Add(Member->GetActorLocation());
        }
    }
 
    if (MemberPositions.Num() == 0)
    {
        return;
    }
 
    FBox ConvoyBounds(MemberPositions);
 
    FVector DesiredLocation = ConvoyBounds.GetCenter();
 
    // Optional smoothing
    float SmoothSpeed = 5.0f;
    FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, SmoothSpeed);
 
    SetActorLocation(NewLocation);
}
```

**Why**
The bounding box center ensures the convoy actor is positioned at the middle of the whole group.
This allows camera systems attached to the convoy actor to frame the entire convoy’s front/back and side-to-side range.
Smoothing avoids jittery camera movement when members shift quickly.

**Acceptance**

* World map visited-location recording continues to work after refactor.
* Camera/pivot behavior remains correct (convoy actor keeps moving in the world).

---

### Step 3 — Remove AI controller dependency from convoy members

**Change**

* In `AFCConvoyMember`:

  * Set `AutoPossessAI = Disabled`.
  * Ensure BP subclasses have `AIControllerClass = None`.

**Why**

* We want one movement authority (manual Tick path-follow), not AI.
* Removes hidden state and makes movement predictable.

**Acceptance**

* Members no longer spawn AI controllers.
* No code assumes `AAIController` exists.

---

### Step 4 — Ensure unpossessed characters can still move

**Change**
In `AFCConvoyMember` constructor:

* `PrimaryActorTick.bCanEverTick = true;`
* Configure CharacterMovement like Explorer (orient to movement, plane constraints, etc.).
* Enable physics movement without controller:

  * `GetCharacterMovement()->bRunPhysicsWithNoController = true;`

**Why**

* Without AI and without possession, some movement may be ignored.
* This is the common “why won’t AddMovementInput work?” pitfall.

**Acceptance**

* With AI disabled, AddMovementInput still moves convoy members.

---

### Step 5 — Add manual path-follow state + API to `AFCConvoyMember` (leader)

**Change**
Add state:

```cpp
TArray<FVector> PathPoints;
int32 CurrentPathIndex = INDEX_NONE;
bool bIsFollowingPath = false;
float AcceptRadius = 50.0f;
FVector FinalTarget = FVector::ZeroVector; // optional
```

Add API:

```cpp
void MoveConvoyMemberToLocation(const FVector& TargetLocation);
void StopConvoyMovement();
```

**Why**

* Path-follow requires stored state across frames.
* Clean API avoids other systems mutating internals.

**Acceptance**

* Leader can start/stop path-follow via clear methods.

---

### Step 6 — Implement Tick-based manual following (leader path + follower offset)

Highlights & Example Snippet for Manual Follow in Tick:

```cpp
void AFCConvoyMember::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsFollowingPath && PathPoints.IsValidIndex(CurrentPathIndex))
    {
        FVector Target = PathPoints[CurrentPathIndex];
        FVector ToTarget = Target - GetActorLocation();
        ToTarget.Z = 0;

        if (ToTarget.Size() < AcceptRadius)
        {
            CurrentPathIndex++;
            if (!PathPoints.IsValidIndex(CurrentPathIndex))
            {
                bIsFollowingPath = false;
                StopConvoyMovement();
            }
        }
        else
        {
            FVector Direction = ToTarget.GetSafeNormal();
            AddMovementInput(Direction, 1.0f, true); // bForce=true important!
        }
    }
    else if (bIsFollowingLeader && Leader)
    {
        FVector DesiredPos = Leader->GetActorLocation() + FollowerOffset;
        FVector ToLeader = DesiredPos - GetActorLocation();
        ToLeader.Z = 0;

        if (ToLeader.Size() > AcceptRadius)
        {
            FVector Direction = ToLeader.GetSafeNormal();
            AddMovementInput(Direction, 1.0f, true);
        }
    }
}
```

**Why**

* This makes movement input explicit and predictable.
* Animations receive meaningful velocity because CharacterMovement is driven by input.

**Acceptance**

* Leader walks with correct animation blending.
* Followers trail behind, also animated correctly.

---

### Step 7 — Expose convoy-level movement APIs (single entry point)

**Change**
In `AFCOverworldConvoy`, add:

```cpp
void MoveConvoyToLocation(const FVector& TargetLocation);
void StopConvoy();
```

Implementation:

* `MoveConvoyToLocation` calls `LeaderMember->MoveConvoyMemberToLocation(TargetLocation)`.
* `StopConvoy` calls `StopConvoyMovement` on all members.

**Why**

* Centralizes movement commands.
* Prevents scattered control and makes future networking easier.

**Acceptance**

* All movement requests go through `AFCOverworldConvoy`.

---

### Step 8 — Remove AI `MoveTo` usage from `AFCPlayerController` (critical)

**Change**

* Replace AI `MoveToLocation` usage in:

  * `HandleOverworldClickMove()`
  * `MoveConvoyToLocation()`
* With calls to:

  * `ActiveConvoy->MoveConvoyToLocation(TargetLocation)`

**Why**

* If PlayerController keeps issuing AI MoveTo, convoy remains AI-driven and refactor never takes effect.

**Acceptance**

* No convoy-related AI MoveTo remains in PlayerController.
* Clicking on overworld moves convoy via manual leader path-follow.

---

### Step 9 — Update POI overlap + stop logic to use manual stop

**Change**

* Replace `StopAllMembers()` internals (AI stop) with `StopConvoy()` (manual stop).
* In `HandlePOIOverlap`, call `StopConvoy()`.

**Why**

* Overlap should stop path-follow state reliably without AI.
* Keeps interaction gating stable.

**Acceptance**

* POI overlap stops convoy immediately and triggers existing overlap event flow.

---

## 4. Best Practices & Pitfalls to Avoid (keep these rules)

* Always use `AddMovementInput(..., bForce=true)` when unpossessed to eliminate ignored input.
* Avoid attaching characters to spawn points; use spawn location only.
* Keep leader’s path computed via NavMesh; followers follow leader + offset (no extra nav).
* Centralize movement commands through convoy manager API to avoid scattered movement control.
* Use weak pointers or careful lifetime management for leader references in followers.
* Enable ticking on convoy members and configure CharacterMovement properly (orient to movement, plane constraints).

---

## 5. Documentation & downstream updates

After completion:

* Document that convoy movement uses manual nav path-follow + `AddMovementInput`, no AI `MoveTo`.
* Document that `AFCOverworldConvoy` remains the authoritative “convoy position” and follows the leader as a pivot.
* Update interaction refactor tasks to request movement only via `AFCOverworldConvoy::MoveConvoyToLocation`.


RESOLVED ISSUE:
Your logs show the **leader is receiving move commands** (click → `HandleOverworldClickMove` → convoy “Moving to location …”), and the convoy can stop everyone on POI overlap. What’s missing is any evidence that the **followers are being driven** (no “start following leader”, no follower Tick movement, no follower target updates).

That almost always boils down to this:

1. **Only the leader has movement logic / gets a destination**, and
2. **Followers are never put into “follow leader” mode**, *or* they’re still **attached to a spawn point / convoy actor** so they’re not actually walking (and if the parent isn’t moving, they’ll look stuck).

### The fix: implement follower-follow and activate it when spawning

You need two pieces:

* A “follow leader” API on `AFCConvoyMember`
* The convoy spawner calling it for follower1/follower2

Below is a clean, minimal implementation that works with your concept (“leader path-follow, followers offset-follow”).

---

## 1) Add follower follow API + state to `AFCConvoyMember`

### `FCConvoyMember.h`

Add this (public + private). Keep your overlap code as-is.

```cpp
// Public API
UFUNCTION(BlueprintCallable, Category="FC|Convoy")
void StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset);

UFUNCTION(BlueprintCallable, Category="FC|Convoy")
void StopFollowingLeader();
```

And add this state:

```cpp
private:
    bool bIsFollowingLeader = false;

    UPROPERTY()
    TWeakObjectPtr<AFCConvoyMember> Leader;

    // Offset relative to leader (local space)
    FVector FollowerLocalOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category="FC|Convoy")
    float FollowAcceptRadius = 80.0f;
```

Why local offset? Because it lets the formation rotate naturally with the leader’s facing.

---

## 2) Implement it in `FCConvoyMember.cpp`

```cpp
void AFCConvoyMember::StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset)
{
    Leader = InLeader;
    FollowerLocalOffset = InLocalOffset;

    bIsFollowingLeader = Leader.IsValid();

    // Followers should not run their own nav path
    bIsFollowingPath = false;
    PathPoints.Reset();
    CurrentPathIndex = INDEX_NONE;

    UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Following leader %s Offset=%s"),
        *GetName(),
        Leader.IsValid() ? *Leader->GetName() : TEXT("null"),
        *FollowerLocalOffset.ToString());
}

void AFCConvoyMember::StopFollowingLeader()
{
    bIsFollowingLeader = false;
    Leader = nullptr;
}
```

Then in your `Tick`, add (or ensure you have) this follower block:

```cpp
if (bIsFollowingLeader && Leader.IsValid())
{
    // Rotate offset by leader rotation so the formation turns with the leader
    const FVector DesiredPos =
        Leader->GetActorLocation() + Leader->GetActorRotation().RotateVector(FollowerLocalOffset);

    FVector ToDesired = DesiredPos - GetActorLocation();
    ToDesired.Z = 0.0f;

    if (ToDesired.Size() > FollowAcceptRadius)
    {
        AddMovementInput(ToDesired.GetSafeNormal(), 1.0f, /*bForce=*/true);
    }
}
```

Also make sure you have:

* `GetCharacterMovement()->bRunPhysicsWithNoController = true;`
* `AddMovementInput(..., true)` (bForce)

Otherwise unpossessed followers often ignore input.

---

## 3) Activate follower follow when spawning the convoy

In `AFCOverworldConvoy::SpawnConvoyMembers()` after spawning:

* **Remove attachments** (`AttachToComponent`).
  If followers are attached, they won’t “walk” correctly (and if the parent doesn’t move, they’ll appear stuck).

* Then:

```cpp
// After Leader is spawned:
LeaderMember = Leader;

// Example offsets: behind leader in local space
if (Follower1)
{
    Follower1->StartFollowingLeader(LeaderMember, FVector(-150.f, 0.f, 0.f));
}

if (Follower2)
{
    Follower2->StartFollowingLeader(LeaderMember, FVector(-300.f, 0.f, 0.f));
}
```

If you want to preserve your formation exactly as defined by the spawn points:

* compute offsets from the spawn points **relative** to the leader spawn point (local convoy space), then treat that as “local offset”.

---

## 4) One more check that matches your “pivot follows leader” choice

If your convoy actor is the pivot (camera + world map logic), ensure it actually follows the leader each tick. If the pivot doesn’t move, followers attached to pivot won’t move either (and your world-map recording won’t update).

---

## Why the logs fit this diagnosis

Your logs show:

* Move requests are happening (`Moving convoy to …`).
* Stops work on POI overlap (all members get “Stopped member …”).
* But nothing indicates followers were ever told *how to move* (no follow activation, no follower movement events).

Once you add `StartFollowingLeader` and call it during spawning, you should immediately see your new log lines for follower setup—and visually, followers will start moving.