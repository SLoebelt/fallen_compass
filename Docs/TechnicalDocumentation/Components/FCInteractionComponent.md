## UFCInteractionComponent — Technical Documentation

### Where to find it (paths)

* **Header:** `Interaction/FCInteractionComponent.h` 
* **Source:** `Interaction/FCInteractionComponent.cpp` 

---

## Responsibilities (what this component owns)

**`UFCInteractionComponent`** is the central **POI interaction orchestrator** that lives on `AFCPlayerController` (not on pawns), making it available across all scenes.

Key responsibilities:

1. **POI interaction flow**

   * Handles POI clicks (right-click and left-click) via `HandlePOIClick(AActor* POIActor)`.
   * Queries available actions from POI actors implementing `IIFCInteractablePOI`.
   * Auto-selects single actions or shows action selection UI (via `UFCUIManager`) for multiple actions.

2. **Mode-aware movement**

    * Detects current camera mode (via `UFCCameraManager`) to choose appropriate movement command:
       * **TopDown (Overworld):** calls `AFCPlayerController::MoveConvoyToLocation()`
       * **POIScene (Camp):** calls `AFCPlayerController::MoveExplorerToLocation()`
    * Uses NavMesh-based movement via the controller's movement commands (no AIController owned by this component).

3. **Action selection callback**

   * `OnPOIActionSelected(EFCPOIAction)` handles player's action choice from UI widget.
   * Stores pending action and triggers movement if not already at POI.
   * Immediate execution if already at POI location (left-click scenario).

4. **Arrival vs incidental overlap notification**

   * Provides two closely-related entry points into the interaction state machine:
     * `NotifyArrivedAtPOI(AActor* POIActor)` — **canonical arrival** API used by Overworld convoy and Camp explorer when they reach a POI (overlap or path-complete). If `bAwaitingArrival` is set and `PendingPOI`/`PendingAction` match, it executes the pending action via `ExecutePOIActionNow`; otherwise, it falls back to incidental handling.
     * `NotifyPOIOverlap(AActor* POIActor)` — **incidental overlap** handler for unplanned collisions (enemy ambush, LMB move collisions, exploratory walking). If there is a matching pending POI and action (and we are not still awaiting selection), it completes that execution; otherwise, it auto-executes single-action POIs or opens the selection UI for multi-action POIs.
   * Both code paths use the internal helper `ExecutePOIActionNow(AActor* POIActor, EFCPOIAction Action)` to clear pending interaction state exactly once (via `ResetInteractionState()`) and then execute the selected action via `IIFCInteractablePOI::Execute_ExecuteAction()`.
   * This guarantees idempotent execution even if overlaps arrive re-entrantly or multiple arrival events are fired for the same POI.

5. **Office trace-based interaction (legacy)**

   * `Interact()` performs forward trace from character camera to detect `IFCInteractable` objects.
   * Shows/hides interaction prompt widget based on what's in view.
   * Still supported for Office first-person interactions with desks/objects.

---

## Public API

### POI interaction (Overworld + Camp)

* `HandlePOIClick(AActor* POIActor)` → entry point for POI interactions (called by `AFCPlayerController`).
* `OnPOIActionSelected(EFCPOIAction SelectedAction)` → callback from action selection UI widget.
* `NotifyArrivedAtPOI(AActor* POIActor)` → canonical arrival entry point used by convoy/explorer when they reach a POI; completes a pending, arrival-gated action if it matches, otherwise delegates to incidental overlap handling.
* `NotifyPOIOverlap(AActor* POIActor)` → incidental overlap handler used for unplanned collisions; may still execute pending actions or start a new selection flow depending on current state.
* `GetPendingInteractionPOI()` → returns currently queued POI actor.

### Office interaction (FirstPerson)

* `Interact()` → performs trace and interacts with focused object (called by `AFCPlayerController` on 'E' key).
* `UpdateInteractionPrompt()` → updates/shows/hides prompt widget based on trace results (called from Tick).

### Delegates

