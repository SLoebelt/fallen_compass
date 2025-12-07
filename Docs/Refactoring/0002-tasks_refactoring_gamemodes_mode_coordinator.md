# 0002 — Refactoring GameModes + Player Mode Coordinator

## Relevant Files

- `/Docs/UE_NamingConventions.md` — Unreal-specific naming and folder structure guidance.
- `/Docs/UE_CodeConventions.md` — Unreal-specific coding conventions.
- `/Docs/Fallen_Compass_GDD_v0.2.md` — Game Design Document.
- `/Docs/DRM_Draft.md` — Development Roadmap.
- `/Docs/TechnicalDocumentation/FCRuntime.md` — Runtime glossary and ownership map.
- `/Docs/Refactoring/0002_refactoring_gamemodes_mode_coordinator.md` — PRD for this refactor.
- `/Docs/Prompts/task-template-schema.md` — Task structure schema.
- `/FC/Source/FC/Core/FCGameMode.h/.cpp` — Current GameMode implementation.
- `/FC/Source/FC/Core/FCPlayerController.h/.cpp` — Player controller and current mode logic.
- `/FC/Source/FC/Components/FCInputManager.h/.cpp` — Input mapping contexts.
- `/FC/Source/FC/Components/FCCameraManager.h/.cpp` — Camera mode and blending.
- `/FC/Source/FC/Interaction/FCInteractionComponent.h/.cpp` — Interaction orchestration.
- `/FC/Source/FC/World/FCOverworldConvoy.h/.cpp` — Overworld convoy (for references from GameMode).
- `/FC/Source/FC/Characters/FCFirstPersonCharacter.h/.cpp` — Office pawn.
- `/FC/Source/FC/Characters/FC_ExplorerCharacter.h/.cpp` — Camp pawn.
- `/FC/Source/FC/Core/FCBaseGameMode.h/.cpp` — New base GameMode (to be created).
- `/FC/Source/FC/GameModes/FCOfficeGameMode.h/.cpp` — New thin Office GameMode (to be created).
- `/FC/Source/FC/GameModes/FCCampGameMode.h/.cpp` — New thin Camp GameMode (to be created).
- `/FC/Source/FC/GameModes/FCOverworldGameMode.h/.cpp` — New thin Overworld GameMode (to be created).
- `/FC/Source/FC/Components/FCPlayerModeCoordinator.h/.cpp` — New mode coordinator (to be created).
- `/FC/Config/DefaultEngine.ini` — Default GameModes and map overrides.

## Interaction Model

- This task list is designed for a **junior Unreal Engine developer**.
- Work should be done in **weekly iterations**, one major task block per week when possible.
- Follow `/Docs/Prompts/task-template-schema.md` and the UE conventions docs for naming and style.

---

## 1. Create Base and Thin Per-Level GameModes

### 1.1 Introduce `AFCBaseGameMode` with shared helpers

1.1.1 Inspect `AFCGameMode` in `FCGameMode.h/.cpp` and `FCRuntime.md` to list:
- Current default pawn/controller setup.
- Level-transition initialization (e.g., calls to `UFCLevelTransitionManager::InitializeLevelTransitionOnLevelStart`).
- Any scene-specific logic (Office/Camp/Overworld assumptions).

1.1.2 Create `FCBaseGameMode.h/.cpp` under `Core/` that defines `AFCBaseGameMode : public AGameModeBase` (or `AGameMode` matching the existing base).

1.1.3 Move shared “level-start kick” logic from `AFCGameMode::BeginPlay` into `AFCBaseGameMode::BeginPlay`, keeping:
- Call to `Super::BeginPlay()`.
- Call to `UFCLevelTransitionManager::InitializeOnLevelStart()` (preferred entry point; this can delegate internally to `InitializeLevelTransitionOnLevelStart()` as needed).

1.1.4 Add a minimal logging line in `AFCBaseGameMode::BeginPlay` (Log level) to confirm it ran, including the map name and class.

1.1.5 Ensure `AFCBaseGameMode` does **not** reference Office/Camp/Overworld-specific pawns or actors.

**Acceptance criteria**
- `AFCBaseGameMode` compiles and runs as a drop-in replacement for existing `AFCGameMode` behavior for level-start initialization.
- No Office/Camp/Overworld branching appears in `AFCBaseGameMode`.

