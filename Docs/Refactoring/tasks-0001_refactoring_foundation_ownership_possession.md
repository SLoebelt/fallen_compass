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
- Current FC_ExplorerCharacter uses AI possession to keep static camera.
- New approach: PlayerController possesses Explorer; camera stays static by setting view target to CampCamera actor.

**Implementation**
- [ ] Decide *one* approach (choose the lowest-risk for your project setup):
  - **Option A (preferred for prototype):** In Camp level, set placed Explorer actor `Auto Possess Player = Player 0`, and disable AutoPossessAI for that placed instance.
  - **Option B:** In Camp GameMode (if already exists), set `DefaultPawnClass = AFC_ExplorerCharacter` so it is spawned/possessed normally.
- [ ] Update `AFC_ExplorerCharacter` defaults if needed:
  - [ ] Ensure it does not *require* AIController for movement.
  - [ ] Avoid enabling Tick by default (unless required temporarily for debugging).
- [ ] Update `AFCPlayerController` Camp init path:
  - [ ] Find/resolve CampCamera actor (prefer explicit reference; tag fallback OK).
  - [ ] Call `SetViewTargetWithBlend(CampCameraActor, BlendTime)` once on Camp start.

**Testing After Step 1.1.1** ✅ CHECKPOINT
- [ ] Compile (User): ✅
- [ ] PIE Camp:
  - [ ] PlayerController `GetPawn()` returns Explorer
  - [ ] View target is CampCamera actor (static)
  - [ ] Click/move input causes Explorer to move
- [ ] Output Log: no errors, no Accessed None

**COMMIT POINT 1.1.1**: `git commit -m "feat(camp): possess explorer and keep static view target"`

---

#### Step 1.1.2: Confirm movement path works with possession
**Analysis**
- With possession restored, ensure movement input uses the possessed controller path (TopDown template style).
- If you used `SimpleMoveToLocation`, it should now target the PlayerController (not AI).

**Implementation**
- [ ] Confirm the Camp click-to-move handler issues movement through the PlayerController path.
- [ ] Remove/disable the “commanded actor” glue in Camp if present (e.g., controller commanding an unpossessed pawn AI controller).
- [ ] Add a one-time log on Camp start:
  - `UE_LOG(LogFCPlayerController, Log, TEXT("Camp init: Pawn=%s ViewTarget=%s"), ...)`

**Testing After Step 1.1.2** ✅ CHECKPOINT
- [ ] PIE Camp: Explorer moves reliably on click
- [ ] Output Log: includes one Camp init line; no spam

**COMMIT POINT 1.1.2**: `git commit -m "refactor(camp): remove commanded-actor assumption for explorer movement"`

---

### Step 1.2: InteractionComponent — Cache Controller + Remove Instigator-Based Resolution

#### Step 1.2.1: Cache AFCPlayerController* in InteractionComponent
**Analysis**
- Current InteractionComponent creates widgets using `GetOwner()->GetInstigatorController()`, which is unreliable when Owner is already the controller.
- Fix: cache `AFCPlayerController*` from `GetOwner()`.

**Implementation**
- [ ] In `/Source/FC/Components/FCInteractionComponent.h`:
  - [ ] Add private member:
    - `TWeakObjectPtr<class AFCPlayerController> OwnerPC;`
  - [ ] Add private helper:
    - `AFCPlayerController* GetOwnerPCCheckedOrNull() const;`
- [ ] In `BeginPlay()` or `OnRegister()`:
  - [ ] `OwnerPC = Cast<AFCPlayerController>(GetOwner());`
  - [ ] If null, log high-signal error once (not in Tick).
- [ ] Replace all `GetOwner()->GetInstigatorController()` usage with `OwnerPC.Get()`.

**Testing After Step 1.2.1** ✅ CHECKPOINT
- [ ] Compile (User): ✅
- [ ] PIE Office + Camp + Overworld:
  - [ ] No “controller is null” errors
  - [ ] Prompt widget creation does not crash if class missing
- [ ] Output Log: no Accessed None

**COMMIT POINT 1.2.1**: `git commit -m "fix(interaction): cache owner player controller and remove instigator lookup"`

---

#### Step 1.2.2: Mode-gate FirstPerson trace/prompt updates
**Analysis**
- Focus trace/prompt should only run in FirstPerson mode.

**Implementation**
- [ ] Add a mode check at the start of InteractionComponent tick/update function:
  - If not FirstPerson:
    - [ ] Clear focus target/prompt state if needed.
    - [ ] Return early (no trace, no prompt updates).
- [ ] Ensure this logic is event-driven where possible:
  - Prefer reacting to mode change events over ticking constantly (prototype can keep minimal tick if needed).

**Testing After Step 1.2.2** ✅ CHECKPOINT
- [ ] PIE Overworld/Camp: no interaction prompt updates or FP traces run
- [ ] PIE Office: FP trace works as before

