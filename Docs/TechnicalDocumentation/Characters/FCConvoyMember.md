## AFCConvoyMember — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `Characters/Convoy/FCConvoyMember.h` 
* **Source:** `Characters/Convoy/FCConvoyMember.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFCConvoyMember`** is the base class for all **Overworld convoy member characters**. It owns capsule overlap detection, forwarding POI events to its parent convoy, and the **actual movement behavior** for the convoy (leader path-follow + follower formation).

1. **Convoy membership wiring**

   * Stores a pointer to its parent `AFCOverworldConvoy` in `ParentConvoy`.
   * Exposes `SetParentConvoy(AFCOverworldConvoy* InConvoy)` so the convoy root can assign itself after spawning members.

2. **POI overlap detection**

   * Binds the capsule component’s `OnComponentBeginOverlap` to `OnCapsuleBeginOverlap` in `BeginPlay`.
   * In `OnCapsuleBeginOverlap`, when overlapping a POI actor that implements `UIFCInteractablePOI`, it forwards the event to the parent convoy via `ParentConvoy->HandlePOIOverlap(OtherActor)`.
   * Leaves all decision-making (stopping, latching, interaction routing) to `AFCOverworldConvoy` and `UFCInteractionComponent`.

3. **Leader path-follow movement (manual, no AI MoveTo)**

   * `MoveConvoyMemberToLocation(const FVector& TargetLocation)` is used for the **leader** member when the player issues an overworld click-to-move.
   * Projects `TargetLocation` onto the NavMesh via `UNavigationSystemV1::ProjectPointToNavigation`.
   * Requests a path using `UNavigationSystemV1::FindPathToLocationSynchronously` and stores the resulting `PathPoints`.
   * Sets `bIsFollowingPath = true` and starts consuming `PathPoints` in `Tick` by steering with `AddMovementInput(Direction, 1.0f, /*bForce=*/true)` toward the current point.
   * When the current point is within `AcceptRadius`, advances `CurrentPathIndex`; when no more points are valid, clears state and calls `StopConvoyMovement()`.

4. **Follower formation movement (leader + local offset)**

   * Follower members do **not** compute their own NavMesh paths.
   * `StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset)` enables follower mode:
     * Stores a weak pointer to `InLeader` in `Leader`.
     * Stores `FollowerOffset` (local-space offset relative to the leader).
     * Sets `bIsFollowingLeader = true` and disables any active path-follow (`bIsFollowingPath = false`).
   * In `Tick`, followers:
     * Compute a desired world position as `Leader->GetActorLocation() + Leader->GetActorRotation().RotateVector(FollowerOffset)` so the formation rotates with the leader.
     * If the distance to the desired position is greater than `AcceptRadius`, apply `AddMovementInput` toward that position (again with `bForce = true`).
   * `StopFollowingLeader()` clears `bIsFollowingLeader` and the cached `Leader` pointer.

5. **Character presentation**

   * Serves as the base for Blueprint instances that configure mesh, materials, and `UCharacterMovementComponent` parameters for each convoy member.

---

## Public API (Blueprint-facing / usable surface)

* `SetParentConvoy(AFCOverworldConvoy* InConvoy)` — must be called by the parent convoy after spawning a member to establish the parent-child relationship.
* `MoveConvoyMemberToLocation(const FVector& TargetLocation)` — used on the leader to start NavMesh path-following toward a world-space target.
* `StopConvoyMovement()` — stops any active path-follow, clears path state, and asks `UCharacterMovementComponent` to stop movement immediately.
* `StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset)` — enables follower mode with a configurable local-space formation offset.
* `StopFollowingLeader()` — disables follower mode and clears the cached leader.

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `AFCOverworldConvoy` — convoy aggregation + movement API owner

**Delegated:** exposing the high-level movement API and aggregating POI overlaps.
**How it connects:**
- When `OnCapsuleBeginOverlap` detects a relevant actor (POI), the member calls `ParentConvoy->HandlePOIOverlap(OtherActor)`.
- `AFCOverworldConvoy::MoveConvoyToLocation` calls `LeaderMember->MoveConvoyMemberToLocation(TargetLocation)` on the designated leader.
- `AFCOverworldConvoy::StopConvoy` calls `StopConvoyMovement()` on all members to halt both leader and followers.

### 2) `UFCInteractionComponent` (via convoy) — arrival-gated interaction

**Delegated:** deciding whether an overlap represents a planned arrival or incidental collision, and executing POI actions.
**How it connects:**
- `AFCOverworldConvoy` raises `OnConvoyPOIOverlap(POIActor)`.
- `AFCPlayerController` has bound this delegate to `UFCInteractionComponent::NotifyArrivedAtPOI` so the interaction state machine can react accordingly.

### 3) `UCharacterMovementComponent` — movement simulation

**Delegated:** physics and movement simulation for both leader and followers.
**Why:** convoy members express intent via `AddMovementInput`; the movement component handles velocity, acceleration, and plane-constrained motion.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Blueprint configuration

* Blueprint subclasses of `AFCConvoyMember` define the visual appearance and movement tuning (via `UCharacterMovementComponent`).

### Parent wiring expectations

* `AFCOverworldConvoy::SpawnConvoyMembers()` is expected to:
   * Spawn each `AFCConvoyMember` from `ConvoyMemberClass`.
   * Call `SetParentConvoy(this)` so overlap events are routed correctly.
   * Choose which member is the leader and call `StartFollowingLeader` with appropriate offsets on followers.
