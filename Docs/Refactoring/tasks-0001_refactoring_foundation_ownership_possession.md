# Task List — tasks-0001_refactoring_foundation_ownership_possession.md
> Generated from `0001_refactoring_foundation_ownership_possession.md` using `/Docs/task-template-schema.md` structure (Phase 2: expanded steps).

---

## Task Metadata

```yaml
Task ID: 0001
Sprint/Phase: Week 1 (Foundation)
Feature Name: Foundation Refactor — Interaction Ownership + Camp Possession
Dependencies: []
Estimated Complexity: Medium-High
Primary Files Affected:
  - /Source/FC/Core/FCPlayerController.h
  - /Source/FC/Core/FCPlayerController.cpp
  - /Source/FC/Components/FCInteractionComponent.h
  - /Source/FC/Components/FCInteractionComponent.cpp
  - /Source/FC/Characters/FC_ExplorerCharacter.h
  - /Source/FC/Characters/FC_ExplorerCharacter.cpp
  - /Source/FC/Overworld/FCOverworldConvoy.h
  - /Source/FC/Overworld/FCOverworldConvoy.cpp
  - /Source/FC/Components/FCInputConfig.h (new)
  - /Source/FC/Components/FCInputConfig.cpp (new)
  - /Content/FC/Input/Data/DA_InputConfig_PlayerRuntime.uasset (new)
  - /Content/FC/Levels/Camp/* (level config changes)
Key Decisions:
  - Interaction remains controller-owned. World actors only broadcast delegates.
  - Camp uses standard possession (PlayerController possesses Explorer) while camera remains static via SetViewTargetWithBlend.
  - Input assets are referenced via UFCInputConfig DataAsset (no hardcoded /Game/* paths).
  - FirstPerson focus trace/prompt updates are mode-gated (only in FirstPerson).
```

### Reference Documents
- `/Docs/UE_CodeConventions.md` — Encapsulation, event-driven design, pointer safety, IWYU
- `/Docs/UE_NamingConventions.md` — Asset prefixes/folder structure (DataAssets, Input)
- `/Docs/Fallen_Compass_GDD_v0.2.md` — POI/Overworld/Camp gameplay loop context

---

## Pre-Implementation Phase

### Step 1.0: Analysis & Discovery

