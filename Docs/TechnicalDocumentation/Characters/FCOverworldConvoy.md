## AFCOverworldConvoy — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `Characters/Convoy/FCOverworldConvoy.h` 
* **Source:** `Characters/Convoy/FCOverworldConvoy.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFCOverworldConvoy`** is the project’s **Overworld convoy root actor**. It owns the convoy hierarchy, member spawning, **convoy pivot position**, and aggregation of POI overlap events.

1. **Convoy composition and hierarchy**

   * Owns editor-visible spawn points for the leader and followers (`LeaderSpawnPoint`, `Follower1SpawnPoint`, `Follower2SpawnPoint`) attached to a common `ConvoyRoot`.
   * Spawns instances of `ConvoyMemberClass` at runtime in `SpawnConvoyMembers()`, attaches them logically to the convoy, and records them in the `ConvoyMembers` array.
   * Tracks the designated `LeaderMember` for convenience (movement API + follower formation setup).
   * Exposes `GetLeaderMember()` and `GetCameraAttachPoint()` for other systems (e.g., `UFCCameraManager`, `AFCPlayerController`).

2. **Convoy pivot + camera attachment for top-down view**

   * Provides `CameraAttachPoint` above the convoy root so the overworld camera can track the convoy in top-down mode.
   * On `Tick`, computes the bounding box that encloses all valid convoy members and lerps the convoy actor’s location toward the **bounds center**. This keeps cameras, sampling, and meta-systems centered on the group instead of a single pawn.

3. **POI interaction aggregation**

   * Acts as the single aggregation point for POI overlaps from its `AFCConvoyMember` children.
   * Maintains a simple latch `bIsInteractingWithPOI` to prevent double-triggering while an interaction is in progress.
   * On `HandlePOIOverlap(AActor* POIActor)`:
     * Early-outs if `POIActor` is null or `bIsInteractingWithPOI` is already true.
     * Sets `bIsInteractingWithPOI = true`.
     * Calls `StopConvoy()` to halt movement for all members.
     * Broadcasts `OnConvoyPOIOverlap.Broadcast(POIActor)` so higher-level systems (via the player controller and `UFCInteractionComponent`) can treat this as an arrival into the interaction state machine.
   * `SetInteractingWithPOI(bool)` allows the interaction component to clear the latch once a POI action finishes (via `ExecutePOIActionNow`).

4. **Movement API surface (delegating to members)**

   * `MoveConvoyToLocation(const FVector& TargetLocation)` logs the command and forwards it to `LeaderMember->MoveConvoyMemberToLocation(TargetLocation)`, which performs NavMesh pathfinding and path-follow movement.
   * `StopConvoy()` iterates over `ConvoyMembers` and calls `StopConvoyMovement()` on each member, ensuring both leader path-follow and follower formation movement are stopped immediately.

---

## Public API (Blueprint-facing / usable surface)

* `GetLeaderMember() -> AFCConvoyMember*` — returns the convoy leader used by the controller for movement and camera.
* `GetCameraAttachPoint() -> USceneComponent*` — attachment point for overworld camera follow.
* `IsInteractingWithPOI() -> bool` — returns whether the convoy is currently considered to be interacting with a POI.
* `SetInteractingWithPOI(bool bInteracting)` — called by `UFCInteractionComponent` to clear/set the interaction latch.
* `StopAllMembers()` — utility to stop AI movement on all convoy members.
* `HandlePOIOverlap(AActor* POIActor)` — internal/Blueprint-callable entry point when a convoy member detects a POI overlap; sets the latch, stops members, and broadcasts `OnConvoyPOIOverlap`.
* `OnConvoyPOIOverlap` (multicast delegate) — event that passes the `POIActor` to listeners (typically the player controller's interaction component).

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `AFCConvoyMember` — overlap detection

**Delegated:** actual capsule overlap detection with POIs and forwarding to the parent convoy.
**How it connects:**
- Each convoy member caches a pointer to its parent convoy via `SetParentConvoy(AFCOverworldConvoy* InConvoy)`.
- In `OnCapsuleBeginOverlap`, a member checks if `OtherActor` is a POI and, if so, calls `ParentConvoy->HandlePOIOverlap(OtherActor)`.

### 2) `AFCPlayerController` + `UFCInteractionComponent` — movement and interaction orchestration

**Delegated:** deciding how the convoy should move and what to do on arrival.
**How it connects now (post 0003 Step 2):**
- `AFCPlayerController` resolves the active convoy and binds `AFCOverworldConvoy::OnConvoyPOIOverlap` to `UFCInteractionComponent::NotifyArrivedAtPOI`.
- When the convoy overlaps a POI, the interaction component receives this as a **planned arrival** and decides whether to execute a pending, arrival-gated POI action (`bAwaitingArrival` + matching `PendingPOI`/`PendingAction`) or to treat it as an incidental overlap and route to `NotifyPOIOverlap`.
- `UFCInteractionComponent::ExecutePOIActionNow` calls `Convoy->SetInteractingWithPOI(false)` once execution finishes so the convoy can trigger future POIs again.

### 3) `AAIController` (per member) — movement control

**Delegated:** actual movement commands/steering for each convoy member in Overworld.
**Why:** the convoy root only needs to stop members (via `StopAllMembers()`); movement commands are issued via the controller and AI for now.

### 4) `UFCCameraManager` / overworld camera

**Delegated:** camera follow behavior.
**How it connects:**
- Overworld camera uses `GetCameraAttachPoint()` as a follow target for the convoy.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Blueprint configuration

* `ConvoyMemberClass` — set in the `AFCOverworldConvoy` Blueprint to the desired convoy member character class.
* Spawn point components (`LeaderSpawnPoint`, `Follower1SpawnPoint`, `Follower2SpawnPoint`) — can be repositioned/rotated in the editor to change convoy formation.

### Event binding expectations

* In `AFCPlayerController`, the active convoy’s `OnConvoyPOIOverlap` is bound to `UFCInteractionComponent::NotifyArrivedAtPOI` so POI overlaps become arrivals in the interaction state machine.

### Interaction latch

* `bIsInteractingWithPOI` is private and only mutable via `SetInteractingWithPOI`. Callers must clear it when they are done executing the POI action to allow fresh overlaps.
