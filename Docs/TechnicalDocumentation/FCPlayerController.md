## AFCPlayerController — Technical Documentation (Manager-style)

(Structure aligned to your existing template doc.)

### Where to find it (paths)

* **Header:** `FCPlayerController.h` 
* **Source:** `FCPlayerController.cpp` 

---

## Responsibilities (what this "manager" owns)

**`AFCPlayerController`** is the project's central **player input + camera + UI interaction router**, and the glue between game state and "how the player can interact right now".

Key responsibilities:

1. **Component ownership**

   * Owns `UFCInputManager`, `UFCCameraManager`, and `UFCInteractionComponent` as ActorComponents (created in constructor).
   * These components are available across all scenes and camera modes, making POI interactions consistent.

2. **Input orchestration (Enhanced Input + config asset)**

   * Binds input actions to handlers in `SetupInputComponent()` (Interact, Click, Escape/Pause, QuickSave, QuickLoad, OverworldPan/Zoom, ToggleOverworldMap).
   * Does **not** load actions via hardcoded asset paths or per-action `UPROPERTY`s anymore. Instead, it requests the shared `UFCInputConfig` from `UFCInputManager` and binds the `UInputAction*`s exposed by that config if present.
   * Delegates mapping-context application for each mode to `UFCInputManager::SetInputMappingMode`, which looks up the correct `UInputMappingContext` in the same `UFCInputConfig` data asset.

3. **Camera-mode routing**

   * Exposes `SetCameraModeLocal(...)` as a **camera-only** helper that selects which logical camera mode should be active and delegates the actual blending to `UFCCameraManager`.
   * Does **not** update cursor visibility or input mode here; those are applied from `FPlayerModeProfile` by `UFCPlayerModeCoordinator`.
   - For Camp/POI, the controller does **not** search for cameras itself; instead `UFCCameraManager::BlendToPOISceneCamera(POISceneCameraActor, BlendTime)` handles static Camp camera targets and uses `SetViewTargetWithBlend` on the controller while the player-controlled `AFC_ExplorerCharacter` moves.

4. **Game-flow entry points**

   * Initializes the main menu UI (`InitializeMainMenu()`), starts gameplay from menu (`TransitionToGameplay()`), and can return to main menu (`ReturnToMainMenu()`), primarily by driving `UFCGameStateManager` transitions and UIManager flows. Presentation (camera/input/cursor) for each resulting state is applied by `UFCPlayerModeCoordinator`.

5. **World interaction routing**

   * A single click action is routed depending on camera mode:

     * **TopDown (Overworld):** click-to-move convoy + POI interaction (via `InteractionComponent`). 
     * **POIScene (Camp):** click-to-move explorer + POI interaction (via `InteractionComponent`).
     * **FirstPerson (Office):** trace-based interaction (via `InteractionComponent`) + table-object interaction.
     * **TableView:** line trace + table-object interaction through `IFCTableInteractable`. 

6. **Movement command delegation**

   * `MoveConvoyToLocation(FVector)` → in Overworld, forwards click-to-move requests to the active `AFCOverworldConvoy` actor, which in turn starts/stops **manual path-follow** on its leader `AFCConvoyMember` (no AI `MoveTo`). The controller never talks to a convoy AI controller directly.
   * `MoveExplorerToLocation(FVector)` → in Camp/POIScene, routes Camp click-to-move to the **possessed** `AFC_ExplorerCharacter` (no AIController). The controller delegates pathfinding + steering to the pawn instead of driving movement via an AI controller.

7. **Pause / resume behavior**

   * Handles ESC by consulting `UFCGameStateManager` and toggling pause state (via state stack) and pause UI through `UFCUIManager`. 

8. **Dev save/load shortcuts**

   * Implements `DevQuickSave()` / `DevQuickLoad()` using `UFCGameInstance::SaveGame(...)` and `LoadGameAsync(...)` with slot `"QuickSave"`. 

9. **Startup consistency on load**

   * Defers subscription to `UFCGameStateManager::OnStateChanged` to `UFCPlayerModeCoordinator` (a controller-owned component). The coordinator caches the current state on BeginPlay, maps it to an `EFCPlayerMode`, looks up an `FPlayerModeProfile` in its `UFCPlayerModeProfileSet`, and applies camera/input/cursor directly so presentation is correct after loading.


