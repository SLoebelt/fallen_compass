# Convoy Movement Refactor Concept (Pre-Interaction 0003)

Goal: align Overworld convoy movement with the working Camp explorer pattern (`AFC_ExplorerCharacter`) so walking animations drive from `AddMovementInput` (no AI controllers), and provide a clean foundation for later interaction/arrival-gated work.

---

## 1. Current setups (baseline)

### 1.1 Camp Explorer (`AFC_ExplorerCharacter`)

- Inherits `ACharacter`, **no AI controller**:
  - `AutoPossessPlayer = EAutoReceiveInput::Player0;`
  - `AIControllerClass = nullptr;`
- `UCharacterMovementComponent` configured for top-down:
  - `bOrientRotationToMovement = true;`
  - `bUseControllerDesiredRotation = false;`
  - `bConstrainToPlane = true;`
  - `bSnapToPlaneAtStart = true;`
  - `MaxWalkSpeed = 300.0f`.
- Movement is **manual, pawn-driven**:
  - `MoveExplorerToLocation(const FVector& TargetLocation)` calls `UNavigationSystemV1::FindPathToLocationSynchronously`.
  - Stores the resulting path in `PathPoints`, tracks `CurrentPathIndex` and `bIsFollowingPath`.
  - On `Tick`, if following:
    - Compute `ToTarget = PathPoints[CurrentPathIndex] - GetActorLocation()` (Z flattened).
    - If `Distance > AcceptRadius` → `AddMovementInput(Direction, 1.0f)`.
    - Else advance `CurrentPathIndex` or finish.
  - **No AI MoveTo**; animations blend correctly from velocity driven by input.

### 1.2 Overworld Convoy (`AFCOverworldConvoy` + `AFCConvoyMember`)

#### `AFCOverworldConvoy`

- Pure `AActor` (not a character).
- Owns a component hierarchy:
  - Root + `CameraAttachPoint` + `LeaderSpawnPoint` + `Follower1SpawnPoint` + `Follower2SpawnPoint`.
- Spawns three `AFCConvoyMember` instances at the spawn points and attaches them.
- Tracks:
  - `TArray<AFCConvoyMember*> ConvoyMembers;`
  - `AFCConvoyMember* LeaderMember;`
  - `bool bIsInteractingWithPOI;`.
- Movement-related API:
  - `StopAllMembers()` currently assumes AI:
    - For each member, casts `Member->GetController()` to `AAIController` and calls `StopMovement()`.
  - `HandlePOIOverlap(AActor* POIActor)`:
    - Guards double triggers via `bIsInteractingWithPOI`.
    - Calls `StopAllMembers()` and broadcasts `OnConvoyPOIOverlap`.

#### `AFCConvoyMember`

- Inherits `ACharacter`.
- Configures capsule + overlap events.
- Explicit AI:
  - `AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;`
- No explicit movement API yet; convoy locomotion is implied to be AI-driven elsewhere (likely via `AAIController::MoveTo`), which explains broken animation compared to Explorer.

**Key gap:**

- Explorer: manual `AddMovementInput` + internal path state → smooth, predictable animations.
- Convoy: AI-driven characters stopped via `AAIController::StopMovement()` → animations not tied to explicit input and harder to reason about.

To match Explorer behavior and prepare for interaction 0003, convoy movement should adopt a **manual path-follow driver** (no AI-dependency) and a single clear movement authority.

---

## 2. Target movement architecture for convoy

**Goal:** single predictable movement pipeline, shared concepts with Explorer.

- Exactly **one authoritative path follower** for the convoy leader:
  - Pathfinding via `UNavigationSystemV1::FindPathToLocationSynchronously`.
  - Path state stored on the **leader character** (not an AI controller).
  - `Tick` (or timer) drives `AddMovementInput` along the path.
- Followers:
  - Either simple **offset-follow** behind the leader using manual movement, or
  - Purely visual children of a leader/convoy root (if we choose ultra-simple later).
- `AFCOverworldConvoy` responsibilities:
  - Spawner + holder of references (leader + followers).
  - Small API surface for `MoveConvoyToLocation(Target)` and `StopConvoy()`.
  - No AI-specific logic; assumes members manage their own movement using character movement.

---

## 3. Step-by-step refactor (pre-interaction)

Treat this as a mini-milestone to complete before applying the interaction/arrival-gated refactor.

### Step 1 — Remove AI controller dependency from convoy members

**1.1 In `AFCConvoyMember`**

- Disable AI auto-possession:
  - `AutoPossessAI = EAutoPossessAI::Disabled;` (or remove explicit AI auto-possession).
- Ensure Blueprint subclasses:
  - Do **not** assign an `AIControllerClass`, or set it to `nullptr`.

