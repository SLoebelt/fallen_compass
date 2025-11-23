# Fallen Compass - Known Issues & Future Backlog

> **Living Document**: This backlog tracks known bugs, technical debt, enhancements, and architectural decisions deferred to future sprints. Items are organized by priority and include detailed context for future implementation.

**Last Updated**: November 23, 2025

---

## Table of Contents

1. [Issue #1: Office State Reset on Return from Overworld](#issue-1-office-state-reset-on-return-from-overworld)
2. [Issue #2: Pause Menu Abort Button Visibility Logic](#issue-2-pause-menu-abort-button-visibility-logic)
3. [Issue #3: Level Loading Order for State Transitions](#issue-3-level-loading-order-for-state-transitions)
4. [Issue #4: ESC Key Doesn't Close Pause Menu When Engine is Paused](#issue-4-esc-key-doesnt-close-pause-menu-when-engine-is-paused)

---

## Issue #1: Office State Reset on Return from Overworld

**Status**: 🐛 Bug - Needs Fix  
**Severity**: Medium  
**Priority**: High  
**Affected Systems**: Level transitions, PlayerController initialization, GameStateManager  
**Discovered**: Week 3 (Task 7)  
**Target Sprint**: Week 4 or 5

### Problem Summary

When aborting expedition from Overworld and returning to L_Office, the game state incorrectly transitions from `Office_Exploration` → `MainMenu`. This causes:

1. Input mode resets to UI-only instead of FirstPerson gameplay
2. Main menu appears when it shouldn't
3. Mouse look (IA_Look) stops working
4. Player cannot interact with office environment

### Root Cause

`InitializeMainMenu()` is called by L_Office Level Blueprint on level load. It currently doesn't check if a valid gameplay state (Office_Exploration) is already active from a previous transition. This was designed for initial game startup but conflicts with mid-game level transitions.

**Code Flow**:

```
1. User aborts expedition in Overworld
2. WBP_PauseMenu calls: GameStateManager->TransitionTo(Office_Exploration)
3. WBP_PauseMenu calls: LevelManager->LoadLevel("L_Office")
4. L_Office loads, Level Blueprint Event BeginPlay fires
5. Level Blueprint calls: PlayerController->InitializeMainMenu() (PROBLEM)
6. InitializeMainMenu() transitions state: Office_Exploration → MainMenu
7. Input mode set to UI-only, FirstPerson controls broken
```

### Expected Behavior

- Abort expedition in Overworld → Load L_Office → State should remain `Office_Exploration`
- Input mode should be `FirstPerson` with mouse look enabled
- Main menu should NOT appear
- Player should be able to move and interact with office objects

### Current Workaround

None. Feature is broken. Office is inaccessible after returning from Overworld.

### Proposed Solution

#### Option A: Conditional InitializeMainMenu (Recommended)

Modify `AFCPlayerController::InitializeMainMenu()` to check current state before transitioning:

```cpp
void AFCPlayerController::InitializeMainMenu()
{
    UFCGameStateManager* StateMgr = GetGameInstance()->GetSubsystem<UFCGameStateManager>();
    if (!StateMgr) return;

    EFCGameStateID CurrentState = StateMgr->GetCurrentState();

    // Only initialize main menu if we're starting fresh (None or MainMenu state)
    // If we're already in a gameplay state (Office_Exploration), preserve it
    if (CurrentState == EFCGameStateID::None || CurrentState == EFCGameStateID::MainMenu)
    {
        StateMgr->TransitionTo(EFCGameStateID::MainMenu);
        // ... existing main menu initialization ...
    }
    else
    {
        // Already in gameplay state - apply correct input/camera for that state
        ApplyInputAndCameraModeForState(CurrentState);
        UE_LOG(LogFallenCompassPlayerController, Log,
            TEXT("InitializeMainMenu: Already in gameplay state %s, preserving state"),
            *UEnum::GetValueAsString(CurrentState));
    }
}

void AFCPlayerController::ApplyInputAndCameraModeForState(EFCGameStateID State)
{
    switch (State)
    {
        case EFCGameStateID::Office_Exploration:
            SetCameraModeLocal(EFCPlayerCameraMode::FirstPerson, 0.5f);
            // Apply FirstPerson input context
            break;
        case EFCGameStateID::Overworld_Travel:
            SetCameraModeLocal(EFCPlayerCameraMode::TopDown, 0.5f);
            // Apply TopDown input context
            break;
        // ... other states ...
    }
}
```

#### Option B: Level Blueprint Check

Modify L_Office Level Blueprint to check state before calling InitializeMainMenu:

```
Event BeginPlay
├─ Get Game Instance
├─ Get GameStateManager Subsystem
├─ Get Current State
└─ Branch (Current State == None OR MainMenu)
    ├─ True: Call InitializeMainMenu()
    └─ False: Do nothing (state already valid)
```

#### Option C: SaveGame Flag

Add a flag to SaveGame system indicating "first launch" vs "mid-game level load":

```cpp
bool bIsFirstLaunch = SaveGameInstance->IsFirstLaunch();
if (bIsFirstLaunch)
{
    InitializeMainMenu();
    SaveGameInstance->SetFirstLaunch(false);
}
```

### Recommendation

**Option A** (Conditional InitializeMainMenu) - Most robust, handles all cases, centralizes logic in C++.

### Related Code

**Files to Modify**:

- `Source/FC/Core/FCPlayerController.h` (add ApplyInputAndCameraModeForState method)
- `Source/FC/Core/FCPlayerController.cpp` (modify InitializeMainMenu, implement helper)
- `Content/TopDown/Maps/L_Office.umap` (Level Blueprint - optional safety check)

**Files to Reference**:

- `Source/FC/Core/FCGameStateManager.h/.cpp` (GetCurrentState, state enum)
- `Source/FC/Core/UFCInputManager.h/.cpp` (input context switching)

**Key Lines**:

- `FCPlayerController.cpp::InitializeMainMenu()` (lines ~650)
- `FCPlayerController.cpp::BeginPlay()` (lines ~132)

### Testing Checklist

After implementing fix:

- [ ] Start new game → Main menu appears correctly ✓
- [ ] Start Journey from Office → Overworld loads ✓
- [ ] Abort Expedition → Return to Office ✓
- [ ] Verify: State is Office_Exploration (not MainMenu) ✓
- [ ] Verify: WASD movement works ✓
- [ ] Verify: Mouse look works ✓
- [ ] Verify: Can interact with map table ✓
- [ ] Round trip 3x: Office → Overworld → Office ✓
- [ ] No "Accessed None" errors in log ✓

---

## Issue #2: Pause Menu Abort Button Visibility Logic

**Status**: 🔧 Enhancement - UX Improvement  
**Severity**: Low  
**Priority**: Low  
**Affected Systems**: Pause menu widget, UI conditional display  
**Discovered**: Week 3 (Task 7)  
**Target Sprint**: Week 4 (low priority polish)

### Problem Summary

"Abort Expedition" button in WBP_PauseMenu is visible in both Office and Overworld levels because it only checks game state (`Paused`). The button should only appear when pausing from Overworld, not when pausing in Office.

**Impact**: Minor UX inconsistency. User sees button in Office but clicking it just loads L_Office (no harmful effect, just confusing).

### Root Cause

Button visibility is based on game state alone (`Paused`), but doesn't check:

1. Which level is currently loaded (L_Office vs L_Overworld)
2. Which state was pushed onto the stack before pause (Office_Exploration vs Overworld_Travel)

**Current Logic**:

```
WBP_PauseMenu Event Construct:
- Get Game State
- If state == Paused: Show button
- (Always shows because we're always paused when menu opens)
```

### Expected Behavior

- ESC in L_Overworld → Pause menu shows "Resume" + "Abort Expedition" buttons
- ESC in L_Office → Pause menu shows "Resume" + "Settings" + "Quit" buttons (no abort)
- Button visibility should be contextual based on level/previous state

### Current Workaround

User sees Abort button in Office. Clicking it loads L_Office (same level), so no crash or data loss. Just confusing UX.

### Proposed Solutions

#### Option A: Check Current Level (Simple)

In WBP_PauseMenu Event Construct, check level name:

```
Event Construct
├─ Get Owning Player World
├─ Get Map Name
├─ Contains "L_Overworld"?
    ├─ True: Set AbortExpeditionButton Visibility = Visible
    └─ False: Set AbortExpeditionButton Visibility = Collapsed
```

**Pros**: Simple, 5 minutes to implement  
**Cons**: Hardcoded level name check

#### Option B: Check State Stack (Robust)

Check GameStateManager state stack to see previous state before pause:

```
Event Construct
├─ Get Game Instance
├─ Get GameStateManager Subsystem
├─ Get State At Depth (0) // Bottom of stack = pre-pause state
├─ Branch (State == Overworld_Travel)
    ├─ True: Show Abort button
    └─ False: Hide Abort button
```

**Pros**: Robust, doesn't rely on level names  
**Cons**: Requires understanding state stack (10-15 min implementation)

#### Option C: Separate Pause Menus (Advanced)

Create two pause menu widgets:

- `WBP_PauseMenu_Office`: Resume, Settings, Quit
- `WBP_PauseMenu_Overworld`: Resume, Abort Expedition

UIManager spawns correct widget based on level/state.

**Pros**: Clean separation, extensible  
**Cons**: More work, duplicate widget layouts

### Recommendation

**Option A** (Check Level) for quick fix in Week 4 polish sprint. Can refactor to Option B later if needed.

### Related Code

**Files to Modify**:

- `Content/FC/UI/Menus/WBP_PauseMenu.uasset` (Designer Tab + Event Graph)

**Files to Reference**:

- `Source/FC/Core/FCPlayerController.cpp::HandlePausePressed()` (lines ~353)
- `Source/FC/Core/FCGameStateManager.h` (GetStateAtDepth method)

### Testing Checklist

After implementing fix:

- [ ] ESC in Office → Abort button NOT visible ✓
- [ ] ESC in Overworld → Abort button IS visible ✓
- [ ] Resume button works in both levels ✓
- [ ] No layout issues (button collapse doesn't break spacing) ✓

---

## Issue #3: Level Loading Order for State Transitions

**Status**: 📋 Architecture - Design Decision Needed  
**Severity**: Low (Currently works but inconsistent)  
**Priority**: Low  
**Affected Systems**: Widget transition logic, LevelManager, GameStateManager  
**Discovered**: Week 3 (Task 7)  
**Target Sprint**: Week 5+ (if transition complexity grows)

### Problem Summary

Widgets performing level transitions must call both `LevelManager->LoadLevel()` and `GameStateManager->TransitionTo()`. Current implementation order is inconsistent between widgets, and it's unclear if order matters.

### Current State

**Inconsistent Call Order**:

- **WBP_ExpeditionPlanning**:
  ```
  1. TransitionTo(Overworld_Travel)
  2. LoadLevel("L_Overworld")
  ```
- **WBP_PauseMenu** (Abort Expedition):
  ```
  1. TransitionTo(Office_Exploration)
  2. LoadLevel("L_Office")
  ```

**Working But Unclear**: Both work, but we don't know if order dependency exists or will break in future.

### Architectural Questions

1. **Does call order matter for correctness?**

   - State before level? Level before state?
   - Are there race conditions or timing issues?

2. **Should GameStateManager automatically trigger level loading?**

   - Example: TransitionTo(Overworld_Travel) auto-loads L_Overworld
   - Rejected: Breaks single responsibility principle

3. **Should widgets know about both systems?**
   - Current: Widgets call both LevelManager and GameStateManager
   - Alternative: Single unified API

### Proposed Solutions

#### Option 1: Status Quo + Documentation (Recommended)

Keep current pattern, establish standard order in documentation:

**Standard Pattern**:

```cpp
// 1. Transition state FIRST (logical change)
GameStateManager->TransitionTo(TargetState);

// 2. Load level SECOND (physical change)
LevelManager->LoadLevel(TargetLevel);
```

**Rationale**: State is logical, level is physical. State change should precede physical world change.

**Action Items**:

- Document pattern in Technical_Documentation.md ✓ (Done)
- Update all widgets to follow standard order
- Add comment in code explaining order importance

#### Option 2: Transition Facade

Create `UFCTransitionCoordinator` subsystem with unified API:

```cpp
class UFCTransitionCoordinator : public UGameInstanceSubsystem
{
public:
    // Single call handles both state and level transition
    void TransitionToLevel(FName LevelName, EFCGameStateID TargetState, bool bUseFade = true);
};
```

**Usage in Widgets**:

```
Get Game Instance
└─ Get TransitionCoordinator Subsystem
   └─ TransitionToLevel("L_Overworld", Overworld_Travel, true)
```

**Pros**: Clean API, encapsulates coordination logic  
**Cons**: New subsystem, migration cost for existing widgets

#### Option 3: PlayerController Helper

Add convenience method to AFCPlayerController:

```cpp
void AFCPlayerController::TransitionToLevel(const FName& LevelName, EFCGameStateID TargetState, bool bUseFade)
{
    UFCGameStateManager* StateMgr = GetGameInstance()->GetSubsystem<UFCGameStateManager>();
    UFCLevelManager* LevelMgr = GetGameInstance()->GetSubsystem<UFCLevelManager>();

    if (StateMgr) StateMgr->TransitionTo(TargetState);
    if (LevelMgr) LevelMgr->LoadLevel(LevelName, bUseFade);
}
```

**Pros**: Simple, no new subsystem  
**Cons**: PlayerController becomes transition coordinator (mixed responsibility)

### Recommendation

**Option 1** (Status Quo + Documentation) - Document standard pattern, update widgets to be consistent. Only move to Option 2 if transition complexity grows significantly (e.g., rollback support, async loading).

### Related Code

**Files to Update**:

- `Content/FC/UI/Planning/WBP_ExpeditionPlanning.uasset` (standardize order)
- `Content/FC/UI/Menus/WBP_PauseMenu.uasset` (standardize order)
- `Docs/Technical_Documentation.md` (add transition pattern section) ✓ (Done)

### Implementation Notes

**Standard Pattern to Follow**:

```
// Widget transition logic
Get Game Instance
├─ Get GameStateManager Subsystem
│  └─ TransitionTo(TargetState)
└─ Get LevelManager Subsystem
   └─ LoadLevel(TargetLevel, bUseFade)
```

**Order Rationale**: State represents logical game progression, level represents physical environment. Logical changes should precede physical manifestation.

---

## Issue #4: ESC Key Doesn't Close Pause Menu When Engine is Paused

**Status**: 🐛 Bug - Needs Fix  
**Severity**: Low  
**Priority**: Medium  
**Affected Systems**: Enhanced Input, Pause menu, Engine pause behavior  
**Discovered**: Week 3 (Task 8)  
**Target Sprint**: Week 4 or 5

### Problem Summary

When the pause menu is open in L_Overworld (with engine pause active via `SetPause(true)`), pressing ESC key a second time does not close the pause menu. This breaks the expected toggle behavior where ESC opens and closes the menu.

**Impact**: User must click "Resume" button instead of pressing ESC. Minor UX annoyance, not game-breaking.

### Root Cause

Engine pause (`SetPause(true)`) prevents Enhanced Input actions from firing. The IA_Escape input action is blocked when the game is paused, so `HandlePausePressed()` is never called to unpause.

**Why This Happens**:

```
1. User presses ESC in Overworld
2. HandlePausePressed() called (input active)
3. GameStateManager->PushState(Paused)
4. UIManager->ShowPauseMenu() called
5. ShowPauseMenu() detects L_Overworld, calls SetPause(true)
6. Engine pause activates, ALL input processing stops
7. User presses ESC again
8. Input system is paused, IA_Escape action never fires
9. HandlePausePressed() never called, menu stays open
```

**Why Office Works**:

- Office doesn't use engine pause (SetPause stays false)
- Enhanced Input continues processing
- ESC key toggles normally

### Expected Behavior

- Press ESC in Overworld → Pause menu opens, engine pauses
- Press ESC again → Pause menu closes, engine unpauses
- Consistent toggle behavior like in Office (where engine pause is not used)

### Current Workaround

Use "Resume" button instead of ESC key to close menu. Button works because it's UMG widget input, not Enhanced Input action.

### Proposed Solutions

#### Option A: Widget OnKeyDown Event (Quick Fix - Recommended)

WBP_PauseMenu captures ESC key directly via UMG input system:

**Implementation (Blueprint)**:

```
WBP_PauseMenu Widget Blueprint:
├─ Override: OnKeyDown(MyGeometry, InKeyEvent)
│  ├─ Get Key from InKeyEvent
│  ├─ Branch: Key == Escape?
│  │  ├─ True:
│  │  │  ├─ Get Owning Player
│  │  │  ├─ Cast to AFCPlayerController
│  │  │  ├─ Call ResumeGame()
│  │  │  └─ Return: Handled
│  │  └─ False: Return: Unhandled
```

**Pros**:

- Simple, 10-15 minutes to implement
- Works immediately
- No C++ changes needed

**Cons**:

- Duplicates input handling logic (ESC handled in two places)
- Couples widget to specific input key
- Less maintainable

#### Option B: FInputModeUIOnly + Manual Movement Stop (Robust)

Replace engine pause with input mode change + manual convoy disable:

**Implementation (C++ UFCUIManager)**:

```cpp
void UFCUIManager::ShowPauseMenu()
{
    // ... widget creation ...

    if (bIsOverworld)
    {
        // Don't use SetPause(true) - use input mode instead
        PC->SetInputMode(FInputModeUIOnly()); // Blocks gameplay input

        // Manually stop convoy AI
        ABP_OverworldConvoy* Convoy = FindConvoyInLevel();
        if (Convoy)
        {
            Convoy->StopMovement();
            Convoy->SetActorTickEnabled(false); // Pause AI tick
        }
    }
}
```

**Pros**:

- Enhanced Input continues working (ESC toggles properly)
- More control over what's paused vs active
- Cleaner architecture

**Cons**:

- More code (manual convoy management)
- 1-2 hours to implement and test
- Need reference to convoy actor

#### Option C: PlayerController Input While Paused (Experimental)

Configure PlayerController to accept input while paused:

```cpp
AFCPlayerController::AFCPlayerController()
{
    bShouldPerformFullTickWhenPaused = true; // Allow input processing when paused
}
```

**Pros**:

- Minimal code change
- Fixes ESC toggle immediately

**Cons**:

- May have unintended side effects (other systems might expect no input during pause)
- Needs extensive testing
- Affects all input, not just ESC

### Recommendation

**Option A** (Widget OnKeyDown) for **quick fix in Week 4**. Implements in Blueprint, no C++ rebuild needed, works immediately.

**Option B** (Input Mode + Manual Stop) for **proper fix in Week 5**. Better architecture, more maintainable, but requires more time.

### Related Code

**Option A Implementation (Blueprint)**:

- `Content/FC/UI/Menus/WBP_PauseMenu.uasset` (Override OnKeyDown function)
- `Source/FC/Core/FCPlayerController.h` (ResumeGame method already exists)

**Option B Implementation (C++)**:

- `Source/FC/Core/UFCUIManager.cpp::ShowPauseMenu()` (line ~110)
- `Source/FC/Core/UFCUIManager.cpp::HidePauseMenu()` (line ~147)
- `Content/FC/Overworld/BP_OverworldConvoy.uasset` (add StopMovement method)

**Option C Implementation (C++)**:

- `Source/FC/Core/FCPlayerController.cpp` (constructor)

### Testing Checklist

After implementing fix:

- [ ] ESC in Overworld → Menu opens ✓
- [ ] ESC again → Menu closes (NEW - should work) ✓
- [ ] Resume button still works ✓
- [ ] Convoy stops when paused ✓
- [ ] Convoy resumes when unpaused ✓
- [ ] ESC in Office still works (regression check) ✓
- [ ] Round trip ESC toggle 5x in Overworld ✓
- [ ] No "Accessed None" errors ✓

### Trade-off Analysis

| Criterion               | Option A               | Option B                  | Option C                 |
| ----------------------- | ---------------------- | ------------------------- | ------------------------ |
| **Implementation Time** | 15 min                 | 1-2 hours                 | 5 min                    |
| **Code Quality**        | Fair (duplicate logic) | Good (clean architecture) | Fair (side effects)      |
| **Maintainability**     | Low                    | High                      | Low                      |
| **Risk**                | Low                    | Low                       | Medium (unknown effects) |
| **Recommended For**     | Quick fix              | Long-term solution        | Experimental only        |

---

## Priority Summary

### High Priority (Week 4)

1. **Issue #1**: Office State Reset - Medium severity, blocks gameplay
   - Estimated: 2-3 hours (C++ implementation + testing)

### Medium Priority (Week 4-5)

2. **Issue #4**: ESC Toggle in Overworld - Low severity, UX annoyance
   - Estimated: 15 min (Option A) or 1-2 hours (Option B)

### Low Priority (Week 5+)

3. **Issue #2**: Abort Button Visibility - Low severity, minor UX inconsistency

   - Estimated: 15 min (simple Blueprint change)

4. **Issue #3**: Level Loading Order - Low severity, documentation task
   - Estimated: 30 min (standardize existing widgets)

---

## Document History

| Date       | Change                                        | Author       |
| ---------- | --------------------------------------------- | ------------ |
| 2025-11-23 | Initial backlog extraction from 0003-tasks.md | AI Assistant |

---

**END OF BACKLOG.MD**