## Public API

### Input / camera

* `SetInputMappingMode(EFCInputMappingMode NewMode)` → delegates mapping changes to `UFCInputManager`.
* `SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime=2.0f)` → camera switching only; cursor/input-mode configuration is now profile-driven via `UFCPlayerModeCoordinator`. 
* `FadeScreenOut(float Duration=1.0f, bool bShowLoading=false)` / `FadeScreenIn(float Duration=1.0f)` → delegates fades to `UFCTransitionManager`. 

### Game flow

* `InitializeMainMenu()` → shows main menu via `UFCUIManager`. 
* `TransitionToGameplay()` → transitions to `Office_Exploration`, hides menu UI, blends to FirstPerson, restores input after blend via timer. 
* `ReturnToMainMenu()` → delegates to `UFCLevelTransitionManager::ReturnToMainMenuFromGameplay()`. 

### Table interaction

* `HandleTableObjectClick()` → trace under cursor, require `IFCTableInteractable::CanInteract`, then `OnTableObjectClicked(...)`. 
* `OnTableObjectClicked(AActor* TableObject)` → opens the correct table widget and sets Desk/Object sub-modes; may delegate state change to `UFCLevelTransitionManager`. 
* `CloseTableWidget()` → closes table widget + restores table camera. 
* `CanProcessWorldInteraction() const` → blocks traces when UI says a focused blocking widget (or table widget) is open. 

> Note: the header comment says "Bound to IA_TableClick", but in code the click action loaded is `/Game/FC/Input/IA_Click` and it's bound to `HandleClick(...)`, which then routes to table click behavior based on camera mode.

### Movement commands (Overworld + Camp)
* `GetPossessedConvoy()` / `MoveConvoyToLocation(const FVector&)` → Overworld click-to-move calls into the active `AFCOverworldConvoy` instance, which exposes `MoveConvoyToLocation` / `StopConvoy` and drives leader + follower movement via manual NavMesh path-follow and `AddMovementInput` (no AI `MoveTo`).
* `MoveExplorerToLocation(const FVector&)` → in Camp/POIScene, routes Camp click-to-move to the **possessed** `AFC_ExplorerCharacter` (no AIController). The controller delegates NavMesh pathfinding and steering to the pawn via `AFC_ExplorerCharacter::MoveExplorerToLocation` (which computes a NavMesh path and follows it in `Tick` using `AddMovementInput`).
* `SetMenuCameraActor(ACameraActor*)`, `SetPOISceneCameraActor(ACameraActor*)` → assign cameras used by the camera manager / POI scenes. 

### Save/load

* `DevQuickSave()` / `DevQuickLoad()` (BlueprintCallable) → uses slot `"QuickSave"` plus `DoesSaveGameExist(...)` guard. 

---

## Connected systems ("connected managers") and what/why is delegated

### 1) `UFCInputManager` (component) — mapping contexts / input mode policy

**Delegated:** the actual mapping-context switching (`SetInputMappingMode`).
**Why:** keeps this controller as the *router*, while input policy stays centralized in the input component.

### 2) `UFCCameraManager` (component) — camera blends and view targets

**Delegated:** blending to FirstPerson/TopDown/Menu/TableObject/POIScene/etc. (`BlendToFirstPerson`, `BlendToTopDown`, `BlendToMenuCamera`, `BlendToTableObject`, `BlendToPOISceneCamera`, …).
**Why:** camera behavior remains encapsulated; controller just chooses the mode.

### 3) `UFCInteractionComponent` (component) — POI interaction orchestration

