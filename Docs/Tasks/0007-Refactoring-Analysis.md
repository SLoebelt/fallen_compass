# Refactoring Task List - Architectural Improvements

**Date**: November 20, 2025  
**Status**: ACTIVE - Priority 1 & 2 Complete, Priority 3+ Pending  
**Last Update**: Week 2 Complete - Table Interaction System Added

---

## Executive Summary

**Completed (Priorities 1 & 2)**: Core architecture refactorings completed in Week 1-2. UFCLevelManager, UFCUIManager, UFCGameStateManager subsystems implemented. Level metadata system and state machine operational. All tests passing.

**Current Focus (Priority 3-4)**: Before Week 3 features, must extract camera management and table widget management from PlayerController to prevent continued growth (currently 1240 lines, projected 3600+ by Week 21).

**Critical Path**:

- **Priority 3** (Camera) blocks Week 3 Overworld camera implementation
- **Priority 4** (Table UI) maintains consistency with existing UI patterns
- **Priority 5** (State Stack) required before Week 5 camp/combat complexity

**Time Investment**:

- Priority 3: 6-8 hours (before Week 3)
- Priority 4: 3-4 hours (before Week 3)
- Priority 5: 4-5 hours (during Week 3)
- Priority 7: 3-4 hours (before Week 8)
- Total: ~20 hours spread across 6 weeks

**Risk if Skipped**: PlayerController will grow uncontrollably (+350 lines by Week 6, +750 lines by Week 12), making maintenance nearly impossible. State management bugs will proliferate as complexity increases.

---

## Table of Contents