**1.2 In `AFCOverworldConvoy::StopAllMembers()`**

- Stop assuming `AAIController`.
- Replace AI-based `StopMovement()` with a character-side API, e.g.:

  ```cpp
  void AFCConvoyMember::StopConvoyMovement();
  ```

- Implementation sketch:
  - Clear `bIsFollowingPath` and path indices on each member.
  - Optionally zero accumulated input or use `GetCharacterMovement()->StopMovementImmediately()`.

**Acceptance:**

- In PIE, convoy members should no longer rely on AI controllers for movement.
- Calling `StopAllMembers()` should not crash or assume an `AAIController` exists.

---

### Step 2 — Design shared path-follow state on convoy members

We can later factor this into a reusable component; for now, keep it simple on `AFCConvoyMember`.

**2.1 State fields (leader + optionally followers)**

Add (private):

```cpp
TArray<FVector> PathPoints;          // Nav path points
int32 CurrentPathIndex = INDEX_NONE; // Current waypoint index
bool bIsFollowingPath = false;       // Are we actively walking a path?
float AcceptRadius = 50.0f;          // Distance to advance/finish
```

**2.2 Minimal movement API**

```cpp
void MoveConvoyMemberToLocation(const FVector& TargetLocation);
void StopConvoyMovement();
bool IsMoving() const; // optional helper
```

**2.3 Implementation sketch (mirror Explorer)**

- In `MoveConvoyMemberToLocation`:
  - Obtain `UWorld*` and `UNavigationSystemV1*`.
  - Call `FindPathToLocationSynchronously(World, StartLocation, TargetLocation, this)`.
  - If no valid path → log and call `StopConvoyMovement()`.
  - Else:
    - Fill `PathPoints` from `NavPath->PathPoints`.
    - Set `CurrentPathIndex = 0; bIsFollowingPath = true;`.

- In `StopConvoyMovement`:
  - Set `bIsFollowingPath = false; CurrentPathIndex = INDEX_NONE;`.
  - Optionally clear `PathPoints` and call `GetCharacterMovement()->StopMovementImmediately()`.

---

### Step 3 — Implement Tick-based path following on the leader

**3.1 Enable Tick + configure movement**

- In `AFCConvoyMember` constructor:
  - `PrimaryActorTick.bCanEverTick = true;`
  - Configure `UCharacterMovementComponent` like Explorer:
    - `bOrientRotationToMovement = true;`
    - `bUseControllerDesiredRotation = false;`
    - `bConstrainToPlane = true;`
    - `bSnapToPlaneAtStart = true;`
    - `MaxWalkSpeed` tuned for convoy speed.

**3.2 Tick implementation sketch**

```cpp
void AFCConvoyMember::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsFollowingPath || !PathPoints.IsValidIndex(CurrentPathIndex))
    {
        return;
    }

    const FVector Target = PathPoints[CurrentPathIndex];
    FVector ToTarget = Target - GetActorLocation();
    ToTarget.Z = 0.0f;

    const float Distance = ToTarget.Size();
    if (Distance <= AcceptRadius)
    {
        ++CurrentPathIndex;
        if (!PathPoints.IsValidIndex(CurrentPathIndex))
        {
            StopConvoyMovement();
            // (Later) broadcast arrival event to convoy/interaction
            return;
        }
    }
    else
    {
        const FVector Direction = ToTarget.GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}
```

**Effect:**

- Leader locomotion becomes Explorer-style: animations see character velocity from `AddMovementInput`, not opaque AI moves.

---

### Step 4 — Decide and implement follower movement strategy

Two options; we can start with the simplest and evolve later.

#### 4.1 Option A — Static slot followers (no independent path)

- Keep Follower1/Follower2 as children of sockets on the leader or convoy root.
- Only the leader uses path-follow logic; followers visually “ride along”.

**Pros:**

- Very simple, minimal runtime cost.
- Only one path is computed and followed.

**Cons:**

- Less organic following behavior; all members effectively share the same root transform.

#### 4.2 Option B — Offset-follow via manual movement (no AI)

- Each follower tracks a desired offset from the leader (world-space or local-space):

  ```cpp
  FVector FollowOffset; // configured per member (e.g. -150, 0, 0)
  ```

- On Tick, followers compute:

  ```cpp
  const FVector DesiredLocation = Leader->GetActorLocation() + FollowOffset;
  const FVector ToTarget = DesiredLocation - GetActorLocation();
  // If within small radius, do nothing; otherwise AddMovementInput()
  ```

- No extra nav queries; we rely on the leader’s path being nav-valid.
- Followers still drive animations via `AddMovementInput`.

**Recommended starting point:**

