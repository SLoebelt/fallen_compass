# FC Runtime — Central Glossary & High-Level Context (Managers + Key Classes)

This document is a **map**: what each manager/class is for, how they connect, and where to look for details (**implementation lives in the referenced files**).

---

## 1) Big picture: who owns what (and when)?

### Lifecycle overview (boot → play → travel)

1. **GameInstance created (persistent across travel)**  
   `UFCGameInstance` is the **composition root**: it configures key subsystems (Level metadata table, UI widget classes + TableWidgetMap) and owns save/load + persistent expedition context.

2. **Level loads → GameMode BeginPlay (per-level)**  
   `AFCBaseGameMode` is the shared level bootstrap: it logs level start and kicks **LevelTransition finalization** on level start via `UFCLevelTransitionManager::InitializeOnLevelStart()` so “Loading → TargetState” and other flows complete reliably. Thin per-level GameModes (Office/Camp/Overworld) derive from this base and provide scene-specific defaults.

3. **PlayerController runtime (per player)**  
  `AFCPlayerController` is the glue for “how the player can interact right now”: input routing, camera mode switching (via CameraManager/InputManager), UI gating, and reacting to `UFCGameStateManager` state changes. `UFCPlayerModeCoordinator` (a controller-owned component) subscribes to `UFCGameStateManager::OnStateChanged`, maps game state → `EFCPlayerMode`, looks up an `FPlayerModeProfile` in a `UFCPlayerModeProfileSet` data asset, and applies the resulting profile (camera, input mapping mode, cursor/input mode) in an idempotent, fail-soft way.

### Architecture layers

- **GameInstance Subsystems ("Managers")**: persistent orchestration (UI, transitions, state, level travel, expeditions).
- **Per-level gameplay classes**: `AFCGameMode` and level actors/cameras configured in the map.
- **PlayerRuntime (Controller + Components + Pawns)**:  
  `AFCPlayerController` + `UFCInputManager` + `UFCCameraManager` + `UFCInteractionComponent` + per-scene pawns (`AFCFirstPersonCharacter` for Office, `AFCOverworldConvoy` for Overworld, `AFC_ExplorerCharacter` for Camp).

---

## 2) Composition root & persistence

### `UFCGameInstance` — “Config + persistence hub”

- Configures:
  - `UFCLevelManager` with `LevelMetadataTable`
  - `UFCUIManager` with widget classes + `TableWidgetMap`
- Owns:
  - expedition context (dirty flag + change event)
  - economy state (supplies/money/day)
  - save/load orchestration (including cross-level load via fade + delayed `OpenLevel`)

Details: `UFCGameInstance.md` → `UFCGameInstance.h/.cpp`.

---

## 3) Core managers (UGameInstanceSubsystem) — glossary

### `UFCGameStateManager` — “What mode is the game in?”
Authoritative state machine + valid transitions + state stack; emits `OnStateChanged` that others react to.  
Details: `Managers/FCGameStateManager.md` → `Core/FCGameStateManager.h/.cpp`.

### `UFCLevelManager` — “Where are we, and how do we load levels?”
Tracks normalized current/previous level + type (data-driven via metadata table); loads levels using fade-out handshake.  
Details: `Managers/FCLevelManager.md` → `Core/FCLevelManager.h/.cpp`.

### `UFCTransitionManager` — “Persistent fade overlay”
Fade in/out layer that survives travel; emits fade completion events used as travel handoff.  
Details: `Managers/FCTransitionManager.md` → `Core/FCTransitionManager.h/.cpp`.

### `UFCUIManager` — “Widget lifecycle + UI ↔ gameplay mediation”
Central widget creation/show/hide, table-widget registry, blocking widgets, POI action selection mediator.  
Delegates world-input gating to `UFCUIBlockSubsystem` by registering/unregistering blocking widgets (pause, tables, POI selection, overworld map, expedition summary), and stays view-only for POI actions (shows selection UI and forwards the chosen `EFCPOIAction` back to `UFCInteractionComponent`).  
Details: `Managers/FCUIManager.md` → `Core/FCUIManager.h/.cpp`.