* `OnPOIActionCompleted` → broadcasts `(AActor* POI, EFCPOIAction Action, bool bSuccess)` when POI action finishes.

---

## Connected systems

### 1) `AFCPlayerController` — movement command issuer

**Relationship:** Calls `MoveConvoyToLocation()` or `MoveExplorerToLocation()` based on camera mode.
**Why:** Controller owns movement commands; InteractionComponent determines *when* to move.

### 2) `UFCCameraManager` (component) — mode detection

**Relationship:** Queries `GetCameraMode()` to detect TopDown vs POIScene for movement routing.
**Why:** Camera mode determines which movement function to use.

### 3) `UFCUIManager` (subsystem) — action selection UI

**Relationship:** Calls `ShowPOIActionSelection()` for multi-action POIs; receives callback via `OnPOIActionSelected()`.
**Why:** UI concerns are centralized in UIManager; InteractionComponent just requests UI and handles result.

### 4) `IIFCInteractablePOI` (interface) — POI contract

**Relationship:** Queries `GetAvailableActions()`, `GetPOIName()`, and `ExecuteAction()` on POI actors.
**Why:** POI behavior is defined by interface; InteractionComponent orchestrates the flow.

---

## Architecture notes

### Why on PlayerController instead of pawn?

**Previous:** Lived on `AFCFirstPersonCharacter`, only available in Office.  
**Current:** Lives on `AFCPlayerController`, available across all scenes (Office, Overworld, Camp).

**Benefits:**
- Single POI interaction system works consistently across Overworld and Camp.
- No need to duplicate POI logic on different pawn types.
- Mode-aware design allows same component to handle different movement patterns.
- Internally caches its `AFCPlayerController` owner in `OnRegister`/`BeginPlay` and uses that cached pointer everywhere (no `GetInstigatorController`/`GetFirstPlayerController`), logging a clear error once if mis-owned.
- Gates FirstPerson focus tracing and prompts behind a boolean (`bFirstPersonFocusEnabled`) that is now toggled explicitly by higher-level configuration (e.g., `UFCPlayerModeCoordinator`) instead of being recomputed every Tick from `UFCCameraManager::GetCameraMode()`. When disabled, it clears the current focus, hides the prompt widget, and early-outs from its trace/prompt update path so Overworld/Camp do not run FP traces.

### Invariants (0003 – pending state + arrival-gated execution)

To keep POI interactions predictable and maintainable, `UFCInteractionComponent` enforces these invariants:

- **Single-action policy**
  - 0 actions → log and ignore.
  - 1 action → **auto-run**:
    - Set `PendingPOI` + `PendingAction`.
    - Use a distance-based check (`NeedsMovement` inside `HandlePOIClick`) to decide:
      - If movement is needed → set `bAwaitingArrival = true` and issue movement via a local `IssueMovementToPOI` lambda (which calls `MoveConvoyToLocation` / `MoveExplorerToLocation` on the controller).
      - If already at POI → call `NotifyArrivedAtPOI` immediately (still arrival-gated, no movement).
  - >1 actions → **Selecting**:
    - `bAwaitingSelection = true`, `bAwaitingArrival = false`.
    - `PendingPOI` stores the clicked POI.
    - Ask `UFCUIManager` to show the action-selection UI.
    - No movement until `OnPOIActionSelected` is called.

- **Single movement authority**
  - Movement for POI interactions is only issued from:
    - `HandlePOIClick` (for 1‑action POIs), and
    - `OnPOIActionSelected` (for >1‑action POIs after selection),
  - Both issue movement exclusively via controller helpers (`MoveConvoyToLocation` / `MoveExplorerToLocation`) inside the shared `IssueMovementToPOI` lambda.
  - UI widgets and `UFCUIManager` never call movement functions directly.