---

### 1.2 Implement `AFCOfficeGameMode` as a thin Office default GameMode

1.2.1 Create `FCOfficeGameMode.h/.cpp` in `GameModes/`, deriving from `AFCBaseGameMode`.

1.2.2 In the constructor, set:
- `DefaultPawnClass = AFCFirstPersonCharacter::StaticClass()`.
- `PlayerControllerClass = AFCPlayerController::StaticClass()`.

1.2.3 Add `UPROPERTY(EditInstanceOnly, Category="FC|Office")` references for Office-specific actors (e.g. desk actor, optional office camera), leaving them optional for now.

1.2.4 Override `BeginPlay` to:
- Call `Super::BeginPlay()`.
- If any explicit Office references are set, forward them to the appropriate managers (e.g., register a specific office camera with `UFCCameraManager` if that’s part of your design for later PRDs).
- Keep total logic under ~50 lines.

1.2.5 Update any Office maps’ World Settings or `DefaultEngine.ini` overrides so that Office levels use `AFCOfficeGameMode`.

**Acceptance criteria**
- Office PIE runs use `AFCOfficeGameMode` and still spawn `AFCFirstPersonCharacter` + `AFCPlayerController` as before.
- `AFCOfficeGameMode` remains under ~50 lines and only adds Office defaults, not global behavior.

---

### 1.3 Implement `AFCCampGameMode` as a thin Camp default GameMode

1.3.1 Create `FCCampGameMode.h/.cpp` in `GameModes/`, deriving from `AFCBaseGameMode`.

1.3.2 In the constructor, set:
- `DefaultPawnClass = AFC_ExplorerCharacter::StaticClass()` (or leave null if explorer is level-placed and spawn is suppressed by GameMode, as per current behavior).
- `PlayerControllerClass = AFCPlayerController::StaticClass()`.

1.3.3 Add `UPROPERTY(EditInstanceOnly, Category="FC|Camp")` fields for:
- `ACameraActor* CampCameraActor` (primary POIScene camera).
- Optional camp root/POI root actors if useful for later wiring.

1.3.4 In `BeginPlay`:
- Call `Super::BeginPlay()`.
- If `CampCameraActor` is assigned, obtain the player controller and call into `UFCCameraManager` (`SetPOISceneCameraActor` or equivalent) to register it.
- If explorer is already level-placed, ensure this GameMode cooperates with the existing `SpawnDefaultPawnAtTransform` override pattern (no duplicate pawn spawn).

1.3.5 Update Camp maps’ World Settings / config entries so they use `AFCCampGameMode`.

**Acceptance criteria**
- Camp PIE uses `AFCCampGameMode`.
- Explorer possession and static Camp camera behavior remain correct.
- No Camp-specific logic remains in any non-Camp GameMode.

---

### 1.4 Implement `AFCOverworldGameMode` as a thin Overworld default GameMode

1.4.1 Create `FCOverworldGameMode.h/.cpp` in `GameModes/`, deriving from `AFCBaseGameMode`.

1.4.2 In the constructor, set:
- `DefaultPawnClass` as appropriate for Overworld (may be null if convoy is level-placed only).
- `PlayerControllerClass = AFCPlayerController::StaticClass()`.

1.4.3 Add `UPROPERTY(EditInstanceOnly, Category="FC|Overworld")` references for:
- `AFCOverworldConvoy* DefaultConvoy`.
- Optional `ACameraActor* OverworldCamera` if needed.

1.4.4 In `BeginPlay`:
- Call `Super::BeginPlay()`.
- If `DefaultConvoy` is set, find the `AFCPlayerController` and either:
  - Set `ActiveConvoy` via a public controller API, or
  - Call a new helper that lets the controller bind convoy delegates using this reference.
- If `DefaultConvoy` is not set, optionally perform a one-time `UGameplayStatics::GetAllActorsOfClass` search, log a **Warning**, and set it.

1.4.5 Remove any convoy resolution/binding logic from `AFCPlayerController::BeginPlay` that was previously global; update it to rely on the GameMode wiring.

**Acceptance criteria**
- Overworld PIE uses `AFCOverworldGameMode` and a convoy is correctly assigned and bound.
- `AFCPlayerController::BeginPlay` no longer scans for convoy actors itself.