- [Completion Status](#completion-status)
- [Priority 3 - Camera & Component Refactoring (Before Week 3)](#priority-3---camera--component-refactoring-before-week-3)
- [Priority 4 - Table Widget Management (Before Week 3)](#priority-4---table-widget-management-before-week-3)
- [Priority 5 - Advanced State Management (Before Week 5)](#priority-5---advanced-state-management-before-week-5)
- [Priority 6 - Input Manager Component (Before Week 7)](#priority-6---input-manager-component-before-week-7)
- [Priority 7 - SaveGame Versioning (Before Week 8)](#priority-7---savegame-versioning-before-week-8)
- [Priority 8 - Performance & Polish (Before Week 12)](#priority-8---performance--polish-before-week-12)
- [Testing Strategy](#testing-strategy)
- [Dependencies & Scheduling](#dependencies--scheduling)

---

## Completion Status

### ✅ Completed Refactorings

**Priority 1 - Core Architecture** (Week 1-2)

- ✅ Refactoring 1A: UFCLevelManager Subsystem
- ✅ Refactoring 1B: UFCUIManager Subsystem
- ✅ Refactoring 1C: Widget Lifecycle Fix (Persistent Widgets)

**Priority 2 - Data-Driven & State Systems** (Week 2)

- ✅ Refactoring 2A: Level Metadata System (DT_LevelMetadata)
- ✅ Refactoring 2B: Explicit Game State Machine (UFCGameStateManager)

**Architecture Corrections**

- ✅ L_MainMenu architecture fix (removed non-existent level, main menu is a state in L_Office)
- ✅ PIE prefix normalization centralized in UFCLevelManager
- ✅ Blueprint widget coupling reduced (main menu widgets use UFCUIManager)

---

### 🔄 Current Technical Debt (Week 2 Analysis)

**PlayerController Growth**:

- Current size: 1240 lines (220 lines .h + 1020 lines .cpp)
- Net growth Week 1→2: +137 lines despite Priority 1 refactorings
- **Projection**: At current rate, will reach 3600+ lines by Week 21 (unmaintainable)

**New Issues Added in Week 2**:

1. **Table Widget Management**: Table widgets managed directly in PlayerController (inconsistent with main menu pattern)
2. **Camera Complexity**: SetCameraModeLocal() grew to 167 lines with 4 camera modes and dynamic camera spawning
3. **State Fragmentation**: Multiple overlapping state variables (CameraMode, CurrentGameState, bIsPauseMenuDisplayed, bIsInTableView)
4. **Timer Safety**: Inconsistent lambda capture patterns in 3+ locations

**Upcoming DRM Requirements Requiring Architecture**:

- Week 3: Overworld camera system (topdown, pan, zoom) → +150 lines if not refactored
- Week 4: Fog-of-war UI widget → +100 lines
- Week 5: Camp management UI → +120 lines
- Week 6: Combat camera + UI → +200 lines
- Week 7: Time/Resource HUD → +80 lines

---

## Priority 3 - Camera & Component Refactoring (Before Week 3)

**Target**: Complete before Overworld camera implementation  
**Effort**: 6-8 hours  
**Dependencies**: None (can start immediately)  
**Blocks**: Week 3 Overworld camera, Week 6 Combat camera

### Refactoring 3A: UFCCameraManager Component

**Problem**: Camera management scattered across 167 lines in PlayerController, hardcoded blend times, dynamic camera spawning complexity

**Current State (Week 2)**:

```cpp
// AFCPlayerController::SetCameraModeLocal() - 167 lines
// Handles 4 camera modes: FirstPerson, TableView, MainMenu, SaveSlotView
// Spawns/destroys temporary cameras dynamically
// Hardcoded 2.0f blend time throughout
// Complex cleanup logic with lambda timers
```

**Solution**: Extract to dedicated component

#### Implementation Steps

**Step 3A.1: Create UFCCameraManager Component (C++)**

- [x] Create `Source/FC/Components/FCCameraManager.h/.cpp`
- [x] Inherit from `UActorComponent`
- [x] Add camera mode enum: `EFCPlayerCameraMode` (FirstPerson, TableView, MainMenu, SaveSlotView, TopDown)
- [x] Add properties:
  - [x] `TObjectPtr<ACameraActor> MenuCamera`
  - [x] `TObjectPtr<ACameraActor> TableViewCamera`
  - [x] `TObjectPtr<AActor> OriginalViewTarget`
  - [x] `EFCPlayerCameraMode CurrentCameraMode`
  - [x] `float DefaultBlendTime = 2.0f` (data-driven, no more hardcoded values)
- [x] Compile successfully

**Step 3A.2: Implement Camera Transition API**

- [x] Add methods:
  - [x] `void BlendToMenuCamera(float BlendTime = -1.0f)` (-1 uses DefaultBlendTime)
  - [x] `void BlendToFirstPerson(float BlendTime = -1.0f)`
  - [x] `void BlendToTableObject(AActor* TableObject, float BlendTime = -1.0f)`
  - [x] `void BlendToTopDown(float BlendTime = -1.0f)` (Week 3 preparation)
  - [x] `void RestorePreviousViewTarget(float BlendTime = -1.0f)`
- [x] Add queries:
  - [x] `EFCPlayerCameraMode GetCameraMode() const`
  - [x] `AActor* GetOriginalViewTarget() const`
  - [x] `bool IsInTransition() const`
- [x] Compile successfully

**Step 3A.3: Extract Camera Logic from PlayerController**

- [x] Move camera blend logic from `AFCPlayerController::SetCameraModeLocal()` to component
- [x] Move camera spawning/destruction logic
- [x] Move timer-based cleanup logic (use FTimerDelegate::CreateUObject consistently)
- [x] Update PlayerController to delegate to CameraManager:

  ```cpp
  // FROM:
  SetCameraModeLocal(EFCPlayerCameraMode::TableView, 2.0f);

  // TO:
  CameraManager->BlendToTableObject(TableObject);
  ```

- [x] Compile successfully

**Step 3A.4: Blueprint Integration**

- [x] Open `BP_FC_PlayerController`
- [x] Add `UFCCameraManager` component (drag to component hierarchy)
- [x] Assign `MenuCamera` reference in component details
- [x] Save and compile Blueprint

**Step 3A.5: Testing**

- [x] PIE starts → Main menu camera works
- [x] Click "New Legacy" → Blend to first-person works
- [x] Click table object → Blend to table view works
- [x] Return to menu → Blend back to menu camera works
- [x] Check logs: All camera transitions logged by UFCCameraManager
- [x] Verify timer cleanup (no crashes on rapid transitions)

**Benefits**:

- ✅ Reduces PlayerController by ~167 lines
- ✅ Data-driven blend times (configurable in Blueprint)
- ✅ Reusable for TopDownPlayerController (Week 3+)
- ✅ Easier to add new camera modes (combat, camp)
- ✅ Consistent timer safety patterns

---

## Priority 4 - Table Widget Management (Before Week 3)

**Target**: Complete before additional table objects added  
**Effort**: 3-4 hours  
**Dependencies**: UFCUIManager (completed in Priority 1)  
**Blocks**: Week 2+ table interaction expansion

### Refactoring 4A: Migrate Table Widgets to UFCUIManager

**Problem**: Table widget lifecycle managed in PlayerController (inconsistent with main menu/pause menu pattern)

**Current State (Week 2)**:

```cpp
// AFCPlayerController - Lines 900-1020 (120 lines)
void OnTableObjectClicked(AActor* TableObject);
void ShowTableWidget(AActor* TableObject);
void CloseTableWidget();
UUserWidget* CurrentTableWidget; // Property in PlayerController
```

**Solution**: Move to UFCUIManager for consistency

#### Implementation Steps

**Step 4A.1: Extend UFCUIManager API (C++)**

- [ ] Add to `UFCUIManager.h`:

  ```cpp
  /** Show table widget for given table object type */
  UFUNCTION(BlueprintCallable, Category = "UI Management")
  void ShowTableWidget(AActor* TableObject);

  /** Close currently open table widget */
  UFUNCTION(BlueprintCallable, Category = "UI Management")
  void CloseTableWidget();

  /** Get current table widget (if any) */
  UFUNCTION(BlueprintPure, Category = "UI Management")
  UUserWidget* GetCurrentTableWidget() const { return CurrentTableWidget; }

  /** Check if table widget is open */
  UFUNCTION(BlueprintPure, Category = "UI Management")
  bool IsTableWidgetOpen() const;

  private:
    /** Widget class registry: TableObjectClass → WidgetClass */
    UPROPERTY()
    TMap<TSubclassOf<AActor>, TSubclassOf<UUserWidget>> TableWidgetMap;

    /** Currently displayed table widget */
    UPROPERTY()
    TObjectPtr<UUserWidget> CurrentTableWidget;
  ```

- [ ] Compile successfully

**Step 4A.2: Implement Table Widget Management**

- [ ] Implement `UFCUIManager::ShowTableWidget()`:
  - [ ] Look up widget class from `TableWidgetMap`
  - [ ] Create widget with `CreateWidget(GetGameInstance(), WidgetClass)`
  - [ ] Add to viewport with appropriate Z-order
  - [ ] Store in `CurrentTableWidget`
  - [ ] Log widget creation
- [ ] Implement `UFCUIManager::CloseTableWidget()`:
  - [ ] Remove from viewport
  - [ ] Clear `CurrentTableWidget` reference
  - [ ] Log widget closure
- [ ] Compile successfully

**Step 4A.3: Migrate PlayerController Code**

- [ ] Remove from `AFCPlayerController.h`:
  - [ ] `void ShowTableWidget(AActor* TableObject);`
  - [ ] `void CloseTableWidget();`
  - [ ] `TObjectPtr<UUserWidget> CurrentTableWidget;`
- [ ] Update `AFCPlayerController::OnTableObjectClicked()`:

  ```cpp
  // FROM:
  ShowTableWidget(TableObject);

  // TO:
  UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
  UFCUIManager* UIMgr = GI->GetSubsystem<UFCUIManager>();
  if (UIMgr)
  {
      UIMgr->ShowTableWidget(TableObject);
  }
  ```

- [ ] Compile successfully

**Step 4A.4: Configure Blueprint Widget Registry**

- [ ] Open `BP_FC_GameInstance`
- [ ] In UFCUIManager details, populate `TableWidgetMap`:
  - [ ] Key: `BP_TableObject_Map`, Value: `WBP_MapTable`
  - [ ] Key: `BP_TableObject_Logbook`, Value: `WBP_Logbook` (when created)
  - [ ] Key: `BP_TableObject_Letters`, Value: `WBP_MessagesHub` (when created)
- [ ] Save and compile Blueprint

**Step 4A.5: Testing**

- [ ] PIE → Walk to table
- [ ] Click map object → `WBP_MapTable` appears
- [ ] Close widget → Returns to gameplay correctly
- [ ] Check logs: UFCUIManager handles widget lifecycle
- [ ] Verify consistency with main menu/pause menu patterns

**Benefits**:

- ✅ Reduces PlayerController by ~120 lines
- ✅ Consistent UI management across all widget types
- ✅ Easy to add new table objects (just add to registry)
- ✅ Single source of truth for UI configuration

---

## Priority 5 - Advanced State Management (Before Week 5)

**Target**: Complete before camp/combat state complexity  
**Effort**: 4-5 hours  
**Dependencies**: UFCGameStateManager (completed in Priority 2B)  
**Blocks**: Week 5+ camp substates, Week 6+ combat turn states

### Refactoring 5A: State Stack for Pause/Modal States

**Problem**: No support for nested states (e.g., Paused while in TableView)

**Current State**:

```cpp
// UFCGameStateManager - Basic transition only
bool TransitionTo(EFCGameStateID NewState);
bool CanTransitionTo(EFCGameStateID NewState) const;

// No state history beyond PreviousState
// No stack for returning from pause/modal dialogs
```

**Solution**: Add state stack support

#### Implementation Steps

**Step 5A.1: Extend State Manager API**

- [ ] Add to `UFCGameStateManager.h`:

  ```cpp
  /** Push new state onto stack (for pause/modal) */
  UFUNCTION(BlueprintCallable, Category = "Game State")
  void PushState(EFCGameStateID NewState);

  /** Pop state and return to previous */
  UFUNCTION(BlueprintCallable, Category = "Game State")
  bool PopState();

  /** Get state stack depth */
  UFUNCTION(BlueprintPure, Category = "Game State")
  int32 GetStateStackDepth() const { return StateStack.Num(); }

  /** Get state at stack position (0 = bottom, -1 = top) */
  UFUNCTION(BlueprintPure, Category = "Game State")
  EFCGameStateID GetStateAtDepth(int32 Depth) const;

  private:
    /** State stack for pause/modal states */
    TArray<EFCGameStateID> StateStack;
  ```

- [ ] Compile successfully

**Step 5A.2: Implement Stack Operations**

- [ ] Implement `PushState()`:
  - [ ] Push current state to stack
  - [ ] Transition to new state
  - [ ] Log stack operation
- [ ] Implement `PopState()`:
  - [ ] Validate stack not empty
  - [ ] Pop state from stack
  - [ ] Transition to popped state
  - [ ] Return success/failure
- [ ] Compile successfully

**Step 5A.3: Update Pause Logic**

- [ ] Update `AFCPlayerController::HandlePausePressed()`:

  ```cpp
  // FROM: Manual state checking

  // TO: Use state stack
  if (StateMgr->GetCurrentState() == EFCGameStateID::Paused)
  {
      StateMgr->PopState(); // Return to state before pause
  }
  else
  {
      StateMgr->PushState(EFCGameStateID::Paused);
  }
  ```

- [ ] Compile successfully

**Step 5A.4: Testing**

- [ ] PIE → Enter gameplay
- [ ] Press Pause → State: Office_Exploration → Paused (stack: [Office_Exploration])
- [ ] Press Pause again → State: Paused → Office_Exploration (stack: [])
- [ ] Open table widget → State: Office_TableView
- [ ] Press Pause → State: Office_TableView → Paused (stack: [Office_TableView])
- [ ] Press Pause again → Returns to Office_TableView correctly
- [ ] Check logs: All stack operations logged

**Benefits**:

- ✅ Supports nested states (pause while in any state)
- ✅ Proper "return to previous" behavior
- ✅ Foundation for modal dialogs (confirmation, etc.)
- ✅ Stack depth tracking for debugging

---

## Priority 6 - Input Manager Component (Before Week 7)

**Target**: Complete before complex input contexts (combat, camp tasks)  
**Effort**: 6-8 hours  
**Dependencies**: None  
**Blocks**: Week 6+ combat input, Week 7+ camp task input

### Refactoring 6A: UFCInputManager Component

**Problem**: Input mapping mode management scattered, Enhanced Input contexts managed in PlayerController

**Solution**: Extract to dedicated component (OPTIONAL - defer if time constrained)

#### Implementation Steps

**Step 6A.1: Create UFCInputManager Component**

- [ ] Create `Source/FC/Components/FCInputManager.h/.cpp`
- [ ] Inherit from `UActorComponent`
- [ ] Move `EFCInputMappingMode` enum to component
- [ ] Add Input Mapping Context registry
- [ ] Compile successfully

**Step 6A.2: Extract Input Logic**

- [ ] Move `SetInputMappingMode()` logic to component
- [ ] Move context switching logic
- [ ] Move input binding setup
- [ ] Update PlayerController to delegate to component
- [ ] Compile successfully

**Step 6A.3: Blueprint Integration & Testing**

- [ ] Add component to BP_FC_PlayerController
- [ ] Test all input modes (FirstPerson, TopDown, Fight, StaticScene)
- [ ] Verify Enhanced Input contexts switch correctly

**Note**: This refactoring is OPTIONAL. Only implement if time allows before Week 7.

---

## Priority 7 - SaveGame Versioning (Before Week 8)

**Target**: Complete before MVP vertical slice testing  
**Effort**: 3-4 hours  
**Dependencies**: None  
**Blocks**: Phase 2+ save compatibility

### Refactoring 7A: Save Data Migration System

**Problem**: No versioning strategy for save file format changes

**Solution**: Implement save version tracking and migration hooks

#### Implementation Steps

**Step 7A.1: Add Version Tracking**

- [ ] Add to `UFCSaveGame.h`:

  ```cpp
  /** Save file format version */
  UPROPERTY(SaveGame)
  int32 SaveVersion = 1;

  /** Game build version when saved */
  UPROPERTY(SaveGame)
  FString GameVersion;
  ```

- [ ] Compile successfully

**Step 7A.2: Create Migration Framework**

- [ ] Add to `UFCGameInstance`:

  ```cpp
  /** Migrate save data from old version to current */
  bool MigrateSaveData(UFCSaveGame* SaveGame, int32 FromVersion, int32 ToVersion);

  /** Register version-specific migration functions */
  TMap<int32, TFunction<void(UFCSaveGame*)>> SaveMigrations;
  ```

- [ ] Implement version check in `LoadGameAsync()`
- [ ] Log migration operations
- [ ] Compile successfully

**Step 7A.3: Testing**

- [ ] Create test save files with different versions
- [ ] Verify migration triggers correctly
- [ ] Verify backward compatibility (can load old saves)
- [ ] Verify forward protection (can't load future saves)

**Benefits**:

- ✅ Supports save format evolution
- ✅ Backward compatibility with old saves
- ✅ Clear migration audit trail
- ✅ Prevents corruption from version mismatches

---

## Priority 8 - Performance & Polish (Before Week 12)

**Target**: Complete during polish phase  
**Effort**: 4-6 hours  
**Dependencies**: All previous refactorings  
**Blocks**: None (optimization phase)

### Refactoring 8A: Timer Safety Audit

**Problem**: Inconsistent lambda capture patterns in timer delegates

**Solution**: Audit all timer usage and standardize on safe patterns

#### Implementation Steps

**Step 8A.1: Find All Timer Usage**

- [ ] Search codebase for `GetWorldTimerManager()` and `SetTimer()`
- [ ] Document all timer lambda captures
- [ ] Identify unsafe patterns (raw pointer captures)

**Step 8A.2: Standardize Timer Patterns**

- [ ] Replace all timers with `FTimerDelegate::CreateUObject(this, &Class::Method)`
- [ ] Remove inline lambda captures where possible
- [ ] Add validation (check `IsValid()` before accessing captured pointers)
- [ ] Compile successfully

**Step 8A.3: Testing**

- [ ] Rapid transition testing (spam buttons, trigger rapid state changes)
- [ ] Verify no crashes from stale timer callbacks
- [ ] Check logs for timer cleanup messages

---

### Refactoring 8B: Blueprint Exposure Audit

**Problem**: Too much C++ exposed to Blueprint without clear boundaries

**Solution**: Review and restrict Blueprint exposure to intentional API

#### Implementation Steps

**Step 8B.1: Audit BlueprintCallable Functions**

- [ ] Review all `UFUNCTION(BlueprintCallable)` declarations
- [ ] Identify functions that should be C++-only
- [ ] Document intended Blueprint API surface

**Step 8B.2: Restrict Unnecessary Exposure**

- [ ] Remove `BlueprintCallable` from internal methods
- [ ] Add `BlueprintProtected` where appropriate
- [ ] Update documentation comments
- [ ] Compile successfully

**Step 8B.3: Testing**

- [ ] Open all Blueprints and verify no broken references
- [ ] Ensure intended functionality still accessible
- [ ] Document final Blueprint API in Technical_Documentation.md

---

## Testing Strategy

### Mandatory Testing Checklist (Run After Each Refactoring)

```
[ ] PIE starts successfully without crashes
[ ] Main menu appears correctly
[ ] "New Legacy" button works and transitions to gameplay
[ ] Can walk around office with WASD + mouse
[ ] F6 QuickSave works (check logs for success)
[ ] Door interaction returns to menu (fade + reload)
[ ] "Continue" button loads save correctly
[ ] Table object interaction works (Week 2+)
[ ] Camera transitions smooth (no black screens)
[ ] No "Accessed None" Blueprint errors
[ ] No crashes or errors in Output Log
[ ] Live Coding or manual recompile succeeded
```

### Regression Testing Matrix

| Test Scenario      | Priority 3   | Priority 4   | Priority 5   | Priority 6   | Priority 7   | Priority 8   |
| ------------------ | ------------ | ------------ | ------------ | ------------ | ------------ | ------------ |
| Main Menu Flow     | ✅           | ✅           | ✅           | ✅           | ✅           | ✅           |
| New Legacy         | ✅           | ✅           | ✅           | ✅           | ✅           | ✅           |
| Camera Transitions | **🎯 FOCUS** | ✅           | ✅           | ✅           | ✅           | ✅           |
| Table Interaction  | ✅           | **🎯 FOCUS** | ✅           | ✅           | ✅           | ✅           |
| Pause Menu         | ✅           | ✅           | **🎯 FOCUS** | ✅           | ✅           | ✅           |
| QuickSave/Load     | ✅           | ✅           | ✅           | ✅           | **🎯 FOCUS** | ✅           |
| Input Contexts     | ✅           | ✅           | ✅           | **🎯 FOCUS** | ✅           | ✅           |
| Rapid Transitions  | ✅           | ✅           | ✅           | ✅           | ✅           | **🎯 FOCUS** |

---

## Dependencies & Scheduling

### Dependency Graph

```
Priority 1 (✅ Complete) ──┐
Priority 2 (✅ Complete) ──┤
                          ├──> Priority 3 (Camera) ──┐
                          │                          ├──> Priority 5 (State Stack)
                          └──> Priority 4 (Table UI) ─┘

Priority 3, 4, 5 (All Complete) ──> Priority 6 (Input) ──┐
                                                         ├──> Priority 8 (Polish)
Priority 1-6 (All Complete) ──────> Priority 7 (SaveGame) ┘
```

### Recommended Implementation Order

**Week 2 (Current)**: ✅ Priorities 1 & 2 Complete

**Before Week 3 Start** (November 21-22):

1. **Priority 3**: UFCCameraManager Component (6-8 hours)
   - Blocks: Week 3 Overworld camera
   - Critical: Prevents +150 line growth in PlayerController
2. **Priority 4**: Table Widget Management (3-4 hours)
   - Blocks: Week 2+ table expansion
   - Critical: Maintains UI consistency

**During Week 3** (November 23-26): 3. **Priority 5**: State Stack Implementation (4-5 hours)

- Blocks: Week 5+ camp/combat states
- Can be done in parallel with Week 3 features

**Week 4-6** (As Time Allows): 4. **Priority 6**: Input Manager Component (6-8 hours) - OPTIONAL

- Nice to have before combat input
- Can defer if time constrained

5. **Priority 7**: SaveGame Versioning (3-4 hours)
   - Important before Phase 2
   - Can be done anytime before Week 8

**Week 7-12** (Polish Phase): 6. **Priority 8**: Performance & Polish (4-6 hours)

- Timer safety audit
- Blueprint exposure cleanup
- Final optimization pass

---

## Risk Mitigation

### If Refactorings Are Skipped

**Skip Priority 3 (Camera)**: Week 3 PlayerController will grow by ~150 lines for Overworld camera, ~200 more for combat camera (Week 6). Total growth: +350 lines → **1590 lines total**.

**Skip Priority 4 (Table UI)**: Each new table object adds ~40 lines to PlayerController. Week 13 adds 3+ table objects → +120 lines.

**Skip Priority 5 (State Stack)**: Manual state management becomes increasingly fragile. Camp substates (Week 12) and combat turns (Week 6) require complex state tracking. Risk of invalid state bugs.

**Skip Priority 6 (Input)**: Input management complexity remains in PlayerController. Not critical but increases maintenance burden.

**Skip Priority 7 (SaveGame)**: Save files will break between versions. Players lose progress. Critical for Phase 2+ development.

**MANDATORY TESTING CHECKLIST** (Run After Each Step):

```
[ ] PIE starts successfully without crashes
[ ] Main menu appears correctly
[ ] "New Legacy" button works and transitions to gameplay
[ ] Can walk around office with WASD + mouse
[ ] F6 QuickSave works (check logs for success)
[ ] Door interaction returns to menu (fade + reload)
[ ] "Continue" button loads save correctly
[ ] No crashes or errors in Output Log
[ ] No "Accessed None" Blueprint errors
[ ] Live Coding or manual recompile succeeded
```
