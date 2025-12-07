## UFCPlayerModeCoordinator — Technical Documentation

### Where to find it (paths)

- Header: `Components/FCPlayerModeCoordinator.h`
- Source: `Components/FCPlayerModeCoordinator.cpp`
- Related types:
  - `Core/FCPlayerModeTypes.h` — defines `EFCPlayerMode`.
  - `Components/Data/FCPlayerModeProfileSet.h/.cpp` — defines `UFCPlayerModeProfileSet`.

---

## Responsibilities (what this component owns)

`UFCPlayerModeCoordinator` is a non-ticking `UActorComponent` that lives on `AFCPlayerController`. It centralizes the subscription to `UFCGameStateManager::OnStateChanged`, maps low-level game states to higher-level player modes, looks up `FPlayerModeProfile` data in a `UFCPlayerModeProfileSet` asset, and applies the resulting profile (camera, input mapping mode, cursor/input mode) in an idempotent, fail-soft way.

### 1. Game state subscription

- On `BeginPlay()`:
  - Verifies that its owner is a local `APlayerController`.
  - Retrieves the `UFCGameInstance` from the controller.
  - Obtains `UFCGameStateManager` via `GetSubsystem<UFCGameStateManager>()` and caches it in a `TWeakObjectPtr`.
  - Subscribes to `UFCGameStateManager::OnStateChanged` via `AddDynamic(this, &UFCPlayerModeCoordinator::OnGameStateChanged)`.
  - Reads the current `EFCGameStateID` and schedules a one-tick-delayed call to `OnGameStateChanged(EFCGameStateID::None, Current)` using `FTimerManager::SetTimerForNextTick`, so that presentation is applied once everything is initialized.
- On `EndPlay()`:
  - If the cached state manager is still valid, unsubscribes from `OnStateChanged` with `RemoveDynamic`.

### 2. Game state → player mode mapping

- Exposes a helper `MapStateToMode(EFCGameStateID State) const` that translates runtime game states into a coarse `EFCPlayerMode`:
  - Office modes (`Office_Exploration`, `Office_TableView`) → `EFCPlayerMode::Office`.
  - Overworld/combat (`Overworld_Travel`, `Combat_PlayerTurn`, `Combat_EnemyTurn`) → `EFCPlayerMode::Overworld`.
  - Camp local (`Camp_Local`) → `EFCPlayerMode::Camp`.
  - Global/static contexts (`MainMenu`, `Paused`, `Loading`, `ExpeditionSummary`) → `EFCPlayerMode::Static`.
- Logs a warning and falls back to `EFCPlayerMode::Static` for any unhandled `EFCGameStateID`.

### 3. Mode tracking + profile lookup

- Maintains `CurrentMode` as an `EFCPlayerMode` (initialized to `Static`).
- Holds an optional `UFCPlayerModeProfileSet* ModeProfileSet` asset via `UPROPERTY(EditDefaultsOnly, Category="FC|Mode")`.
- Exposes a helper `GetProfileForMode(EFCPlayerMode Mode, FPlayerModeProfile& OutProfile) const` that:
  - Returns `false` and logs a **Warning** if `ModeProfileSet` is null or no profile exists for the requested mode.
  - Returns `true` and copies out the profile otherwise.

### 4. Profile-driven, idempotent `ApplyMode`

- `ApplyMode(EFCPlayerMode NewMode)` implements the profile application pipeline:
  - Looks up the profile with `GetProfileForMode`; logs and returns early if none is found.
  - Validates the profile (at minimum ensuring the camera mode is a known value), collecting problems into a small array and logging them as warnings without crashing.
  - Derives an `EFCInputMappingMode` from `Profile.CameraMode` (e.g., FirstPerson, TopDown, POIScene, StaticScene) via a tiny helper.
  - Obtains the owning `AFCPlayerController` and applies input mapping mode through `UFCInputManager` (which clears and reapplies contexts each time).
  - Applies cursor visibility and input mode from the profile (GameOnly/GameAndUI/UIOnly) via an internal helper, setting `bShowMouseCursor`, `bEnableClickEvents`, and using `SetInputMode` appropriately.
  - Calls `SetCameraModeLocal(Profile.CameraMode, BlendTime)` on the controller, relying on it to be camera-only (no input/cursor side effects).
  - Logs old mode → new mode and the profile asset name using `LogFCPlayerModeCoordinator`.
  - Updates `CurrentMode`.
- Reapplying the same mode is safe: all of the above operations are written to be idempotent (no stacked mappings, no duplicated camera state).

---

## Public API and lifecycle

### Construction

- `UFCPlayerModeCoordinator()`
  - Disables ticking (`PrimaryComponentTick.bCanEverTick = false`).

### Lifecycle

- `BeginPlay()`
  - Performs safety checks on owner/game instance.
  - Caches and subscribes to `UFCGameStateManager`.
  - Schedules a next-tick call to `OnGameStateChanged(EFCGameStateID::None, CurrentState)` to bootstrap presentation.
- `EndPlay(const EEndPlayReason::Type EndPlayReason)`
  - Unsubscribes from `OnStateChanged` if the cached state manager is still valid.

---

## Connected systems (who it talks to and why)

### `UFCGameStateManager` — authoritative game state

- **What is delegated**
  - Emitting `OnStateChanged(OldState, NewState)` whenever the game state machine transitions.
- **Why**
  - Coordinator needs a single source of truth for overall game state to decide which player mode/profile should be active.

### `AFCPlayerController` — presentation application

- **What is delegated**
  - Implementing camera behavior for each `EFCPlayerCameraMode` via `SetCameraModeLocal`, and exposing access to `UFCInputManager` for mapping-mode changes.
- **Why**
  - Keeps camera blending logic and input binding rooted in the controller, while the coordinator decides *when* and *which* profile to apply.

### `UFCPlayerModeProfileSet` — profile source

- **What is delegated**
  - Data-driven configuration (`FPlayerModeProfile`) for each `EFCPlayerMode` (camera mode, optional input config override, interaction profile placeholder, click policy, cursor/mouse lock settings).
- **Why**
  - Profiles keep mode definitions designer-editable and centralize configuration data; the coordinator is the only place that interprets and applies them to runtime systems.

---

## Notes

- `InputConfig` in `FPlayerModeProfile` is treated as an optional/future override; the primary config remains assigned on `UFCInputManager` in editor. `UFCInputManager::SetInputMappingMode` guards against a missing config and logs errors instead of crashing.
- `EFCClickPolicy` and `InteractionProfile` are defined on the profile for future work (click-intent routing and interaction tuning), but are not yet applied by the coordinator.