- Make **only the leader** own a nav path.
- Use **simple offset-follow** for followers if static sockets are too visually rigid.

---

### Step 5 — Expose convoy-level movement APIs

**5.1 In `AFCOverworldConvoy`**

Add:

```cpp
UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
void MoveConvoyToLocation(const FVector& TargetLocation);

UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
void StopConvoy();
```

**5.2 Implement convoy methods**

- `MoveConvoyToLocation`:
  - If `LeaderMember` is valid, call `LeaderMember->MoveConvoyMemberToLocation(TargetLocation);`.
  - Followers either:
    - Rely on offset-follow logic, or
    - (Optional) receive the same target if we later add per-member paths.

- `StopConvoy`:
  - Iterate `ConvoyMembers` and call `StopConvoyMovement()`.
  - Replace the internals of `StopAllMembers()` with a call to `StopConvoy()` or remove `StopAllMembers` entirely.

**5.3 Update call sites**

- Any existing logic (e.g. in `AFCPlayerController` or `UFCInteractionComponent`) that used AI-specific movement should now call:
  - `OverworldConvoy->MoveConvoyToLocation(TargetLocation);`
  - `OverworldConvoy->StopConvoy();`

This establishes `AFCOverworldConvoy` as the **single entry point** for convoy movement, backed by Explorer-style movement under the hood.

---

### Step 6 — Update POI/interaction glue to use the new movement

**6.1 Within `AFCOverworldConvoy`**

- Keep `HandlePOIOverlap` as the central place that:
  - Receives overlaps from any `AFCConvoyMember`.
  - Calls `StopConvoy()` instead of AI `StopMovement()`.
  - Sets `bIsInteractingWithPOI` and broadcasts `OnConvoyPOIOverlap`.

**6.2 Within `UFCInteractionComponent` (later 0003 work)**

- For Overworld clicks, use `MoveConvoyToLocation` when the player selects a POI.
- Listen to convoy events (or new `NotifyArrivedAtPOI`) to transition from MovingToPOI → Arrived → Executing.

For this concept, the crucial part is that movement semantics now match Explorer; the interaction refactor simply observes these events instead of owning two different movement patterns.

---

## 4. Best practices & pitfalls (UE-aligned)

These notes summarise UE idioms and gotchas relevant to this refactor.

### 4.1 Strengths of this design

- **No AI controller dependency:** manual movement is clearer, easier to debug, and better aligned with UE’s standard for direct character control.
- **Single movement authority & state ownership:** all path state and movement logic live on the leader character (and its followers), not on transient controllers.
- **Tick-driven path following:** matching `AFC_ExplorerCharacter` ensures consistent animation response and avoids opaque AI state.
- **Consistent movement component setup:** matching `UCharacterMovementComponent` settings between explorer and convoy members gives a coherent feel across scenes.
- **Clear API surface:** `MoveConvoyToLocation` / `StopConvoy` make it obvious where convoy movement is controlled.

### 4.2 Pitfalls to watch for

- **Tick cost:**
  - Multiple followers ticking can be costly in large convoys.
  - Mitigations:
    - Disable Tick on inactive followers.
    - Use simple distance thresholds to skip tiny adjustments.
    - Consider timer-based updates or lower Tick frequency if needed.

- **Path validity & failures:**
  - Always handle invalid paths in `MoveConvoyMemberToLocation`:
    - Log and call `StopConvoyMovement()` on failure.
  - Avoid leaving `bIsFollowingPath` true with an empty `PathPoints`.

- **Follower collisions & spacing:**
  - Offset-following may require minimal collision handling if followers bump into each other.
  - Start simple; add steering/avoidance only if gameplay demands it.

- **Mixed movement authority:**
  - After this refactor, no other system should directly drive convoy member movement (AI controllers, random `MoveTo`, etc.).
  - Centralise all calls through `AFCOverworldConvoy` APIs.

- **Replication (if/when multiplayer):**
  - Replicate simple state (current path index, target) rather than re-running nav on every client.
  - Keep AI controllers out of the equation to simplify networked behavior.

---

## 5. Documentation & downstream updates

Once convoy movement has been refactored:

- Update relevant technical docs (Convoy + Explorer + `FCRuntime.md`) to state:
  - Explorer and convoy leader both use **manual NavMesh path-following with `AddMovementInput`**.
  - AI controllers are no longer part of overworld movement.
- Adjust the interaction refactor tasks (`0003-tasks_refactoring_interaction_predictability.md`):
  - Movement assumptions for Overworld should reference `MoveConvoyToLocation` (not AI `MoveTo`).
  - Make it explicit that `AFCOverworldConvoy` is the **single convoy movement authority**, and `UFCInteractionComponent` requests movement only through this API.