---

### 1.5 Expose editor-visible properties on each GameMode for key actors

1.5.1 Review Office/Camp/Overworld flows to list all “implicit” dependencies currently resolved by tags or names (e.g., `CampCamera`, `BP_OfficeDesk`).

1.5.2 For each thin GameMode, add `EditInstanceOnly` `UPROPERTY`s for those key actors, grouped by category (Office, Camp, Overworld) as appropriate.

1.5.3 In each GameMode’s `BeginPlay`, prefer explicit references; only fall back to tag-based or name-based lookups if the property is unset.

1.5.4 Add clear log messages when a fallback search is used, to encourage designers to set explicit references.

1.5.5 Document these properties briefly in comments and (optionally) in `FCRuntime.md` under the PlayerRuntime section.

**Acceptance criteria**
- Designers can wire the main office/camp/overworld actors via GameMode properties instead of relying on tags.
- When properties are unset, the existing tag-based behaviors still work but emit warnings.

---

### 1.6 Wire defaults in project settings and map overrides

1.6.1 Update `DefaultEngine.ini` as needed so that the project default GameMode stays general and maps override to the specific thin GameModes.

1.6.2 For each key map (Office, Camp, Overworld), set the `GameMode Override` in World Settings to the correct thin GameMode.

1.6.3 Validate in PIE for each map:
- Correct GameMode class is instantiated.
- Correct pawn and controller are used.

1.6.4 Update `FCRuntime.md` to:
- Mention `AFCBaseGameMode` and the three thin GameModes.
- Explain that per-level wiring now lives in these GameModes.

**Acceptance criteria**
- All three level types use the correct thin GameMode in PIE without manual overrides.
- Documentation reflects the new structure.

---

## 2. Implement `UFCPlayerModeCoordinator` Component

### 2.1 Add `UFCPlayerModeCoordinator` and attach to controller

2.1.1 Create `FCPlayerModeCoordinator.h/.cpp` in `Components/`, deriving from `UActorComponent`.

2.1.2 Add a `UPROPERTY(EditDefaultsOnly, Category="FC|Mode")` for a profile set asset (defined in Task 3).

2.1.3 Define a log category `LogFCPlayerModeCoordinator` and add basic startup logs.

2.1.4 In `AFCPlayerController`’s constructor, create the component (or ensure the BP default class includes it as a component).

2.1.5 Ensure the component ticks only if needed (likely not; rely on events), so disable ticking by default.

**Acceptance criteria**
- Project compiles with `UFCPlayerModeCoordinator` attached to the player controller.
- On BeginPlay, coordinator logs indicate that it initialized successfully for the controller.

---

### 2.2 Subscribe to `UFCGameStateManager::OnStateChanged`

2.2.1 In `BeginPlay` of the coordinator, retrieve `UFCGameInstance` using `GetGameInstance<UFCGameInstance>()` from the owner controller.

2.2.2 From the game instance, obtain `UFCGameStateManager` via `GetSubsystem<>()`.

2.2.3 Bind a delegate handler, e.g. `void OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState)`, on the manager’s `OnStateChanged` multicast.

2.2.4 Add `EndPlay` implementation to safely unbind from `OnStateChanged` using `RemoveAll(this)` or a stored handle.

2.2.5 Add a log in `OnGameStateChanged` showing `OldState` and `NewState` name strings via the `StaticEnum` helper.

**Acceptance criteria**
- When game state changes, coordinator logs show the OldState → NewState transition.
- No dangling delegate warnings occur when the controller is destroyed or level changes.

---

### 2.3 Define `EFCPlayerMode` and state → mode mapping

2.3.1 Introduce `EFCPlayerMode` enum (in a shared header such as `FCPlayerModeTypes.h`) with at least:
- `Office` (Office_Exploration).
- `Overworld` (Overworld_Travel).
- `Camp` (Camp_Local / POIScene).
- `Static` (ExpeditionSummary, cutscenes).

2.3.2 In the coordinator, implement a `EFCPlayerMode MapStateToMode(EFCGameStateID State) const` function with a switch covering all currently used states.

2.3.3 Decide on a default/fallback mode for unsupported states (e.g. `Office` or `Static`), and log a **Warning** if that path is taken.

