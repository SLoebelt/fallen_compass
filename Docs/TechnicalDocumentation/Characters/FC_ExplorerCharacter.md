## AFC_ExplorerCharacter — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `FC_ExplorerCharacter.h` 
* **Source:** `FC_ExplorerCharacter.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFC_ExplorerCharacter`** is the project’s **Camp / POI scene exploration pawn**. It represents the designated explorer (male or female) in local scenes and owns:

1. **Top-down movement in Camp / POIScene**

   * Uses standard `ACharacter` movement tuned for top-down navigation (no controller rotation driving facing).
   * `MoveExplorerToLocation(const FVector& TargetLocation)` computes a NavMesh path (via `UNavigationSystemV1::FindPathToLocationSynchronously`) and stores the resulting path points in `PathPoints`.
   * `Tick(float DeltaTime)` advances along `PathPoints` by steering with `AddMovementInput` toward the current point until within `AcceptRadius`, then progresses to the next point; when the last point is consumed, it stops following.

2. **Explorer identity**

   * Exposes `ExplorerType` (`EFCExplorerType::Male` / `Female`) as an editor-facing property to drive mesh/appearance selection in Blueprint.

3. **POI arrival notification (Camp side)**

   * Caches a `UFCInteractionComponent` reference from the possessing `AFCPlayerController` in `BeginPlay`.
   * Binds the capsule’s `OnComponentBeginOverlap` to `OnCapsuleBeginOverlap`.
   * When overlapping an actor that implements `UIFCInteractablePOI`, logs and forwards the arrival to the interaction component via `InteractionComponent->NotifyArrivedAtPOI(OtherActor)`.
   * This makes the explorer behave like a “mini convoy member” from the interaction system’s perspective: it does **not** decide actions, it only reports arrivals.

4. **Debug logging and diagnostics**

   * Logs `ExplorerType`, current controller, and overlap events to ease debugging.
   * Exposes `PathPoints`, `CurrentPathIndex`, `bIsFollowingPath`, and `AcceptRadius` as debug or editor-visible properties under `Movement` / `Movement|Debug`.

---

## Public API (Blueprint-facing / usable surface)

### Movement

* `MoveExplorerToLocation(const FVector& TargetLocation)` — starts NavMesh pathfinding and path-follow steering toward a world-space location in Camp/POIScene.

### Debug / state introspection

* `ExplorerType` (`EFCExplorerType`) — editable in the editor to choose which explorer variant is used.
* `PathPoints`, `CurrentPathIndex`, `bIsFollowingPath`, `AcceptRadius` — visible for debug in-editor to inspect movement behavior.

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `AFCPlayerController` — movement command source and interaction owner

**Delegated to controller:** deciding *when* the explorer should move and which POI is active.
**How it connects:**
- In Camp (`POIScene` camera mode), `AFCPlayerController::HandleClick` ultimately calls `MoveExplorerToLocation()` on the possessed explorer pawn.
- The controller owns `UFCInteractionComponent`, which tracks pending POI state and handles arrival-gated execution.

### 2) `UNavigationSystemV1` — pathfinding

**Delegated:** actual NavMesh path computation.
**Why:** the pawn queries `FindPathToLocationSynchronously`, then uses the returned `UNavigationPath::PathPoints` as steering targets.

### 3) `UFCInteractionComponent` — POI interaction orchestration

**Delegated:** deciding what to do when arriving at a POI.
**How it connects:**
- On `BeginPlay`, the explorer looks up the owning `AFCPlayerController` and finds its `UFCInteractionComponent`.
- Capsule overlaps with POIs call `InteractionComponent->NotifyArrivedAtPOI(OtherActor)`, entering the shared arrival-gated state machine (planned vs incidental overlap).

### 4) `UCharacterMovementComponent` — physics and movement simulation

**Delegated:** applying velocity, acceleration, and rotation.
**Why:** the explorer only supplies intent via `AddMovementInput`; movement tuning (speed, rotation rate, plane constraint) is configured in the movement component.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Movement tuning

* `AcceptRadius` (default `50.0f`) — distance within which the explorer considers a path point “reached” and advances to the next.
* CharacterMovement properties (set in C++ and tweakable in BP):
  * `bOrientRotationToMovement = true`
  * `RotationRate = (0, 500, 0)`
  * `bConstrainToPlane = true`, `bSnapToPlaneAtStart = true`
  * `MaxWalkSpeed = 300.0f`

### Explorer identity

* `ExplorerType` — used by Camp/POI Blueprints to select appropriate mesh/appearance.

### Interaction wiring

* Capsule overlap is bound automatically in `BeginPlay` via `UCapsuleComponent::OnComponentBeginOverlap.AddDynamic(...)`.
* Requires `AFCPlayerController` to own a `UFCInteractionComponent` so arrival events can be forwarded correctly.