This keeps the movement layer clean and predictable before layering on the 0003 interaction/arrival-gated behavior.

---

## 6. Optional robustness extensions (for scaling & multiplayer)

The core refactor above is intentionally minimal. The following extensions can be layered on top to improve feel, robustness, and scalability as the project grows.

### 6.1 Movement smoothing & interpolation

**Leader and followers** can use interpolation to avoid abrupt direction or speed changes:

- For followers using offset-follow, instead of instantly snapping towards `Leader + FollowOffset`, interpolate:

  ```cpp
  FVector CurrentLocation = GetActorLocation();
  FVector TargetLocation  = Leader->GetActorLocation() + FollowOffset;
  const float SmoothSpeed = 4.0f; // tune per project

  SetActorLocation(FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, SmoothSpeed));
  ```

- For the leader, you can smooth rotation or speed changes using `FMath::RInterpTo` or speed-based lerps, while still driving motion via `AddMovementInput`.

**Benefit:** more natural convoy motion and less jitter when targets or offsets change.

### 6.2 Dynamic path recalculation

If the leader becomes stuck or the path is invalidated (dynamic obstacles, nav mesh changes):

- Track a simple "stuck" timer or distance moved over time.
- If the character has not advanced meaningfully towards `PathPoints[CurrentPathIndex]` for a configurable interval, attempt to **recompute the path**:
  - Call `FindPathToLocationSynchronously` again with the current location and original target.
- Optionally expose this as a configurable behavior (e.g., max retries, cooldown).

**Benefit:** convoy recovers from dynamic nav issues without manual intervention.

### 6.3 Movement lifecycle events

Expose multicast delegates on the leader (and optionally followers) to provide hooks for other systems:

- `FOnConvoyPathStarted`
- `FOnConvoyPathCompleted`
- `FOnConvoyMovementStopped`
- `FOnConvoyArrivedAtPOI`

These can be broadcast from `MoveConvoyMemberToLocation`, `StopConvoyMovement`, and the point where the leader reaches its final waypoint.

**Benefit:** simplifies integration with `UFCInteractionComponent`, logging, debugging, and future systems without tight coupling.

### 6.4 Tick optimisation

To keep Tick overhead under control, especially with many followers:

- Use `PrimaryActorTick.TickGroup` and `PrimaryActorTick.TickInterval` to move less-critical follower updates into a later group or lower frequency.
- Disable Tick on followers when:
  - The convoy is idle (`bIsFollowingPath == false`), or
  - The follower is already within a small radius of its desired offset.
- Consider a shared manager or timer-driven updates for large convoys.

**Benefit:** preserves performance as convoy size and scene complexity grow.

### 6.5 Replication strategy (for multiplayer)

If/when Overworld convoy movement becomes networked:

- Replicate minimal movement state from the leader:
  - `bIsFollowingPath`
  - `CurrentPathIndex`
  - Current target or final destination
- Use `RepNotify` to trigger client-side interpolation or re-creation of local steering.
- Keep `UCharacterMovementComponent` configuration uniform and leverage UE's network smoothing settings.

**Benefit:** keeps replicated movement smooth and predictable without re-running full nav logic on every client.

### 6.6 Movement failure handling & fallbacks

Add explicit failure handling for:

- Empty/invalid paths (already partially covered in Step 2).
- Stuck characters (no progress over time).
- Nav failures (null `UNavigationSystemV1`, map misconfigurations).

On failure:

- Call `StopConvoyMovement()`.
- Log a clear warning with context (target, map, member name).
- Optionally trigger fallback behaviour (e.g., idle animation, UI hint, or a single retry).

**Benefit:** more robust behavior in edge cases and clearer diagnostics when level/nav setup is wrong.

### 6.7 Movement mode tagging (for animation & logic)

Introduce a small enum or `FGameplayTag` to represent high-level movement modes for convoy members, e.g.:

- `Idle`
- `FollowingLeader`
- `Stopped`
- `ArrivedAtPOI`

Update this mode from the movement code (`MoveConvoyMemberToLocation`, `StopConvoyMovement`, arrival handling) and feed it into animation blueprints or higher-level logic.

**Benefit:** clearer integration between movement, animation state machines, and debugging tools.

### 6.8 Documented extension points for steering/AI

Even though AI controllers are removed from the core path, we can explicitly document where steering behaviours or more advanced logic could plug in later:

- A future `UFCConvoySteeringComponent` that applies obstacle avoidance forces before calling `AddMovementInput`.
- Additional movement modes (e.g. flee, escort) layered on top of the same path-follow base.

**Benefit:** keeps the design open for future gameplay additions without reintroducing AI controller complexity into basic convoy locomotion.