2.3.4 In `OnGameStateChanged`, call `MapStateToMode(NewState)` and log the resulting `EFCPlayerMode`.

**Acceptance criteria**
- For all current gameplay states, a deterministic `EFCPlayerMode` is selected.
- Logs clearly show game state and mapped mode for debugging.

---

### 2.4 Implement coordinator logging and high-level API

2.4.1 Add a `void ApplyMode(EFCPlayerMode Mode)` method on the coordinator that is responsible for retrieving and applying the appropriate profile.

2.4.2 In `OnGameStateChanged`, delegate to `ApplyMode(MapStateToMode(NewState))`.

2.4.3 Inside `ApplyMode`, log at `Log` level:
- OldMode, NewMode.
- Profile asset name (if any) being applied.

2.4.4 Ensure `ApplyMode` only makes calls into managers/components and the controller for configuration; it must not:
- Open or close UI.
- Trigger movement.
- Execute gameplay actions.

**Acceptance criteria**
- Coordinator is the single entry point for applying mode profiles.
- Logs clearly indicate mode and profile application without side-effect behavior.

---

## 3. Design and Implement `FPlayerModeProfile` Data Structures

### 3.1 Define shared mode/profile types

3.1.1 Create or update `Core/FCPlayerCameraTypes.h` to contain `EFCPlayerCameraMode` (moved out of `FCPlayerController.h`), so other systems (like the coordinator) can reference it without circular dependencies.

3.1.2 Create or update `Core/FCPlayerModeTypes.h` to define:
- `EFCPlayerMode` (existing, reused).
- `EFCClickPolicy` as a `UENUM(BlueprintType) enum class : uint8` (e.g., `None`, `OverworldClickMove`, `CampClickMove`, `TableObjectClick`).
- `FPlayerModeProfile` as a `USTRUCT(BlueprintType)` containing:
  - `TSoftObjectPtr<UFCInputConfig> InputConfig` (optional/future override; not required for Milestone 3).
  - `EFCPlayerCameraMode CameraMode`.
  - `TSoftObjectPtr<UObject> InteractionProfile` (placeholder for a concrete type later).
  - `EFCClickPolicy ClickPolicy`.
  - `bool bShowMouseCursor`.
  - `EMouseLockMode MouseLockMode`.

3.1.3 Provide sensible editor defaults and field comments so designers can understand and edit profiles safely.

**Acceptance criteria**
- `FPlayerModeProfile` compiles and can be edited in the editor.
- Fields are grouped and named meaningfully for designers.

---

### 3.2 Implement `UFCPlayerModeProfileSet` and hook it into the coordinator

3.2.1 Define `UFCPlayerModeProfileSet : public UDataAsset` in `Components/Data/FCPlayerModeProfileSet.h` with:

```cpp
UCLASS(BlueprintType)
class FC_API UFCPlayerModeProfileSet : public UDataAsset
{
    GENERATED_BODY();

public:
    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TMap<EFCPlayerMode, FPlayerModeProfile> Profiles;
};
```

3.2.2 Add a `UPROPERTY(EditDefaultsOnly, Category="FC|Mode")` `TObjectPtr<UFCPlayerModeProfileSet>` to `UFCPlayerModeCoordinator` and expose it in the controller defaults.

3.2.3 Implement a helper on the coordinator, e.g. `bool GetProfileForMode(EFCPlayerMode Mode, FPlayerModeProfile& OutProfile) const`, that:
- Returns `false` and logs a **Warning** if `ModeProfileSet` is null or if the map does not contain `Mode`.
- Returns `true` and copies out the profile otherwise.

3.2.4 Create a default profile set asset in the editor (e.g., `/Game/FC/Config/DA_PlayerModeProfiles`) and populate entries for Office, Overworld, Camp, and Static.

**Acceptance criteria**
- Coordinator can retrieve a profile for any defined `EFCPlayerMode` from the data asset.
- Missing entries log warnings but do not crash the game.

---

### 3.3 Make `ApplyMode` profile-driven, idempotent, and fail-soft

3.3.1 Implement `ValidateProfile(const FPlayerModeProfile&, TArray<FString>& OutProblems)` (static or member) that checks:
- `CameraMode` is a known value (required).
- Optional fields (`InputConfig`, `InteractionProfile`) are logged as warnings only when missing, not treated as hard errors.

