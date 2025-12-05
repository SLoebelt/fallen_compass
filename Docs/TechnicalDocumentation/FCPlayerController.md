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

2. **Input orchestration (Enhanced Input)**

   * Binds input actions to handlers in `SetupInputComponent()` (Interact, Click, Escape/Pause, QuickSave, QuickLoad, OverworldPan/Zoom, ToggleOverworldMap). 
   * Loads several input actions via hardcoded asset paths (e.g. `/Game/FC/Input/IA_Click`, `/Game/FC/Input/IA_Escape`, `/Game/FC/Input/IA_OverworldPan`, `/Game/FC/Input/IA_OverworldZoom`, `/Game/FC/Input/IA_ToggleOverworldMap`). 

3. **Camera-mode routing**

   * Owns "what camera mode should we be in," but delegates the actual blending to `UFCCameraManager`.
   * `SetCameraModeLocal(...)` switches between MainMenu / FirstPerson / TableView / TopDown / POIScene and updates cursor + input mode accordingly. 
   - For Camp/POI (`EFCGameStateID::Camp_Local`), the controller does **not** search for cameras itself; instead it calls `UFCCameraManager::BlendToPOISceneCamera(POISceneCameraActor, BlendTime)` when the state changes.
   - If `POISceneCameraActor` is `nullptr`, `UFCCameraManager` auto-resolves an `ACameraActor` tagged `CampCamera` (or named accordingly) and uses `SetViewTargetWithBlend` on the controller to keep the Camp camera static while the player-controlled `AFC_ExplorerCharacter` moves.

4. **Game-flow entry points**

   * Initializes the main menu UI (`InitializeMainMenu()`), starts gameplay from menu (`TransitionToGameplay()`), and can return to main menu (`ReturnToMainMenu()`). 

5. **World interaction routing**

   * A single click action is routed depending on camera mode:

     * **TopDown (Overworld):** click-to-move convoy + POI interaction (via `InteractionComponent`). 
     * **POIScene (Camp):** click-to-move explorer + POI interaction (via `InteractionComponent`).
     * **FirstPerson (Office):** trace-based interaction (via `InteractionComponent`) + table-object interaction.
     * **TableView:** line trace + table-object interaction through `IFCTableInteractable`. 

6. **Movement command delegation**

   * `MoveConvoyToLocation(FVector)` → commands Overworld convoy AI controller (TopDown mode, unchanged).
   * `MoveExplorerToLocation(FVector)` → in Camp/POIScene, routes Camp click-to-move to the **possessed** `AFC_ExplorerCharacter` (no AIController). The controller delegates pathfinding + steering to the pawn instead of driving movement via an AI controller.

7. **Pause / resume behavior**

   * Handles ESC by consulting `UFCGameStateManager` and toggling pause state (via state stack) and pause UI through `UFCUIManager`. 

8. **Dev save/load shortcuts**

   * Implements `DevQuickSave()` / `DevQuickLoad()` using `UFCGameInstance::SaveGame(...)` and `LoadGameAsync(...)` with slot `"QuickSave"`. 

9. **Startup consistency on load**

   * Subscribes to `GameStateManager.OnStateChanged` and manually triggers `OnGameStateChanged(...)` on BeginPlay for certain states (Overworld_Travel / ExpeditionSummary / Camp_Local) so camera/input are correct after loading. 


## Public API

### Input / camera

* `SetInputMappingMode(EFCInputMappingMode NewMode)` → delegates mapping changes to `UFCInputManager`. 
* `SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime=2.0f)` → camera switching + cursor/input-mode cleanup. 
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

* `GetPossessedConvoy()` / `MoveConvoyToLocation(const FVector&)` → Overworld click-to-move sends commands to the convoy leader's `AAIController` (projected onto NavMesh).
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

**Delegated:** POI action selection, movement triggering, and overlap handling (`HandlePOIClick`, `OnPOIActionSelected`, `NotifyPOIOverlap`).
**Why:** centralizes POI interaction logic across Office, Overworld, and Camp; mode-aware movement (convoy vs explorer).
**Note:** Moved from `AFCFirstPersonCharacter` to `AFCPlayerController` to work across all scenes.

### 4) `UFCGameStateManager` (subsystem) — authoritative state + pause stack

**Delegated:** state transitions (`TransitionTo`, `PushState`, `PopState`), current state queries, and broadcasting changes.
**Why:** controller reacts to state (camera/input/cursor), but the *truth* of state lives in the state manager.

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