- **Arrival-gated execution**
  - Planned arrivals (from convoy or explorer) enter via `NotifyArrivedAtPOI`:
    - If `bAwaitingArrival` is true and `PendingPOI`/`PendingAction` match the arriving POI, the component executes the action via `ExecutePOIActionNow`.
    - If arrival does not match the pending POI, it cancels the old intent via `ResetInteractionState()` and falls back to `NotifyPOIOverlap` for incidental handling.
  - Incidental overlaps (enemy ambush, LMB move collisions, exploratory walking) enter via `NotifyPOIOverlap` and:
    - Execute a matching pending POI/action if appropriate, or
    - Auto-execute single-action POIs, or
    - Start a new selection flow for multi-action POIs.

- **Idempotent execution**
  - All actual POI action execution goes through `ExecutePOIActionNow(AActor* POIActor, EFCPOIAction Action)`, which:
    - Calls `ResetInteractionState()` once before executing.
    - Invokes `IIFCInteractablePOI::Execute_ExecuteAction` on the POI.
    - Clears the convoy’s `bIsInteractingWithPOI` latch (via the active convoy, if any).
  - This prevents double-fires even if multiple overlaps or arrival events occur for the same POI.

- **FP carve-out**
  - FirstPerson / Office flow uses `CurrentInteractable` + `Interact()` only.
  - FP interactions never call convoy/explorer movement helpers and do not touch POI pending state (`PendingPOI`, `PendingAction`, `bAwaitingSelection`, `bAwaitingArrival`).

### Movement pattern difference

**Overworld (TopDown):**
- PlayerController possesses `AFCOverworldConvoy` pawn.
- Convoy movement is pawn-driven (character movement + path-following on the leader), not via an AIController owned by `UFCInteractionComponent`.
- InteractionComponent calls `MoveConvoyToLocation()` on the controller; the controller/pawn own the concrete movement implementation.

**Camp (POIScene):**
- PlayerController possesses `AFC_ExplorerCharacter` pawn (standard pawn possession; camera remains static via `UFCCameraManager`).
- `AFC_ExplorerCharacter` owns its NavMesh path + steering logic; movement is pawn-driven and steered through `AddMovementInput` in `Tick`.
- InteractionComponent calls `MoveExplorerToLocation()` on the controller, which forwards to the possessed explorer pawn.

**Office (FirstPerson):**
- PlayerController possesses `AFCFirstPersonCharacter` pawn.
- No POI movement; uses legacy `Interact()` trace-based system for desks/objects only.

### Interaction profiles (0004 – mode-specific FP interaction)

`UFCInteractionComponent` can be configured by `UFCInteractionProfile` DataAssets:

* A profile defines:
  * Probe type (currently applied as line traces; future: sphere trace, overlap, cursor-hit).
  * Trace channel and range.
  * Optional radius (for non-line traces).
  * Optional prompt widget class.
  * Optional allowed actor tags (filter which actors can ever be considered as interactables in this mode).
* The current active profile is supplied by `UFCPlayerModeCoordinator` via the current `FPlayerModeProfile::InteractionProfile` and applied through `UFCInteractionComponent::ApplyInteractionProfile(UFCInteractionProfile* NewProfile)`.
* `DetectInteractables()` resolves the effective profile (`ActiveProfile` or `DefaultProfile`), uses its `Range` and `TraceChannel` to build the trace, and applies `AllowedTags` filtering before checking `IIFCInteractable` and per-object `GetInteractionRange` / `CanInteract`.
* Before tracing, `DetectInteractables()` checks `AFCPlayerController::CanWorldInteract()` (backed by `UFCUIBlockSubsystem`) and early-outs if world interaction is currently blocked by UI so prompts/logs do not appear behind modals.

---

## Configuration

### Widget setup

* **Interaction Prompt Widget Class:** Set in Blueprint (expects `SetInteractionPrompt(FString)` function).
* **POI Action Selection Widget:** Configured via `UFCGameInstance` → `UFCUIManager` (expects `PopulateActions(TArray<FFCPOIActionData>)` function).

### POI Actor requirements

POIs must implement `IIFCInteractablePOI` and provide:
- `GetAvailableActions()` → returns array of `FFCPOIActionData`.
- `GetPOIName()` → returns display name string.
- `ExecuteAction(EFCPOIAction, AActor* Instigator)` → performs action logic.