3.3.2 Refactor `UFCPlayerModeCoordinator::ApplyMode(EFCPlayerMode NewMode)` to:
- Allow reapplying the same mode (remove the early-out on `NewMode == CurrentMode`).
- Retrieve the profile via `GetProfileForMode`; warn and return early if missing.
- Call `ValidateProfile` and log any reported problems, but proceed with the valid parts.
- Derive an `EFCInputMappingMode` from `Profile.CameraMode` using a small helper (e.g., FirstPerson → FirstPerson, TopDown → TopDown, POIScene → POIScene, Table/Static → StaticScene).
- Ask the controller to apply mapping mode via `UFCInputManager` (e.g., `InputManager->SetInputMappingMode(...)`), which remains idempotent by clearing and reapplying contexts.
- Apply cursor visibility and mouse lock via a helper such as `ApplyCursorAndInputMode(const FPlayerModeProfile&)`, configuring `bShowMouseCursor` and `SetInputMode(FInputModeGameOnly/GameAndUI/UIOnly)` appropriately.
- Call `SetCameraModeLocal(Profile.CameraMode, BlendTime)` on the controller (now camera-only) so the camera moves into the correct mode without re-doing input/cursor work.
- Log old mode → new mode and the profile asset name for debugging.

3.3.3 Ensure `UFCInputManager::SetInputMappingMode` has a guard that logs an **Error** and returns if its `InputConfig` is null, but otherwise remains safe to call repeatedly (clears mappings before applying new ones).

**Acceptance criteria**
- Invalid or incomplete profiles do not crash; they emit warnings and skip only the broken pieces.
- Reapplying the same mode does not produce duplicated effects (no stacked input mappings, no repeated camera transitions, no cursor/input-mode drift).
- Controller no longer needs to manage camera/input/cursor configuration in its `Apply*Presentation` helpers; profiles + `ApplyMode` are the single source of truth.

---

## 4. Make Coordinator the Single Profile-Driven Applier

### 4.1 Finish `UFCInputManager` API and harden mapping

4.1.1 Implement `GetInputConfig`, `GetCurrentMappingMode`, and `SetInputConfig` in `FCInputManager.cpp` to back the existing declarations in `FCInputManager.h`.

4.1.2 In `SetInputConfig`, early-out if the config pointer is unchanged, assign `InputConfig`, log the asset name, and re-assert the current mapping mode by calling `SetInputMappingMode(CurrentMappingMode)` (which clears and reapplies contexts).

4.1.3 In `SetInputMappingMode`, guard against a missing enhanced input subsystem and a null `InputConfig`; if `InputConfig` is null, log a **Warning** and return without attempting to add mapping contexts.

4.1.4 Keep `SetInputMappingMode` idempotent by always clearing existing mappings before applying the new mapping context.

**Acceptance criteria**
- `UFCInputManager` exposes working `GetInputConfig`, `GetCurrentMappingMode`, and `SetInputConfig` implementations.
- Calling `SetInputMappingMode` with a null `InputConfig` logs a warning instead of crashing.
- Mapping contexts do not stack across mode changes.

---

### 4.2 Implement profile-driven `ApplyMode` in `UFCPlayerModeCoordinator`

4.2.1 Add required includes in `FCPlayerModeCoordinator.cpp` for `UFCInputManager`, `UFCInteractionComponent`, and `UFCInputConfig` so the coordinator can coordinate components directly.

4.2.2 Implement a helper that derives `EFCInputMappingMode` from `EFCPlayerMode` and `EFCPlayerCameraMode` (e.g., FirstPerson → FirstPerson, TopDown → TopDown, POIScene → POIScene, menu/table/save views → StaticScene).

4.2.3 Implement `GetProfileForMode` on the coordinator to read from `ModeProfileSet->Profiles`, returning `false` and logging a **Warning** if the set is null or the map does not contain the given mode.

4.2.4 Implement `ValidateProfile` so it ensures `CameraMode` is valid, treats missing optional fields (like `InputConfig` and `InteractionProfile`) as warnings, and returns a boolean success flag plus a human-readable problems string.