- [x] **Document what already exists vs. what to create**
  - [x] Create this note in the task file:
    ```
    === ANALYSIS FINDINGS (Step 1.0 Complete) ===
    
    CRITICAL OWNERSHIP ISSUES FOUND:
    
    1. GetInstigatorController() misuse (3 occurrences in FCInteractionComponent.cpp):
       - Line 33: Widget creation path
       - Line 82: DetectInteractables()
       - Line 200: UpdatePromptWidget()
       - ROOT CAUSE: Component owner IS the controller, so GetInstigatorController() returns null
       - IMPACT: Prompt widgets never create, trace never works properly
    
    2. GetFirstPlayerController() overuse (11 occurrences total):
       - FCInteractionComponent.cpp: 4 occurrences (lines 276, 351, 404, 434)
       - FCUIManager.cpp: 4 occurrences (lines 135, 192, 222, 297)
       - FCLevelTransitionManager.cpp: 2 occurrences (lines 236, 328)
       - FCOverworldConvoy.cpp: 1 occurrence (line 178) - CRITICAL PATH
       - IMPACT: Tight coupling, breaks in multiplayer/PIE, hard to reason about
    
    3. Convoy pawn-crawling pattern (FCOverworldConvoy.cpp lines 178-193):
       - Gets PC via GetFirstPlayerController()
       - Casts PC->GetPawn() to AFCFirstPersonCharacter (WRONG in Overworld!)
       - Fetches InteractionComponent from FP pawn (line 184)
       - IMPACT: Fails in Overworld mode because pawn isn't FirstPersonCharacter
       - FALLBACK: Logs warning but interaction never fires
    
    4. Explorer AI-possession pattern (FC_ExplorerCharacter.cpp):
       - AutoPossessAI = PlacedInWorldOrSpawned (line 17)
       - AIControllerClass = AAIController (line 18)
       - NOTE: Comment says "allows SimpleMoveToLocation while maintaining static camp camera"
       - CURRENT: Works but forces "commanded actor" glue everywhere
       - TARGET: Standard possession + SetViewTargetWithBlend for static camera
    
    5. Hardcoded input asset paths (12 occurrences):
       - FCPlayerController.cpp: 8 paths (IA_Interact, IA_QuickSave, IA_QuickLoad, IA_Escape, 
         IA_Click, IA_OverworldPan, IA_OverworldZoom, IA_ToggleOverworldMap)
       - FCFirstPersonCharacter.cpp: 4 paths (IA_Move, IA_Look, IA_Interact, mesh asset)
       - PATTERN: ConstructorHelpers::FObjectFinder(TEXT("/Game/FC/Input/IA_*"))
       - IMPACT: Brittle, breaks on asset rename, can't override per-mode
    
    6. Misleading variable names:
       - PossessedConvoy (FCPlayerController.h line 186) - convoy is NOT possessed
       - CommandedExplorer (FCPlayerController.h line 189) - explorer IS AI-commanded
       - IMPACT: Code readers assume wrong ownership model
    
    EXISTING STRUCTURE (Good foundation):
    - ✅ AFCPlayerController owns InteractionComponent as subobject (correct)
    - ✅ UFCInputManager component exists (can be extended for config)
    - ✅ UFCCameraManager component exists (ready for mode switching)
    - ✅ Convoy overlap handler has space for delegate (current impl broken)
    - ✅ Explorer configured with movement component + anim BP
    
    TO CREATE (New files):
    - UFCInputConfig DataAsset class (C++)
      * Path: /Source/FC/Input/FCInputConfig.h/.cpp
      * Fields: UInputMappingContext*, UInputAction* refs for core actions
    - DA_InputConfig_PlayerRuntime asset (Content)
      * Path: /Content/FC/Input/Data/DA_InputConfig_PlayerRuntime.uasset
      * Assign all IA_* and IMC_* assets used by PlayerController
    
    RISK ASSESSMENT:
    - HIGH: Convoy overlap path completely broken in current form (pawn-crawling)
    - HIGH: InteractionComponent prompt/trace never worked correctly (null controller)
    - MEDIUM: Explorer possession change may affect existing camp scenes
    - LOW: Input config migration (can use hard refs first, soft refs later)
    
    RECOMMENDED ORDER (matches PRD):
    1. Camp possession (highest isolation, enables animation fix)
    2. Cache controller in InteractionComponent (fixes 3 critical nulls)
    3. Convoy delegate binding (unblocks Overworld POI interaction)
    4. Mode-gate FP trace (prevents spurious updates in TopDown)
    5. Input config (removes hardcoded paths, enables Week 2 profiles)
    6. Rename variables (code clarity, prevents future confusion)
    ```

**COMMIT POINT 1.0** (docs-only): `git commit -m "chore(refactor): add baseline notes for PRD 0001"`

---

## Implementation Phase

### Step 1.1: Camp — Possess Explorer + Keep Static Camera

#### Step 1.1.1: Make Explorer the possessed pawn in Camp
**Analysis**
- Old behavior:
  - `AFC_ExplorerCharacter` was AI-possessed (`AutoPossessAI`, `AIControllerClass`), and the controller used a “commanded actor” pattern to move it while keeping a static Camp camera.
  - `AFCPlayerController` contained some camera-discovery logic in `BeginPlay`, and earlier designs assumed calling `SetViewTargetWithBlend` directly in the controller for Camp.
- New behavior (post-refactor):
  - `AFC_ExplorerCharacter` is now a standard **player-possessed** pawn in Camp (template-style TopDown character).
  - `AFCPlayerController` no longer calls `SetViewTargetWithBlend` directly for Camp/POI scenes.
  - Camp camera behavior is owned by `UFCCameraManager`:
    - When the game enters `Camp_Local` state, `AFCPlayerController::OnGameStateChanged` calls `CameraManager->BlendToPOISceneCamera(POISceneCameraActor, BlendTime)`.
    - If `POISceneCameraActor` is `nullptr`, `UFCCameraManager::BlendToPOISceneCamera` auto-resolves a Camp camera by finding an `ACameraActor` tagged `"CampCamera"` (or with a name containing `CampCamera`) and internally calls `SetViewTargetWithBlend` on the controller.
  - This keeps:
    - **Possession** logic in GameMode/Explorer,
    - **Routing** in PlayerController (react to state),
    - **Camera discovery + blending** in `UFCCameraManager`.