### `UFCLevelTransitionManager` — “Multi-system flows”
Orchestrates macro transitions: state → fade → travel → post-load finalization (e.g., Overworld → Office + Summary).  
Details: `Managers/FCLevelTransitionManager.md` → `Core/FCLevelTransitionManager.h/.cpp`.

### `UFCExpeditionManager` — “Expedition lifecycle + world-map exploration”
Owns current expedition state, fog-of-war reveal, route preview, autosave of exploration state; delegates grid/pathfinding to `FFCWorldMapExploration`.  
Details: `Managers/FCExpeditionManager.md` → `Expedition/FCExpeditionManager.h/.cpp`.

### `UFCUIBlockSubsystem` — “World input gating (click vs interact)”
Tracks which widgets currently block world input and exposes cached `CanWorldClick()` / `CanWorldInteract()` queries. Widgets register/unregister as blockers; `AFCPlayerController` and `UFCInteractionComponent` use this to avoid clicking/interacting through modal UI.  
Details: `Managers/FCUIBlockSubsystem.md` → `Core/FCUIBlockSubsystem.h/.cpp`.

---

## 4) Per-level “bootstrap” classes

### `AFCBaseGameMode` + thin per-level GameModes — "Level-start kick + per-scene defaults"
- `AFCBaseGameMode` (in `Core/FCBaseGameMode.h/.cpp`) owns the global level-start hook:
  - Calls `Super::BeginPlay()`.
  - Logs the active map and GameMode class.
  - Retrieves `UFCLevelTransitionManager` from the GameInstance and calls `InitializeOnLevelStart()`.
- Thin per-level GameModes derive from this base and define scene contracts:
  - `AFCOfficeGameMode` (in `GameModes/FCOfficeGameMode.h/.cpp`) sets Office defaults (pawn `AFCFirstPersonCharacter`, controller `AFCPlayerController` and optional Office-specific actors/cameras).
  - `AFCCampGameMode` (in `GameModes/FCCampGameMode.h/.cpp`) configures Camp defaults and owns the “pre-placed explorer” pattern via a `SpawnDefaultPawnAtTransform` override that skips spawning if an `AFC_ExplorerCharacter` already exists; it also exposes an editor-visible `CampCameraActor` reference.
  - `AFCOverworldGameMode` (in `GameModes/FCOverworldGameMode.h/.cpp`) wires Overworld expectations, exposing an editor-visible `DefaultConvoy` (`AFCOverworldConvoy*`) and optional `OverworldCameraActor`, and hands the selected convoy to `AFCPlayerController` via a dedicated API instead of scanning globally.

Details: `FCGameMode.md` — documents the legacy monolithic `AFCGameMode` and points at the new `FCBaseGameMode` + thin GameModes under `Core/` and `GameModes/`.

---

## 5) Player runtime layer — glossary (Controller + components + pawn)

### `AFCPlayerController` — "Input router + movement commands + UI gating"
- Routes input (Enhanced Input) and forwards actions to systems like `UFCInteractionComponent`, `UFCGameInstance`, and `UFCUIManager`.
- Owns `UFCInputManager`, `UFCCameraManager`, `UFCInteractionComponent`, and `UFCPlayerModeCoordinator` as ActorComponents.
- Commands movement across scenes: `MoveConvoyToLocation()` for Overworld, `MoveExplorerToLocation()` for Camp.
- Does **not** own per-state camera/input/cursor defaults anymore; those are applied by `UFCPlayerModeCoordinator` from `FPlayerModeProfile`. Its `SetCameraModeLocal` helper is camera-only.
- Uses a slim `ApplyPresentationForGameState` hook for event wiring only (e.g., binding/unbinding Overworld convoy delegates on state transitions), avoiding duplicate configuration paths.
- Uses explicit world-input queries `CanWorldClick()` / `CanWorldInteract()` backed by `UFCUIBlockSubsystem` so click-to-move and "E interact" are correctly blocked when modal UI is active.
- Triggers UIManager flows (main menu, pause, table widgets, overworld HUD) but keeps all POI action and movement decisions in `UFCInteractionComponent`.
- Treats the overworld convoy and camp explorer as **event sources**: binds their POI-overlap delegates to the controller-owned `UFCInteractionComponent` (no pawn crawling or global controller lookups).

