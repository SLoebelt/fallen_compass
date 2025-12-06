## AFCBaseGameMode and thin GameModes — Technical Documentation

### Where to find them (paths)

- Base:
  - Header: `Core/FCBaseGameMode.h`
  - Source: `Core/FCBaseGameMode.cpp`
- Office:
  - Header: `GameModes/FCOfficeGameMode.h`
  - Source: `GameModes/FCOfficeGameMode.cpp`
- Camp:
  - Header: `GameModes/FCCampGameMode.h`
  - Source: `GameModes/FCCampGameMode.cpp`
- Overworld:
  - Header: `GameModes/FCOverworldGameMode.h`
  - Source: `GameModes/FCOverworldGameMode.cpp`
- Legacy:
  - Monolithic `AFCGameMode` remains in `Core/FCGameMode.h/.cpp` for migration notes and any remaining maps still using it.

---

## Responsibilities (what these GameModes own)

### `AFCBaseGameMode` — global level-start kick

`AFCBaseGameMode` is the shared base class for all per-scene GameModes. It primarily owns:

1. **Global BeginPlay hook**

  * Overrides `BeginPlay()` to call `Super::BeginPlay()` and then perform global startup work that should run for every level.

2. **Boot-time diagnostics**

  * Logs the active GameMode class and current map name using the `LogFallenCompassGameMode` category.

3. **Level-start transition finalization hook**

  * On BeginPlay, retrieves `UFCLevelTransitionManager` from the GameInstance and calls `InitializeOnLevelStart()` to finalize any pending transitions.

It does **not** reference Office/Camp/Overworld-specific pawns or actors.

---

### `AFCOfficeGameMode` — Office defaults + optional menu camera

`AFCOfficeGameMode` derives from `AFCBaseGameMode` and defines the Office scene contract:

1. **Template defaults for pawn + controller**

  * In its constructor, sets `DefaultPawnClass = AFCFirstPersonCharacter::StaticClass()`.
  * Sets `PlayerControllerClass = AFCPlayerController::StaticClass()`.

2. **Menu camera wiring (optional)**

  * In `BeginPlay()`, if `MenuCameraActor` is not set explicitly, searches for an `ACameraActor` tagged `MenuCamera`, assigns it, and logs a warning that a tag-based fallback was used.
  * Leaves further Office-specific wiring to future milestones.

---

### `AFCCampGameMode` — Camp defaults + pre-placed explorer pattern

`AFCCampGameMode` derives from `AFCBaseGameMode` and defines the Camp scene contract:

1. **Camp pawn + controller defaults**

  * In its constructor, sets `DefaultPawnClass = AFC_ExplorerCharacter::StaticClass()` (can be treated as a fallback if a level forgets to place an explorer).
  * Sets `PlayerControllerClass = AFCPlayerController::StaticClass()`.

2. **Camp camera wiring**

  * Exposes an editor-visible `CampCameraActor`.
  * In `BeginPlay()`, if `CampCameraActor` is not set, searches for an `ACameraActor` tagged `CampCamera`, assigns it, and logs a warning when a tag-based fallback is used.
  * If a `CampCameraActor` is available, finds the player controller and calls `SetPOISceneCameraActor` so the camera manager can treat it as the Camp/POI camera.

3. **Pre-placed explorer safety net**

  * In `BeginPlay()`, if the player controller currently has no pawn, searches for an `AFC_ExplorerCharacter` already placed in the level.
  * If found, possesses that pawn so the player controls the pre-placed explorer instead of spawning a duplicate.

The older `SpawnDefaultPawnAtTransform`-override pattern for skipping explorer spawn has been superseded by this more explicit possession logic (see commented-out implementation in code for reference).

---

### `AFCOverworldGameMode` — Overworld convoy + camera wiring

`AFCOverworldGameMode` derives from `AFCBaseGameMode` and defines the Overworld scene contract:

1. **Controller default**

  * In its constructor, leaves `DefaultPawnClass` as `nullptr` (convoy is level-placed) and sets `PlayerControllerClass = AFCPlayerController::StaticClass()`.

2. **Convoy wiring**

  * Exposes an editor-visible `DefaultConvoy` (`AFCOverworldConvoy*`).
  * In `BeginPlay()`, if `DefaultConvoy` is not set explicitly, performs a one-time `GetAllActorsOfClass` search for `AFCOverworldConvoy`, assigns the first result, and logs a warning when a fallback is used; logs an error if no convoy is found.
  * After resolving `DefaultConvoy`, obtains the player controller and calls `SetActiveConvoy(DefaultConvoy)` so the controller can bind convoy delegates without doing its own global search.

3. **Overworld camera wiring**

  * Exposes an editor-visible `OverworldCameraActor`.
  * In `BeginPlay()`, if `OverworldCameraActor` is not set, searches for an `ACameraActor` tagged `OverworldCamera`, assigns it, and logs a warning when resolved by tag.

---

### Legacy `AFCGameMode` — monolithic predecessor

`AFCGameMode` (in `Core/FCGameMode.h/.cpp`) is the older monolithic GameMode that mixed Office defaults, Camp spawn rules, and global level-start behavior. It is kept only for migration notes and for any remaining maps that still reference it; new work should favor `AFCBaseGameMode` + the thin per-level GameModes above.