4.2.5 Refactor `ApplyMode(EFCPlayerMode NewMode)` to:
- Allow reapplying the same mode (no early-out based on `NewMode == CurrentMode`).
- Log old mode → new mode, whether this is a reapply, and the active profile set asset.
- Retrieve the `FPlayerModeProfile` via `GetProfileForMode` and return early if no profile is found.
- Run `ValidateProfile` and log any problems, but proceed with applying safe parts.
- Call `SetCameraModeLocal(Profile.CameraMode, BlendTime)` on the controller so the camera mode is set in one place.
- Optionally override `UFCInputManager`'s `InputConfig` from the profile (if set), loading the soft reference and calling `SetInputConfig` only when it changes.
- Derive and apply the desired `EFCInputMappingMode` through `UFCInputManager`, guarded by `GetCurrentMappingMode` so we avoid redundant calls.
- Apply cursor visibility and `SetInputMode` (`GameOnly`, `GameAndUI`, or `UIOnly`) from profile fields, including mouse lock behavior.
- Log final cursor, lock mode, and camera mode values for debugging.
- Gate first-person interaction by enabling/disabling the interaction component's FP focus based on camera mode instead of polling every tick.

**Acceptance criteria**
- `ApplyMode` is the single place where camera mode, input mapping mode, and cursor/input mode are applied from `FPlayerModeProfile`.
- Reapplying the same mode is safe and does not stack mappings, re-blend cameras unnecessarily, or drift cursor/input mode.
- Missing or partially configured profiles emit warnings but do not crash.

---

### 4.3 Remove per-tick camera polling from `UFCInteractionComponent`

4.3.1 In `TickComponent` of `FCInteractionComponent.cpp`, remove the call that re-derives FP focus from camera mode each frame (e.g., `UpdateFirstPersonFocusGateFromCameraMode()`).

4.3.2 Keep the existing early-outs and booleans so the component still short-circuits when interaction is disabled, but rely on the coordinator to toggle first-person focus when modes change.

4.3.3 Optionally retain `UpdateFirstPersonFocusGateFromCameraMode` as a debug helper or remove it entirely once the coordinator-driven gating proves stable.

**Acceptance criteria**
- `UFCInteractionComponent` no longer polls camera mode every Tick just to decide if FP focus is enabled.
- FP interaction gating is controlled via explicit configuration from the coordinator.

---

### 4.4 Strip mode configuration from `AFCPlayerController` presentation helpers

4.4.1 Audit `AFCPlayerController`'s `Apply*Presentation` helpers (e.g., Overworld, Camp, Office, ExpeditionSummary) and remove any code that:
- Sets `bShowMouseCursor`.
- Calls `SetInputMode(...)`.
- Calls into `UFCInputManager::SetInputMappingMode(...)`.
- Directly sets camera mode or calls `SetViewTargetWithBlend` or equivalent.

4.4.2 Keep only non-configuration "extras" in these helpers, such as binding or unbinding convoy delegates, or other behavior that is not already encoded in `FPlayerModeProfile`.

4.4.3 Simplify `ApplyPresentationForGameState` so it only manages Overworld convoy delegate binding/unbinding on state transitions and does not re-apply camera/input/cursor modes.

**Acceptance criteria**
- `AFCPlayerController` no longer applies camera, input mapping, or cursor/input-mode configuration based on game state; it only handles per-state extras.
- There is no risk of mode stacking or conflicting configuration paths between controller and coordinator.

---

## 5. Refactor Controller to Delegate Mode Logic to Coordinator

### 5.1 Audit `AFCPlayerController` for mode-specific configuration logic

5.1.1 Search `FCPlayerController.cpp` for branches on `EFCGameStateID`, camera modes, or input modes that adjust camera/input/interaction configuration.

5.1.2 Build a small table mapping each branch to its intended effect (camera, input, interaction, cursor) and link them to the new profile fields.

5.1.3 Decide which branches should remain (e.g., pause/menu/debug) and which will be superseded by profile application.

**Acceptance criteria**
- Clear list of controller responsibilities that will move to the coordinator vs. those that will remain.

---

### 5.2 Remove duplicated mode configuration from controller

5.2.1 For each branch identified as configuration-only, replace its logic with a coordinator call (e.g., ensure it has already been handled by `OnGameStateChanged` and can be removed).

