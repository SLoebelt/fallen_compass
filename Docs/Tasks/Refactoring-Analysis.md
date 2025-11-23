# Refactoring Task List - Architectural Improvements

**Date**: November 20, 2025  
**Status**: ACTIVE - Priorities 1-6 Complete, Priority 7+ Pending  
**Last Update**: Week 2 Complete - Priority 6 (Input Manager) completed ahead of schedule

---

## Executive Summary

**Completed (Priorities 1-6)**: All critical architecture refactorings completed in Week 1-2. UFCLevelManager, UFCUIManager, UFCGameStateManager subsystems implemented. UFCCameraManager and UFCInputManager components extracted. Level metadata system and state machine operational. All tests passing.

**Status**: Priority 6 (Input Manager) completed ahead of schedule in Week 2, enabling better input management for Week 3 Overworld features.

**Remaining Priorities**:

- **Priority 7** (SaveGame Versioning): Before Week 8 MVP testing
- **Priority 8** (Performance & Polish): Before Week 12 final polish

**Time Investment**:

- Priority 3: 6-8 hours ✅ COMPLETE
- Priority 4: 3-4 hours ✅ COMPLETE
- Priority 5: 4-5 hours ✅ COMPLETE
- Priority 6: 6-8 hours ✅ COMPLETE
- Priority 7: 3-4 hours (before Week 8)
- Priority 8: TBD (before Week 12)
- Total Completed: ~25 hours (Week 1-2)

**Achievement**: PlayerController successfully stabilized at manageable size through component extraction pattern. Input and camera management now reusable across multiple controller types.

---

## Table of Contents

- [Completion Status](#completion-status)
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

**Priority 3 - Camera & Component Refactoring** (Week 2)

- ✅ Refactoring 3A: UFCCameraManager Component

**Priority 4 - Table Widget Management** (Week 2)

- ✅ Refactoring 4A: Migrate Table Widgets to UFCUIManager

**Priority 5 - Advanced State Management** (Week 2)

- ✅ Refactoring 5A: State Stack for Pause/Modal States

**Priority 6 - Input Manager Component** (Week 2)

- ✅ Refactoring 6A: UFCInputManager Component

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

### Recommended Implementation Order

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