Details: `FCPlayerController.md` → `FCPlayerController.h/.cpp`.

### `UFCInputManager` (ActorComponent) — “Mapping contexts by mode”
Switches Enhanced Input mapping contexts (FirstPerson/TopDown/Fight/StaticScene/POIScene) using data from a shared `UFCInputConfig` asset.  
Details: `Managers/FCInputManager.md` → `Components/FCInputManager.h/.cpp`.

### `UFCCameraManager` (ActorComponent) — “View-target blending by mode”
Encapsulates camera transitions (menu, first-person, table-object, overworld top-down, POI/camp); manages temp cameras and restores previous targets.  
Details: `Managers/FCCameraManager.md` → `Components/FCCameraManager.h/.cpp`.

### `UFCInteractionComponent` (ActorComponent) — “Central interaction orchestrator”
- Lives on `AFCPlayerController` and mediates interaction across Office, Overworld, and Camp.
- Owns the POI interaction state machine (click → optional selection → move → arrival → execute) and the pending state it needs (POI, action, selection/arrival flags).
- Issues movement **indirectly** via controller helpers (`MoveConvoyToLocation` / `MoveExplorerToLocation`) and executes POI actions on arrival in an idempotent way.
- Also handles FirstPerson “look at + interact” for Office, but keeps that logic separate from POI flows.

Details: `TechnicalDocumentation/Components/FCInteractionComponent.md` → `FCInteractionComponent.h/.cpp` (full state machine + click/arrival API details).

### `UFCPlayerModeCoordinator` (ActorComponent) — "Game state → player mode → profile application"
- Lives on `AFCPlayerController` as a non-ticking component.
- Subscribes to `UFCGameStateManager::OnStateChanged` and logs OldState → NewState transitions.
- Maps `EFCGameStateID` values to a higher-level `EFCPlayerMode` enum (Office / Overworld / Camp / Static) and tracks the current player mode.
- Looks up an `FPlayerModeProfile` for the current mode in a `UFCPlayerModeProfileSet` data asset and applies the resulting profile directly: camera mode (via `SetCameraModeLocal` / camera manager), input mapping mode (via `UFCInputManager`), and cursor/input mode, in an idempotent, fail-soft way.

Details: `FCPlayerModeCoordinator.md` → `Components/FCPlayerModeCoordinator.h/.cpp` (plus `Core/FCPlayerModeTypes.h`).

### `AFCFirstPersonCharacter` — "Office exploration pawn"
- First-person camera + movement tuning + look pitch clamping.
- Used only in Office scenes (FirstPerson mode).
- Forwards Interact input to `UFCInteractionComponent` on `AFCPlayerController`.
- Loads character mesh and optionally applies a default anim blueprint.

Details: `Characters/FCFirstPersonCharacter.md` → `FCFirstPersonCharacter.h/.cpp`.

### `AFC_ExplorerCharacter` — "Camp exploration pawn (player-controlled)"
- Top-down camera mode pawn for Camp scenes (POIScene mode).
- Now possessed by `AFCPlayerController` in Camp (standard pawn possession; no dedicated AIController required for movement).
- Camp click-to-move is routed from `AFCPlayerController::HandleClick` to `MoveExplorerToLocation`, which delegates to `AFC_ExplorerCharacter::MoveExplorerToLocation`.
- `AFC_ExplorerCharacter::MoveExplorerToLocation` computes a NavMesh path (via `UNavigationSystemV1::FindPathToLocationSynchronously`) and stores path points; `Tick` then steers along this path using `AddMovementInput` until the destination is reached.
- Static Camp/POI camera is handled by `UFCCameraManager::BlendToPOISceneCamera`, which ultimately calls `SetViewTargetWithBlend` on the owning controller, targeting a level-placed `ACameraActor` tagged `CampCamera` (or matching that name pattern).

Details: `Characters/FC_ExplorerCharacter.md` → `FC_ExplorerCharacter.h/.cpp`.