**COMMIT POINT 1.2.2**: `git commit -m "refactor(interaction): gate focus tracing and prompts to first-person mode"`

---

### Step 1.3: Normalize Overlap → Interaction Routing (Delegates, No Ownership Crawling)

#### Step 1.3.1: Convoy broadcasts overlap; does not fetch components from pawns
**Analysis**
- Current convoy overlap code crawls: World → PC → Pawn → InteractionComponent. This breaks when Pawn isn’t the FP pawn.

**Implementation**
- [ ] In `AFCOverworldConvoy`:
  - [ ] Add multicast delegate (if missing):
    - `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPOIOverlap, AActor*, POIActor);`
    - `UPROPERTY(BlueprintAssignable)` `FOnPOIOverlap OnPOIOverlap;`
  - [ ] In overlap handler:
    - [ ] Validate POI actor
    - [ ] Broadcast `OnPOIOverlap.Broadcast(POIActor);`
  - [ ] Remove any `GetFirstPlayerController()` calls and any casting to FP pawn for interaction delegation.
- [ ] Ensure this does not spam (one broadcast per overlap entry, not per tick).

**Testing After Step 1.3.1** ✅ CHECKPOINT
- [ ] Compile (User): ✅
- [ ] PIE Overworld: entering POI overlap triggers exactly one broadcast (log it once)
- [ ] No reliance on FP pawn

**COMMIT POINT 1.3.1**: `git commit -m "refactor(overworld): convoy broadcasts POI overlap via delegate (no pawn crawling)"`

---

#### Step 1.3.2: PlayerController binds convoy/explorer overlap delegates to controller-owned InteractionComponent
**Analysis**
- Controller is the orchestration owner; it binds world event sources (convoy/explorer) to interaction.

**Implementation**
- [ ] In `AFCPlayerController` initialization for Overworld/Camp:
  - [ ] Resolve ActiveConvoy reference (temporary: by tag or placed actor lookup; prefer explicit ref if available)
  - [ ] Bind:
    - `ActiveConvoy->OnPOIOverlap.AddDynamic(InteractionComponent, &UFCInteractionComponent::NotifyPOIOverlap);`
  - [ ] If Camp POIs use overlap for “arrival”, bind explorer-related arrival/overlap similarly.
- [ ] Rename:
  - [ ] `PossessedConvoy` → `ActiveConvoy` in controller and any related code.

**Testing After Step 1.3.2** ✅ CHECKPOINT
- [ ] PIE Overworld: POI overlap reaches InteractionComponent handler
- [ ] Verify via log in InteractionComponent: “NotifyPOIOverlap received from convoy …”
- [ ] No compile errors after rename

**COMMIT POINT 1.3.2**: `git commit -m "refactor(controller): bind overlap delegates to controller-owned interaction; rename to ActiveConvoy"`

---

### Step 1.4: Input — UFCInputConfig DataAsset + Remove Hardcoded IA Paths

#### Step 1.4.1: Create UFCInputConfig DataAsset class
**Analysis**
- Hardcoded `/Game/...` paths in C++ are brittle. Use a DataAsset for input references.

**Implementation**
- [ ] Create C++ class `UFCInputConfig : public UDataAsset`
  - Path: `/Source/FC/Components/FCInputConfig.h/.cpp` (or `/Source/FC/Input/` if that module exists)
  - Fields (UPROPERTY EditDefaultsOnly):
    - `UInputMappingContext*` for each gameplay mode you need now (can start with one)
    - `UInputAction*` for core actions referenced in C++ (Interact, Click, etc.)
  - Keep the MVP minimal: only what is referenced by code today.
- [ ] Add a dedicated log category if the project uses them for input (optional).

**Testing After Step 1.4.1** ✅ CHECKPOINT
- [ ] Compile (User): ✅

**COMMIT POINT 1.4.1**: `git commit -m "feat(input): add UFCInputConfig data asset class"`

---

#### Step 1.4.2: Create DA_InputConfig_PlayerRuntime asset + wire it up
**Analysis**
- DataAsset must exist in Content and be assigned to the runtime system.

**Implementation**
- [ ] In Content Browser:
  - [ ] Create `DA_InputConfig_PlayerRuntime` under `/Content/FC/Input/Data/`
  - [ ] Assign Input Actions (`IA_*`) and Mapping Contexts (`IMC_*`) used by PlayerController/InputManager
- [ ] In `UFCInputManager` or PlayerController:
  - [ ] Add property `UPROPERTY(EditDefaultsOnly)` `TObjectPtr<UFCInputConfig> InputConfig;`
  - [ ] Replace any `ConstructorHelpers::FObjectFinder("/Game/FC/Input/IA_*")` with `InputConfig` access.
  - [ ] Validate config on BeginPlay; if missing, log one error and disable input setup safely.

**Testing After Step 1.4.2** ✅ CHECKPOINT
- [ ] PIE Office: input still works
- [ ] PIE Overworld/Camp: click/move still works
- [ ] Output Log: no missing asset crashes

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

