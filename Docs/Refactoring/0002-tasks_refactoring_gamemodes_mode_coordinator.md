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

### 3.1 Define `FPlayerModeProfile` struct

3.1.1 Create `FCPlayerModeTypes.h` (or similar) and define:

```cpp
USTRUCT(BlueprintType)
struct FPlayerModeProfile
{
    GENERATED_BODY();

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TSoftObjectPtr<UFCInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    EFCPlayerCameraMode CameraMode;

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TSoftObjectPtr<UObject> InteractionProfile; // Placeholder for a concrete type later

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TEnumAsByte<EFCClickPolicy> ClickPolicy; // New small enum

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    bool bShowMouseCursor = true;

    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TEnumAsByte<EMouseLockMode> MouseLockMode = EMouseLockMode::DoNotLock;
};
```

3.1.2 Define a small `EFCClickPolicy` enum (e.g., `None`, `OverworldClickMove`, `CampClickMove`, `TableObjectClick`) to express high-level click intent.

3.1.3 Add sensible defaults that match current behavior for Office, Overworld, Camp, and Static modes.

3.1.4 Document each field’s intent using comments, referencing the PRD where relevant.

**Acceptance criteria**
- `FPlayerModeProfile` compiles and can be edited in the editor.
- Fields are grouped and named meaningfully for designers.

---

### 3.2 Create a profile set container asset

3.2.1 Define a `UDataAsset` subclass (e.g., `UFCPlayerModeProfileSet`) with:

```cpp
UCLASS(BlueprintType)
class UFCPlayerModeProfileSet : public UDataAsset
{
    GENERATED_BODY();

public:
    UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TMap<EFCPlayerMode, FPlayerModeProfile> Profiles;
};
```

3.2.2 Add a `UPROPERTY(EditDefaultsOnly, Category="FC|Mode")` `TObjectPtr<UFCPlayerModeProfileSet>` to `UFCPlayerModeCoordinator`.

3.2.3 Implement a helper in the coordinator, e.g. `bool GetProfileForMode(EFCPlayerMode Mode, FPlayerModeProfile& OutProfile) const`.

3.2.4 Create a default profile set asset in the editor (e.g., `/Game/FC/Config/DA_PlayerModeProfiles`) and populate entries for Office, Overworld, Camp, and Static.

**Acceptance criteria**
- Coordinator can retrieve a profile for any defined `EFCPlayerMode` from the data asset.
- Missing entries log warnings but do not crash the game.

---

### 3.3 Implement validation and idempotent apply helpers

3.3.1 Implement a `ValidateProfile(const FPlayerModeProfile&)` method (static or member) that checks:
- `InputConfig` is set.
- Camera mode is a known value.
- Other required fields per mode (if any) are present.

3.3.2 In `ApplyMode`, call `ValidateProfile` and log warnings for incomplete profiles, skipping only the invalid parts.

3.3.3 Ensure that calling `ApplyMode` twice with the same mode/profile does not stack duplicate mappings, cursors, or camera changes (idempotent behavior):
- E.g., `UFCInputManager` should clear + apply contexts every time; `UFCCameraManager` should be prepared for repeated calls.

3.3.4 For now, use synchronous `LoadSynchronous()` on soft references as needed, and document that future milestones may switch to async.

**Acceptance criteria**
- Invalid or incomplete profiles do not crash; they emit warnings and skip broken sections.
- Reapplying the same mode does not produce duplicated effects (e.g., multiple contexts, broken camera state).

---

## 4. Wire Coordinator to Input/Camera/Interaction/Controller

### 4.1 Integrate coordinator with `UFCInputManager`

4.1.1 Confirm current `UFCInputManager` API for setting mapping mode and owning `UFCInputConfig`.

4.1.2 Add or reuse an API (e.g., `SetInputConfig(const UFCInputConfig* Config)`) if necessary, or ensure the manager already holds a pointer to a config.

4.1.3 In `UFCPlayerModeCoordinator::ApplyMode`, after resolving `FPlayerModeProfile` and its `InputConfig`, pass that config to `UFCInputManager` and call `SetInputMappingMode` with the appropriate `EFCInputMappingMode` for this `EFCPlayerMode`.

4.1.4 Log input config application with the asset name (or path) for debugging.

**Acceptance criteria**
- Mode changes trigger the correct `UFCInputConfig` and mapping context via `UFCInputManager`.
- No direct config manipulation occurs in `AFCPlayerController` for mode-based mapping.

---

### 4.2 Integrate coordinator with `UFCCameraManager`

4.2.1 Confirm existing `UFCCameraManager` APIs for setting camera mode (e.g., `SetFallenCompassCameraMode` or equivalents).

4.2.2 Map `FPlayerModeProfile.CameraMode` to the appropriate `UFCCameraManager` call(s).

4.2.3 In `ApplyMode`, delegate camera operations **only** to `UFCCameraManager`, not directly using `SetViewTargetWithBlend`.

4.2.4 Add logs only for high-level camera mode changes, not for every internal blend.

**Acceptance criteria**
- Changing `EFCPlayerMode` results in the expected camera mode (FirstPerson/TopDown/POIScene/Static) via `UFCCameraManager`.
- The coordinator never calls `SetViewTargetWithBlend` directly.

---

### 4.3 Integrate coordinator with `UFCInteractionComponent`

4.3.1 Decide which interaction behaviors should be controlled by profile (e.g., enabling FP focus, interaction trace distance, which interaction profile asset to use).

4.3.2 Extend `UFCInteractionComponent` with a lightweight configuration API such as `ApplyInteractionProfile(UObject* ProfileObject)` or a struct-based setter.

4.3.3 In `ApplyMode`, after resolving the profile, call into `UFCInteractionComponent` to apply the profile or its parameters.

4.3.4 Ensure `UFCInteractionComponent` continues to own behavior (what happens when interacting), while the coordinator only chooses configuration (when/where/how probing happens).

**Acceptance criteria**
- Interaction probing/gating (e.g., FP focus, trace) adjusts correctly when switching modes.
- No interaction behavior (like actually performing actions) is moved into the coordinator.

---

### 4.4 Apply cursor and input mode rules on `AFCPlayerController`

4.4.1 In `ApplyMode`, use the `FPlayerModeProfile` cursor and mouse lock fields to configure:
- `bShowMouseCursor`.
- `SetInputMode(FInputModeGameOnly/GameAndUI/UIOnly)` as appropriate.

4.4.2 Carefully preserve existing rules around UI blocking (e.g., blocking widgets still override click handling), just moving the default cursor/input configuration into profiles.

4.4.3 Remove duplicated cursor/input-mode setup from scattered controller state-handling code, replacing it with calls triggered by coordinator mode application.

4.4.4 Add minimal logs for profile-driven cursor/input-mode changes (one per mode switch).

**Acceptance criteria**
- Cursor and input mode are now determined by the active `FPlayerModeProfile`.
- Controller does not branch on game state to set these values directly; it relies on the coordinator.

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