### `AFCOverworldConvoy` + `AFCConvoyMember` — "Overworld convoy pivot + walkers"

- `AFCOverworldConvoy` is the **authoritative convoy pivot** in Overworld: world map revelation and camera systems continue to treat its actor transform as “the convoy position”.
- On `Tick`, the convoy actor follows the **bounding box center** that encloses all valid convoy members, with optional smoothing, so cameras and world-map sampling stay centered on the group instead of a single pawn.
- Convoy movement is driven manually via `UCharacterMovementComponent` (no AI `MoveTo`):
  - The **leader** `AFCConvoyMember` computes a NavMesh path using `UNavigationSystemV1::FindPathToLocationSynchronously` and follows it in `Tick` using `AddMovementInput(..., bForce=true)`.
  - **Followers** do not own their own paths; they follow the leader using a local-space offset (e.g. behind/in formation), also via `AddMovementInput`.
- `AFCOverworldConvoy` exposes a single movement API surface:
  - `MoveConvoyToLocation(const FVector& TargetLocation)` → asks the leader member to start path-follow to `TargetLocation`.
  - `StopConvoy()` → stops leader path-follow and follower following (used by POI overlap handling and interaction system).
- `AFCPlayerController` routes Overworld click-to-move into `ActiveConvoy->MoveConvoyToLocation(...)`; no AI controllers are required for convoy members, and `GetCharacterMovement()->bRunPhysicsWithNoController` is used so unpossessed convoy characters still respond to `AddMovementInput`.

Details: `Characters/FCOverworldConvoy.md`, `Characters/FCConvoyMember.md` → `FCOverworldConvoy.h/.cpp`, `FCConvoyMember.h/.cpp`.

---

## 6) Domain helpers (non-subsystem “model” types)

### `FFCWorldMapExploration` — “256×256 masks + conversions + BFS”
Reveal/Land masks, ID conversion helpers, default reveal application, and shortest path BFS with traversal rules.  
Details: `Managers/FCWorldMapExploration.md` → `WorldMap/FCWorldMapExploration.h/.cpp`.

---

## 7) Key cross-system contracts (how things connect)

### A) State → player experience
`UFCGameStateManager.OnStateChanged` → `UFCPlayerModeCoordinator` (on the player controller) reacts, logs the transition, maps to `EFCPlayerMode`, and applies the appropriate `FPlayerModeProfile` (camera mode, input mapping mode, cursor/input mode) directly. `AFCPlayerController` remains responsible for movement commands and UI flows, but no longer re-derives camera/input/cursor defaults per state.

### B) Level travel orchestration (fade + load + finalize)
`UFCLevelTransitionManager` sequences:  
`TransitionViaLoading(Target)` → `UFCTransitionManager` fade out → `UFCLevelManager` load/travel → post-load state/UI finalization.

### C) “UI blocks world interaction”
`UFCUIManager` maintains blocking widget state; `AFCPlayerController` and `UFCInteractionComponent` use it to prevent clicking/interacting through UI.

### D) Expedition world-map loop
`UFCExpeditionManager` updates reveal/fog + route preview + autosaves; delegates grid math/pathfinding to `FFCWorldMapExploration`.

### E) Level start finalization safety net
On every level BeginPlay, `AFCBaseGameMode` (base of all per-scene GameModes) triggers `UFCLevelTransitionManager::InitializeOnLevelStart()` to complete any pending transitions.

---

## 8) “Look up details here” (single-hop pointers)

- `UFCGameInstance`: `UFCGameInstance.md`
- `AFCBaseGameMode` + thin GameModes: `FCGameMode.md`
- `AFCPlayerController`: `FCPlayerController.md`

- Managers:  
  `FCTransitionManager.md`, `FCUIManager.md`, `FCGameStateManager.md`, `FCLevelManager.md`, `FCLevelTransitionManager.md`, `FCExpeditionManager.md`

- PlayerRuntime components / pawn:  
  `FCInputManager.md`, `FCCameraManager.md`, `FCInteractionComponent.md`, `FCFirstPersonCharacter.md`

- Domain helpers:  
  `FCWorldMapExploration.md`