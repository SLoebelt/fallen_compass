## UFCPlayerModeCoordinator — Technical Documentation

### Where to find it (paths)

- Header: `Components/FCPlayerModeCoordinator.h`
- Source: `Components/FCPlayerModeCoordinator.cpp`
- Related types:
  - `Core/FCPlayerModeTypes.h` — defines `EFCPlayerMode`.
  - `Components/Data/FCPlayerModeProfileSet.h/.cpp` — defines `UFCPlayerModeProfileSet`.

---

## Responsibilities (what this component owns)

`UFCPlayerModeCoordinator` is a non-ticking `UActorComponent` that lives on `AFCPlayerController`. It centralizes the subscription to `UFCGameStateManager::OnStateChanged`, maps low-level game states to higher-level player modes, and delegates presentation changes back to the controller.

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

### 3. Mode tracking + profile hook

- Maintains `CurrentMode` as an `EFCPlayerMode` (initialized to `Static`).
- Holds an optional `UFCPlayerModeProfileSet* ModeProfileSet` asset (Task 3 in the refactor plan) via `UPROPERTY(EditDefaultsOnly, Category="FC|Mode")`.
- `ApplyMode(EFCPlayerMode NewMode)`:
  - Early-outs if `NewMode == CurrentMode` to avoid redundant work.
  - Logs old mode → new mode and the name of the configured profile set (if any) using `LogFCPlayerModeCoordinator`.
  - Updates `CurrentMode`.
  - **Phase 1 note:** does not yet manipulate camera/input/interaction directly; that behavior remains in the controller helpers.

### 4. Phase 1 controller delegation

- `OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState)`:
  - Logs the state transition using `UEnum::GetValueAsString`.
  - Maps `NewState` to `EFCPlayerMode` and calls `ApplyMode(NewMode)`.
  - Casts its owner to `AFCPlayerController` and calls `AFCPlayerController::ApplyPresentationForGameState(OldState, NewState)`.
    - This preserves the existing camera/input/cursor logic inside the controller while centralizing the subscription and mapping logic in one place.

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
  - The concrete camera/input/cursor changes for each game state via `ApplyPresentationForGameState(OldState, NewState)`.
- **Why**
  - Keeps the coordinator focused on mapping and orchestration, while controller remains the place where camera and input policies are actually applied during Phase 1.

### `UFCPlayerModeProfileSet` (future integration)

- **What is delegated**
  - Data-driven configuration (`FPlayerModeProfile`) for each `EFCPlayerMode` (input config, camera mode, interaction profile, click policy, cursor/mouse lock settings).
- **Why**
  - Profiles will allow `UFCPlayerModeCoordinator` to eventually move from “tell controller what to do” to “apply profile-driven configuration directly to `UFCInputManager`, `UFCCameraManager`, and `UFCInteractionComponent` while keeping behavior data-driven.

---

## Notes and future phases

- **Phase 1 (current):**
  - Coordinator owns subscription + mapping and calls back into controller helpers.
  - No direct camera/input/interaction changes are performed here.
- **Later phases:**
  - `ApplyMode` will load and validate `FPlayerModeProfile` data from `UFCPlayerModeProfileSet` and call into `UFCInputManager`, `UFCCameraManager`, `UFCInteractionComponent`, and the controller for configuration only (no UI or gameplay actions).
