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

4. **POI overlap notification**

   * `NotifyPOIOverlap(AActor*)` is bound as a handler to world-side POI-overlap delegates (e.g., `AFCOverworldConvoy`), and triggers when convoy/explorer reaches a POI.
   * Executes stored pending action via `IIFCInteractablePOI::Execute_ExecuteAction()`.
   * Broadcasts result via `OnPOIActionCompleted` delegate.

5. **Office trace-based interaction (legacy)**

   * `Interact()` performs forward trace from character camera to detect `IFCInteractable` objects.
   * Shows/hides interaction prompt widget based on what's in view.
   * Still supported for Office first-person interactions with desks/objects.

---

## Public API

### POI interaction (Overworld + Camp)

* `HandlePOIClick(AActor* POIActor)` → entry point for POI interactions (called by `AFCPlayerController`).
* `OnPOIActionSelected(EFCPOIAction SelectedAction)` → callback from action selection UI widget.
* `NotifyPOIOverlap(AActor* POIActor)` → called when convoy/explorer reaches POI to execute action.
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

### Movement pattern difference

**Overworld (TopDown):**
- PlayerController possesses `AFCOverworldConvoy` pawn.
- Convoy leader has AI controller that drives movement.
- InteractionComponent calls `MoveConvoyToLocation()` → delegates to convoy AI.

**Camp (POIScene):**
- PlayerController possesses `AFC_ExplorerCharacter` pawn (standard pawn possession; camera remains static via `UFCCameraManager`).
- `AFC_ExplorerCharacter` owns its own NavMesh path + steering logic; movement is pawn-driven, not via a dedicated AIController.
- InteractionComponent calls `MoveExplorerToLocation()` → delegates to controller, which forwards to the possessed explorer pawn.

**Office (FirstPerson):**
- PlayerController possesses `AFCFirstPersonCharacter` pawn.
- No POI interactions; uses legacy `Interact()` trace-based system for desks.

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