**Delegated:** POI action selection, movement triggering, and arrival/overlap handling (`HandlePOIClick`, `OnPOIActionSelected`, `NotifyArrivedAtPOI`, `NotifyPOIOverlap`).
**Why:** centralizes POI interaction logic across Office, Overworld, and Camp; mode-aware movement (convoy vs explorer) with a single arrival-gated execution path.
**How it connects now:**
- Overworld convoy and Camp explorer expose POI-overlap or arrival delegates.
- `AFCPlayerController` resolves the active convoy (`ActiveConvoy`) at startup and binds its overlap delegate to `UFCInteractionComponent::NotifyArrivedAtPOI`, treating convoy overlaps as **arrivals** into the interaction state machine.
- Camp explorer reports capsule overlaps with POIs into `NotifyArrivedAtPOI` as well, so both movement sources share the same arrival pipeline.
- `NotifyArrivedAtPOI` decides whether the arrival completes a pending, arrival-gated intent (`bAwaitingArrival` + matching `PendingPOI`/`PendingAction`) or should be treated as an incidental collision and forwarded to `NotifyPOIOverlap`.
- Convoy/explorer remain pure event sources; they never crawl to the controller or interaction component.
 - Routing invariant (0003/0004): the controller only routes input and exposes movement helpers (e.g., `MoveConvoyToLocation`, `MoveExplorerToLocation`); all POI action selection and movement decisions live in `UFCInteractionComponent`.

### World input gating (0004 – UI blocking)

- Exposes two explicit queries used throughout click/interaction handlers:
   - `bool CanWorldClick() const;` — should the game respond to world clicks (click-to-move, POI click, table object click) right now?
   - `bool CanWorldInteract() const;` — should the game run FirstPerson focus traces / "E interact" right now?
- Both methods ask `UFCUIBlockSubsystem` (via the owning `UFCGameInstance`) whether any registered widgets currently block click and/or interact, and fall back to legacy checks while older widgets are migrated.
- Click handlers (`HandleClick`, `HandleOverworldClickMove`, `HandleTableObjectClick`) call `CanWorldClick()` before touching world actors.
- Interaction entry points (`HandleInteractPressed`) call `CanWorldInteract()` before running FP traces or delegating to `UFCInteractionComponent`.
- **Routing invariant:** the controller only routes input and exposes movement helpers; all POI action selection and movement decisions live in `UFCInteractionComponent`.

### 4) `UFCGameStateManager` (subsystem) — authoritative state + pause stack

**Delegated:** state transitions (`TransitionTo`, `PushState`, `PopState`), current state queries, and broadcasting changes.
**Why:** `UFCPlayerModeCoordinator` reacts to state changes and asks the controller to apply camera/input/cursor presentation, but the *truth* of state lives in the state manager.

### 5) `UFCUIManager` (subsystem) — all UI creation/visibility + "blocks interaction" rules

**Delegated:** showing/hiding main menu, pause menu, overworld map HUD, table widgets, POI action selection; exposing "widget open?" queries.
**Why:** prevents UI ownership from leaking into controller; controller just requests UI changes and uses UI state to gate traces.

### 6) `UFCLevelTransitionManager` (subsystem) — cross-level / high-level flows

**Delegated:** returning to main menu flow; switching into Office_TableView state from Office_Exploration when clicking a table object; Camp ↔ Overworld transitions.
**Why:** level/state orchestration belongs in one place; controller triggers it when player interaction demands it.

### 7) `UFCTransitionManager` (subsystem) — screen fades

**Delegated:** fade widget orchestration via `BeginFadeOut/BeginFadeIn`.
**Why:** fade widget persistence and lifetime is subsystem-owned; controller exposes a convenient API. 

### 8) `UFCGameInstance` — save/load + restoration helpers

**Delegated:** saving/loading and restoring player position on startup.
**Why:** persistence concerns belong to the game instance layer, not the controller.

---

## Where to find / configure key pieces (paths & assumptions)

* **Input Actions (assets):**

  * `/Game/FC/Input/IA_Click`
  * `/Game/FC/Input/IA_Escape`
  * `/Game/FC/Input/IA_OverworldPan`
  * `/Game/FC/Input/IA_OverworldZoom`
  * `/Game/FC/Input/IA_ToggleOverworldMap` 
    (Interact/QuickSave/QuickLoad are also bound if assigned; see `SetupInputComponent` guards.) 

* **Blueprint configuration expectation:**

  * Comment indicates MenuCamera is set in Blueprint (`BP_FC_PlayerController`). 

* **Office desk/tableview assumption:**

  * TableView/SaveSlotView mode tries to find an actor named like `BP_OfficeDesk` and a component named like `CameraTargetPoint`. If they're not present/named as expected, it logs a warning and won't enter TableView mode cleanly.

* **Camp camera discovery:**

  * POIScene mode (Camp) auto-finds camera by tag "CampCamera" or name containing "CampCamera" if not pre-registered via `SetPOISceneCameraActor`.