**Implementation**
- [x] In Camp GameMode, set `DefaultPawnClass = AFC_ExplorerCharacter` so it is spawned/possessed normally.
- [x] Update `AFC_ExplorerCharacter` defaults:
  - [x] Ensure it does not *require* AIController for movement (now uses `AutoPossessPlayer = Player0`, `AIControllerClass = nullptr`).
  - [x] Tick is enabled only for debugging during Week 1 (with TODO to disable once movement/animation are validated).
- [x] Update `AFCPlayerController` Camp init path:
  - [x] Remove direct Camp camera discovery from `BeginPlay` (no manual `UGameplayStatics::GetAllActorsOfClass` for `ACameraActor` there).
  - [x] Rely on `OnGameStateChanged(Camp_Local)` → `CameraManager->BlendToPOISceneCamera(POISceneCameraActor, BlendTime)` to select the Camp camera.
  - [x] Let `UFCCameraManager::BlendToPOISceneCamera`:
    - resolve the `CampCamera` actor (by tag/name) when `POISceneCameraActor` is null,
    - call `SetViewTargetWithBlend` on the owning controller exactly once when entering Camp/POI scenes.

**Notes**
- The original plan (“call `SetViewTargetWithBlend(CampCameraActor, BlendTime)` once on Camp start from the PlayerController”) was refined:
  - We **still use `SetViewTargetWithBlend`**, but only **inside `UFCCameraManager`**, not inside `AFCPlayerController`.
  - This matches the architecture principle: *controller routes state → camera manager; camera manager owns view-target logic*.

**Testing After Step 1.1.1** ✅ CHECKPOINT
- [x] Compile (User): ✅
- [x] PIE Camp:
  - [x] PlayerController `GetPawn()` returns `AFC_ExplorerCharacter`.
  - [x] A `CameraActor` with tag `"CampCamera"` exists in the level.
  - [x] Entering `Camp_Local` causes `UFCCameraManager::BlendToPOISceneCamera` to log that it found/used the Camp camera.
  - [x] Camera remains static at the Camp camera while Explorer moves.
- [x] Output Log:
  - [x] No “Accessed None” errors.
  - [x] One-time, high-signal logs from camera manager/controller on Camp entry (no spam).

**COMMIT POINT 1.1.1**: `git commit -m "feat(camp): possess explorer and keep static view target"`

---

#### Step 1.1.2: Confirm movement path works with possession
**Analysis**
- With possession restored, ensure movement input uses the possessed controller path (TopDown template style).
- If you used `SimpleMoveToLocation`, it should now target the PlayerController (not AI).

**Implementation**
- [x] Confirm the Camp click-to-move handler issues movement through the PlayerController path.
- [x] Remove/disable the “commanded actor” glue in Camp if present (e.g., controller commanding an unpossessed pawn AI controller).
- [x] Add a one-time log on Camp start:
  - `UE_LOG(LogFCPlayerController, Log, TEXT("Camp init: Pawn=%s ViewTarget=%s"), ...)`

**Testing After Step 1.1.2** ✅ CHECKPOINT
- [x] PIE Camp: Explorer moves reliably on click
- [x] Output Log: includes one Camp init line; no spam

**COMMIT POINT 1.1.2**: `git commit -m "refactor(camp): remove commanded-actor assumption for explorer movement"`

---

### Step 1.2: InteractionComponent — Cache Controller + Remove Instigator-Based Resolution

#### Step 1.2.1: Cache AFCPlayerController* in InteractionComponent
**Analysis**
- Current InteractionComponent creates widgets using `GetOwner()->GetInstigatorController()`, which is unreliable when Owner is already the controller.
- Fix: cache `AFCPlayerController*` from `GetOwner()`.

**Implementation**
- [x] In `/Source/FC/Components/FCInteractionComponent.h`:
  - [x] Add private member:
    - `TWeakObjectPtr<class AFCPlayerController> OwnerPC;`
  - [x] Add private helper:
    - `AFCPlayerController* GetOwnerPCCheckedOrNull() const;`
- [x] In `BeginPlay()` or `OnRegister()`:
  - [x] `OwnerPC = Cast<AFCPlayerController>(GetOwner());`
  - [x] If null, log high-signal error once (not in Tick).
