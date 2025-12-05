# FC Runtime — Central Glossary & High-Level Context (Managers + Key Classes)

This document is a **map**: what each manager/class is for, how they connect, and where to look for details (**implementation lives in the referenced files**).

---

## 1) Big picture: who owns what (and when)?

### Lifecycle overview (boot → play → travel)

1. **GameInstance created (persistent across travel)**  
   `UFCGameInstance` is the **composition root**: it configures key subsystems (Level metadata table, UI widget classes + TableWidgetMap) and owns save/load + persistent expedition context.

2. **Level loads → GameMode BeginPlay (per-level)**  
   `AFCGameMode` is the level’s lightweight bootstrapper: sets default pawn/controller classes and kicks **LevelTransition finalization** on level start (`InitializeLevelTransitionOnLevelStart`) so “Loading → TargetState” flows complete reliably.

3. **PlayerController runtime (per player)**  
   `AFCPlayerController` is the glue for “how the player can interact right now”: input routing, camera mode switching (via CameraManager/InputManager), UI gating, and reacting to `UFCGameStateManager` state changes.

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
Details: `Managers/FCUIManager.md` → `Core/FCUIManager.h/.cpp`.

### `UFCLevelTransitionManager` — “Multi-system flows”
Orchestrates macro transitions: state → fade → travel → post-load finalization (e.g., Overworld → Office + Summary).  
Details: `Managers/FCLevelTransitionManager.md` → `Core/FCLevelTransitionManager.h/.cpp`.

### `UFCExpeditionManager` — “Expedition lifecycle + world-map exploration”
Owns current expedition state, fog-of-war reveal, route preview, autosave of exploration state; delegates grid/pathfinding to `FFCWorldMapExploration`.  
Details: `Managers/FCExpeditionManager.md` → `Expedition/FCExpeditionManager.h/.cpp`.

---

## 4) Per-level “bootstrap” class

### `AFCGameMode` — "Level-start kick + defaults"
- Sets default pawn `AFCFirstPersonCharacter` and controller `AFCPlayerController`.
- On BeginPlay, triggers `UFCLevelTransitionManager::InitializeLevelTransitionOnLevelStart()` to finalize Loading flows.
- **SpawnDefaultPawnAtTransform override:** Prevents duplicate pawn spawning in Camp/POI levels where explorer is already placed in the level (checks for existing `AFC_ExplorerCharacter` and returns nullptr).

Details: `FCGameMode.md` → `FCGameMode.h/.cpp`.

---

## 5) Player runtime layer — glossary (Controller + components + pawn)

### `AFCPlayerController` — "Input router + camera mode driver + UI gating"
- Routes input (Enhanced Input) and chooses behavior by camera mode (FirstPerson/TableView/TopDown/POIScene).
- Owns `UFCInputManager`, `UFCCameraManager`, and `UFCInteractionComponent` as ActorComponents.
- Commands movement across scenes: `MoveConvoyToLocation()` for Overworld, `MoveExplorerToLocation()` for Camp.
- Reacts to game state changes (camera + input mapping + cursor rules).
- Triggers UIManager flows (main menu, pause, table widgets, overworld HUD) and blocks "world interaction" when UI is active.

Details: `FCPlayerController.md` → `FCPlayerController.h/.cpp`.

### `UFCInputManager` (ActorComponent) — “Mapping contexts by mode”
Switches Enhanced Input mapping contexts (FirstPerson/TopDown/POIScene/etc.).  
Details: `Managers/FCInputManager.md` → `Components/FCInputManager.h/.cpp`.

### `UFCCameraManager` (ActorComponent) — “View-target blending by mode”
Encapsulates camera transitions (menu, first-person, table-object, overworld top-down, POI/camp); manages temp cameras and restores previous targets.  
Details: `Managers/FCCameraManager.md` → `Components/FCCameraManager.h/.cpp`.

### `UFCInteractionComponent` (ActorComponent) — "POI interaction + action selection orchestration"
- Lives on `AFCPlayerController` (not on pawns), making it available across all scenes (Office, Overworld, Camp).
- Detects interactables in front of the player (trace-based in Office), exposes `Interact()` for focused targets.
- Orchestrates POI click/overlap flows (Overworld + Camp): determine available actions → optionally open action selection UI (via `UFCUIManager`) → move convoy/explorer (via `AFCPlayerController`) → execute POI action on arrival.
- Mode-aware movement: calls `MoveConvoyToLocation()` in Overworld (TopDown), `MoveExplorerToLocation()` in Camp (POIScene).
- Shows/hides/updates an interaction prompt widget (requires a widget contract function like `SetInteractionPrompt`).
- Caches its owning `AFCPlayerController` in `OnRegister`/`BeginPlay` and uses `GetOwnerPCCheckedOrNull()` for all controller access (no `GetInstigatorController` / `GetFirstPlayerController`); logs a single high-signal error if mis-owned.
- Gates FirstPerson focus tracing and prompts via a boolean flag (`bFirstPersonFocusEnabled`) that is currently derived from `UFCCameraManager::GetCameraMode()`; when disabled it clears focus, hides the prompt, and early-outs from its trace/prompt update path so Overworld/Camp do not run FP traces.

Details: `Components/FCInteractionComponent.md` → `FCInteractionComponent.h/.cpp`.

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

Details: `FC_ExplorerCharacter.h/.cpp`.

---

## 6) Domain helpers (non-subsystem “model” types)

### `FFCWorldMapExploration` — “256×256 masks + conversions + BFS”
Reveal/Land masks, ID conversion helpers, default reveal application, and shortest path BFS with traversal rules.  
Details: `Managers/FCWorldMapExploration.md` → `WorldMap/FCWorldMapExploration.h/.cpp`.

---

## 7) Key cross-system contracts (how things connect)

### A) State → player experience
`UFCGameStateManager.OnStateChanged` → `AFCPlayerController` reacts (camera mode + input mapping + cursor/input mode) and calls `UFCUIManager` where needed.

### B) Level travel orchestration (fade + load + finalize)
`UFCLevelTransitionManager` sequences:  
`TransitionViaLoading(Target)` → `UFCTransitionManager` fade out → `UFCLevelManager` load/travel → post-load state/UI finalization.

### C) “UI blocks world interaction”
`UFCUIManager` maintains blocking widget state; `AFCPlayerController` and `UFCInteractionComponent` use it to prevent clicking/interacting through UI.

### D) Expedition world-map loop
`UFCExpeditionManager` updates reveal/fog + route preview + autosaves; delegates grid math/pathfinding to `FFCWorldMapExploration`.

### E) Level start finalization safety net
On every level BeginPlay, `AFCGameMode` triggers `InitializeLevelTransitionOnLevelStart()` to complete pending Loading transitions.

---

## 8) “Look up details here” (single-hop pointers)

- `UFCGameInstance`: `UFCGameInstance.md`
- `AFCGameMode`: `FCGameMode.md`
- `AFCPlayerController`: `FCPlayerController.md`

- Managers:  
  `FCTransitionManager.md`, `FCUIManager.md`, `FCGameStateManager.md`, `FCLevelManager.md`, `FCLevelTransitionManager.md`, `FCExpeditionManager.md`

- PlayerRuntime components / pawn:  
  `FCInputManager.md`, `FCCameraManager.md`, `FCInteractionComponent.md`, `FCFirstPersonCharacter.md`

- Domain helpers:  
  `FCWorldMapExploration.md`