5.2.2 Simplify `OnGameStateChanged` in the controller (if it exists) so it:
- Delegates to the coordinator for mode/profile application.
- Performs only controller-specific behaviors that are truly not configuration (e.g., some UI or transition events, as allowed by PRD).

5.2.3 Rebuild and run through key state transitions to ensure behavior remains consistent.

**Acceptance criteria**
- Controller no longer manually sets camera modes, mapping contexts, or interaction probe rules based on state.
- Mode/application logic is centralized in the coordinator.

---

### 5.3 Ensure controller’s remaining responsibilities are scoped

5.3.1 Confirm that input handlers (Interact, Click, Pause, QuickSave/Load) still:
- Route actions to `UFCInteractionComponent`, `UFCGameInstance`, `UFCUIManager`, etc.
- Do not re-derive or re-apply configuration based on mode.

5.3.2 If necessary, add a small comment above relevant handlers documenting that configuration is handled by the coordinator.

5.3.3 Remove any now-dead helper functions or redundant fields.

**Acceptance criteria**
- `AFCPlayerController` is clearly focused on routing input and triggering behaviors, not on configuring modes.
- No dead code remains related to old configuration paths.

---

### 5.4 Update technical documentation

5.4.1 Update `FCRuntime.md` PlayerRuntime section to include `UFCPlayerModeCoordinator` and its relationship to `UFCInputManager`, `UFCCameraManager`, and `UFCInteractionComponent`.

5.4.2 Create `FCPlayerModeCoordinator.md` under `Docs/TechnicalDocumentation/` describing:
- Responsibilities and non-responsibilities (config-only, no UI/movement).
- How it maps game state → player mode → profile.

5.4.3 Update `FCPlayerController.md` to:
- Remove descriptions of now-delegated mode configuration responsibilities.
- Point to `UFCPlayerModeCoordinator` for profile application.

5.4.4 If needed, adjust `FCInputManager.md` and `FCCameraManager.md` docs to mention that they are driven by the coordinator.

**Acceptance criteria**
- Docs accurately reflect that controller no longer owns mode configuration.
- Coordinator’s role is clearly documented for future refactors.

---

## 6. Testing, Validation, and Logging

### 6.1 Define and run a level-mode test matrix

6.1.1 Define a simple test matrix covering:
- Office_Exploration → expected `EFCPlayerMode::Office` profile.
- Overworld_Travel → expected `EFCPlayerMode::Overworld` profile.
- Camp_Local → expected `EFCPlayerMode::Camp` profile.
- ExpeditionSummary → expected `EFCPlayerMode::Static` profile.

6.1.2 For each level type, run PIE and verify:
- Correct pawn and controller are spawned.
- Camera behaves as expected for the mode.
- Input mappings (actions/contexts) follow the mode’s `UFCInputConfig`.
- Cursor visibility and input mode (GameOnly/GameAndUI/UIOnly) match the profile.

6.1.3 Verify transitions between states (e.g., Overworld → Office → Camp) do not require manual reapplication of settings; coordinator handles them automatically.

**Acceptance criteria**
- All tested states and transitions apply the right profiles without manual intervention.

---

### 6.2 Add debugging aids

6.2.1 Ensure coordinator logs for `OnGameStateChanged` and `ApplyMode` include:
- OldState/NewState.
- OldMode/NewMode.
- Profile asset name.

6.2.2 Optionally add a console command (e.g., `fc.DumpPlayerMode`) that prints the current state, mode, and profile.

6.2.3 Optionally add a lightweight debug widget (in dev builds) to show current mode + camera mode + mapping mode.

**Acceptance criteria**
- Developers can quickly see which mode/profile is active while debugging.

---

### 6.3 Regression checks and safety

6.3.1 Re-run existing interaction flows:
- Office table interactions.
- Overworld convoy movement and POI actions.
- Camp explorer movement and Camp POI handling.

6.3.2 Confirm that profile application is safe across travel boundaries (e.g., loading from save, changing maps) and is idempotent.

6.3.3 Verify that there are no circular dependencies:
- UIManager does not call the coordinator.
- Coordinator does not call UIManager.

**Acceptance criteria**
- All key gameplay loops behave as before or better.
- No new crashes or circular dependencies are introduced.