- [x] Replace all `GetOwner()->GetInstigatorController()` usage with `OwnerPC.Get()`.

**Testing After Step 1.2.1** ✅ CHECKPOINT
- [x] Compile (User): ✅
- [x] PIE Office + Camp + Overworld:
  - [x] No “controller is null” errors
  - [x] Prompt widget creation does not crash if class missing
- [x] Output Log: no Accessed None

**COMMIT POINT 1.2.1**: `git commit -m "fix(interaction): cache owner player controller and remove instigator lookup"`

---

#### Step 1.2.2: Mode-gate FirstPerson trace/prompt updates
**Analysis**
- Focus trace/prompt should only run in FirstPerson mode.

**Implementation**
- [x] Add a mode check at the start of InteractionComponent tick/update function:
  - If not FirstPerson:
    - [x] Clear focus target/prompt state if needed.
    - [x] Return early (no trace, no prompt updates).
- [x] Ensure this logic is event-driven where possible:
  - Prefer reacting to mode change events over ticking constantly (prototype can keep minimal tick if needed).

**Testing After Step 1.2.2** ✅ CHECKPOINT
- [x] PIE Overworld/Camp: no interaction prompt updates or FP traces run
- [x] PIE Office: FP trace works as before

**COMMIT POINT 1.2.2**: `git commit -m "refactor(interaction): gate focus tracing and prompts to first-person mode"`

---

### Step 1.3: Normalize Overlap → Interaction Routing (Delegates, No Ownership Crawling)

#### Step 1.3.1: Convoy broadcasts overlap; does not fetch components from pawns
**Analysis**
- Current convoy overlap code crawls: World → PC → Pawn → InteractionComponent. This breaks when Pawn isn’t the FP pawn.

**Implementation**
- [x] In `AFCOverworldConvoy`:
  - [x] Add multicast delegate (if missing):
    - `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPOIOverlap, AActor*, POIActor);`
    - `UPROPERTY(BlueprintAssignable)` `FOnPOIOverlap OnPOIOverlap;`
  - [x] In overlap handler:
    - [x] Validate POI actor
    - [x] Broadcast `OnPOIOverlap.Broadcast(POIActor);`
  - [x] Remove any `GetFirstPlayerController()` calls and any casting to FP pawn for interaction delegation.
- [x] Ensure this does not spam (one broadcast per overlap entry, not per tick).

**Testing After Step 1.3.1** ✅ CHECKPOINT
- [x] Compile (User): ✅
- [x] PIE Overworld: entering POI overlap triggers exactly one broadcast (log it once)
- [x] No reliance on FP pawn

**COMMIT POINT 1.3.1**: `git commit -m "refactor(overworld): convoy broadcasts POI overlap via delegate (no pawn crawling)"`

---

#### Step 1.3.2: PlayerController binds convoy/explorer overlap delegates to controller-owned InteractionComponent
**Analysis**
- Controller is the orchestration owner; it binds world event sources (convoy/explorer) to interaction.

**Implementation**
- [x] In `AFCPlayerController` initialization for Overworld/Camp:
  - [x] Resolve ActiveConvoy reference (temporary: by tag or placed actor lookup; prefer explicit ref if available)
  - [x] Bind:
    - `ActiveConvoy->OnPOIOverlap.AddDynamic(InteractionComponent, &UFCInteractionComponent::NotifyPOIOverlap);`
  - [x] If Camp POIs use overlap for “arrival”, bind explorer-related arrival/overlap similarly.
- [x] Rename:
  - [x] `PossessedConvoy` → `ActiveConvoy` in controller and any related code.

**Testing After Step 1.3.2** ✅ CHECKPOINT
- [x] PIE Overworld: POI overlap reaches InteractionComponent handler
- [x] Verify via log in InteractionComponent: “NotifyPOIOverlap received from convoy …”
- [x] No compile errors after rename

**COMMIT POINT 1.3.2**: `git commit -m "refactor(controller): bind overlap delegates to controller-owned interaction; rename to ActiveConvoy"`

---

### Step 1.4: Input — UFCInputConfig DataAsset + Remove Hardcoded IA Paths

#### Step 1.4.1: Create UFCInputConfig DataAsset class
**Analysis**
- Hardcoded `/Game/...` paths in C++ are brittle. Use a DataAsset for input references.

