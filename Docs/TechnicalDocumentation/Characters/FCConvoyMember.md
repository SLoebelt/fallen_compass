## AFCConvoyMember — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `Characters/Convoy/FCConvoyMember.h` 
* **Source:** `Characters/Convoy/FCConvoyMember.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFCConvoyMember`** is the base class for all **Overworld convoy member characters**. It owns capsule overlap detection and forwarding POI events to its parent convoy.

1. **Convoy membership wiring**

   * Stores a pointer to its parent `AFCOverworldConvoy` in `ParentConvoy`.
   * Exposes `SetParentConvoy(AFCOverworldConvoy* InConvoy)` so the convoy root can assign itself after spawning members.

2. **POI overlap detection**

   * Binds the capsule component’s `OnComponentBeginOverlap` to `OnCapsuleBeginOverlap` in `BeginPlay` (see `.cpp` implementation).
   * In `OnCapsuleBeginOverlap`, when overlapping a POI actor, it forwards the event to the parent convoy via `ParentConvoy->HandlePOIOverlap(OtherActor)`.
   * Leaves all decision-making (stopping, latching, interaction routing) to `AFCOverworldConvoy` and `UFCInteractionComponent`.

3. **Character presentation**

   * Serves as the base for Blueprint instances that configure mesh, materials, and `UCharacterMovementComponent` parameters for each convoy member.

---

## Public API (Blueprint-facing / usable surface)

* `SetParentConvoy(AFCOverworldConvoy* InConvoy)` — must be called by the parent convoy after spawning a member to establish the parent-child relationship.

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `AFCOverworldConvoy` — convoy aggregation

**Delegated:** handling the aggregated POI overlap and interaction latching.
**How it connects:**
- When `OnCapsuleBeginOverlap` detects a relevant actor (POI), the member calls `ParentConvoy->HandlePOIOverlap(OtherActor)`.
- The convoy root then stops movement, sets `bIsInteractingWithPOI`, and broadcasts `OnConvoyPOIOverlap` to the interaction system.

### 2) `UFCInteractionComponent` (via convoy) — arrival-gated interaction

**Delegated:** deciding whether an overlap represents a planned arrival or incidental collision, and executing POI actions.
**How it connects:**
- `AFCOverworldConvoy` raises `OnConvoyPOIOverlap(POIActor)`.
- `AFCPlayerController` has bound this delegate to `UFCInteractionComponent::NotifyArrivedAtPOI` so the interaction state machine can react accordingly.

### 3) `UCharacterMovementComponent` / AI controller — movement

**Delegated:** steering/locomotion and AI decision-making for members.
**Why:** the member class focuses on collision + event forwarding; it does not issue movement commands itself.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Blueprint configuration

* Blueprint subclasses of `AFCConvoyMember` define the visual appearance and movement tuning.

### Parent wiring expectations

* `AFCOverworldConvoy::SpawnConvoyMembers()` is expected to:
  * Spawn each `AFCConvoyMember` from `ConvoyMemberClass`.
  * Attach it to the appropriate spawn point.
  * Call `SetParentConvoy(this)` so overlap events are routed correctly.