**Implementation**
- [x] Create C++ class `UFCInputConfig : public UDataAsset`
  - Path: `/Source/FC/Components/FCInputConfig.h/.cpp` (or `/Source/FC/Input/` if that module exists)
  - Fields (UPROPERTY EditDefaultsOnly):
    - `UInputMappingContext*` for each gameplay mode you need now (can start with one)
    - `UInputAction*` for core actions referenced in C++ (Interact, Click, etc.)
  - Keep the MVP minimal: only what is referenced by code today.
- [x] Add a dedicated log category if the project uses them for input (optional).

**Testing After Step 1.4.1** ✅ CHECKPOINT
- [x] Compile (User): ✅

**COMMIT POINT 1.4.1**: `git commit -m "feat(input): add UFCInputConfig data asset class"`

---

#### Step 1.4.2: Create DA_InputConfig_PlayerRuntime asset + wire it up
**Analysis**
- DataAsset must exist in Content and be assigned to the runtime system.

**Implementation**
- [x] In Content Browser:
  - [x] Create `DA_InputConfig_PlayerRuntime` under `/Content/FC/Input/Data/`
  - [x] Assign Input Actions (`IA_*`) and Mapping Contexts (`IMC_*`) used by PlayerController/InputManager
- [x] In `UFCInputManager` or PlayerController:
  - [x] Add property `UPROPERTY(EditDefaultsOnly)` `TObjectPtr<UFCInputConfig> InputConfig;`
  - [x] Replace any `ConstructorHelpers::FObjectFinder("/Game/FC/Input/IA_*")` with `InputConfig` access.
  - [x] Validate config on BeginPlay; if missing, log one error and disable input setup safely.

**Testing After Step 1.4.2** ✅ CHECKPOINT
- [x] PIE Office: input still works
- [x] PIE Overworld/Camp: click/move still works
- [x] Output Log: no missing asset crashes

**COMMIT POINT 1.4.2**: `git commit -m "refactor(input): remove hardcoded IA asset paths and use UFCInputConfig"`

---

### Step 1.5: Logging & Failure Visibility (Map + Mode Context, No Spam)

#### Step 1.5.1: Add high-signal setup logs + safe aborts
**Analysis**
- Misconfigured scenes should fail loudly once, not silently or by crashing.

**Implementation**
- [ ] Add one-time setup validations (not in Tick) for:
  - Camp camera missing
  - Explorer not possessed in Camp
  - ActiveConvoy missing in Overworld
  - Delegate binding failure (null InteractionComponent)
- [ ] Include map name in logs:
  - `GetWorld()->GetMapName()`
- [ ] Include current high-level mode/state if accessible (prototype: string is ok).
- [ ] Ensure errors do not spam every frame; log once per missing dependency.

**Testing After Step 1.5.1** ✅ CHECKPOINT
- [ ] PIE all scenes: no new unexpected warnings/errors
- [ ] Intentionally misconfigure one dependency (e.g., remove camp camera tag):
  - [ ] See one clear error with map name
  - [ ] No crash

**COMMIT POINT 1.5.1**: `git commit -m "chore(logging): add high-signal setup validation logs for camp/overworld dependencies"`

---

## Post-Implementation Phase

### Step 1.Z: Final Verification Checklist (Required)
- [ ] Compile (User): ✅
- [ ] PIE Office:
  - [ ] FirstPerson prompt/trace works
  - [ ] No unexpected logs
- [ ] PIE Overworld:
  - [ ] Convoy POI overlap reaches controller-owned interaction via delegate
  - [ ] No pawn crawling paths remain
- [ ] PIE Camp:
  - [ ] Explorer possessed by PlayerController
  - [ ] Camera remains static (view target = CampCamera)
  - [ ] Movement works
- [ ] Output Log:
  - [ ] No “Accessed None”
  - [ ] No repeated spam errors
- [ ] Quick code review:
  - [ ] No `GetInstigatorController()` in controller-owned code paths
  - [ ] No hardcoded `/Game/FC/Input/IA_*` asset loads in runtime layer
  - [ ] All UObject pointers introduced are `UPROPERTY()`

**COMMIT POINT 1.Z**: `git commit -m "chore(refactor): complete PRD 0001 foundation checklist"`

---

## Known Issues / Follow-ups (Record as discovered)
- [ ] (Add items here as you test; keep it honest and specific.)

