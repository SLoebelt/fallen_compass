# Refactoring Analysis - Week 1 Task 5 Retrospective

**Date**: November 16, 2025  
**Status**: CONCEPT - NO IMPLEMENTATION YET  
**Scope**: Analysis of current implementations against future scalability requirements

---

## Table of Contents

- [Refactoring Checklist](#refactoring-checklist)
- [Executive Summary](#executive-summary)
- [Key Findings Summary](#key-findings-summary)
  - [✅ What's Working Well](#️-whats-working-well)
  - [⚠️ Critical Issues Requiring Refactoring](#️-critical-issues-requiring-refactoring)
  - [🔶 Medium Priority Issues](#-medium-priority-issues)
- [Blueprint Architecture Analysis](#blueprint-architecture-analysis)
  - [Current Blueprint Assets](#current-blueprint-assets)
  - [Blueprint Architecture Assessment](#blueprint-architecture-assessment)
    - [✅ What's Good](#️-whats-good)
    - [⚠️ Concerns Found](#️-concerns-found)
  - [Blueprint Refactoring Recommendations](#blueprint-refactoring-recommendations)
    - [Priority 1 (With C++ Refactoring)](#priority-1-with-c-refactoring)
    - [Priority 2 (Nice to Have)](#priority-2-nice-to-have)
    - [Blueprint Changes Needed for Priority 1 Refactoring](#blueprint-changes-needed-for-priority-1-refactoring)
    - [Testing Strategy for Blueprint Changes](#testing-strategy-for-blueprint-changes)
- [Detailed Analysis](#detailed-analysis)
  - [Issue 1: Level Tracking Responsibility Split (CRITICAL)](#issue-1-level-tracking-responsibility-split-critical)
  - [Issue 2: PlayerController God Object (CRITICAL)](#issue-2-playercontroller-god-object-critical)
  - [Issue 3: Widget Lifecycle Architecture (CRITICAL for Task 514)](#issue-3-widget-lifecycle-architecture-critical-for-task-514)
  - [Issue 4: Hard-coded Level Name Dependencies (HIGH PRIORITY)](#issue-4-hard-coded-level-name-dependencies-high-priority)
  - [Issue 5: Missing State Machine (MEDIUM-HIGH-priority)](#issue-5-missing-state-machine-medium-high-priority)
- [Testing Strategy](#testing-strategy)
- [Proposed Implementation Order](#proposed-implementation-order)
- [Risks if We Don't Refactor](#risks-if-we-dont-refactor)
- [Recommendation](#recommendation)
- [Next Steps](#next-steps)

---

## Refactoring Checklist

### Priority 1 – Core Architecture (Before Week 2)

#### Refactoring 1A – UFCLevelManager Subsystem

- [ ] **Design & Implementation**
  - [ ] Create `UFCLevelManager` as `UGameInstanceSubsystem`
  - [ ] Centralize `CurrentLevelName` in `UFCLevelManager`
  - [ ] Implement `GetCurrentLevelName()` and normalization (PIE prefix handling)
  - [ ] Implement enum-based `EFCLevelType` and type queries (`IsMenuLevel()`, `IsGameplayLevel()`, etc.)
- [ ] **Integration**
  - [ ] Update `UFCTransitionManager` to use `UFCLevelManager`
  - [ ] Update `UFCGameInstance` load/restore logic to use `UFCLevelManager`
  - [ ] Update `AFCPlayerController::BeginPlay()` to use `UFCLevelManager` (no direct `GetMapName` / string Contains)
- [ ] **Testing**
  - [ ] PIE: Start game → main menu level detected correctly
  - [ ] PIE: Transition to gameplay level → correct level type
  - [ ] PIE: Return to menu → level type updated correctly
  - [ ] Logs: No remaining string-based level checks (`Contains("Office")`, `Contains("MainMenu")`)

---

#### Refactoring 1B – UFCUIManager Subsystem + Blueprint Decoupling

- [ ] **Design & Implementation (C++)**
  - [ ] Create `UFCUIManager` as `UGameInstanceSubsystem`
  - [ ] Move UI-related responsibilities from `AFCPlayerController` into `UFCUIManager`
    - [ ] Main menu display & hide
    - [ ] Save slot selector display & hide
    - [ ] “New Legacy”, “Continue”, “Load”, “Options”, “Quit” handlers
  - [ ] Own widget references in `UFCUIManager` (MainMenu, SaveSlotSelector, etc.)
- [ ] **Blueprint Configuration**
  - [ ] `BP_FC_GameInstance`
    - [ ] Add `UIManagerClass` (if used) and ensure subsystem is available
    - [ ] Move `MainMenuWidgetClass` reference here
    - [ ] Move `SaveSlotSelectorWidgetClass` reference here
  - [ ] `BP_FC_PlayerController`
    - [ ] Remove `MainMenuWidgetClass` and `SaveSlotSelectorWidgetClass`
    - [ ] Keep `MenuCamera` reference only
  - [ ] `WBP_MainMenu`
    - [ ] Replace calls to `Get Player Controller → AFCPlayerController::On*Clicked`
    - [ ] Use `Get Game Instance → Get Subsystem (UFCUIManager) → On*Clicked` instead
  - [ ] `WBP_SaveSlotSelector`
    - [ ] Replace direct `PlayerController->LoadSaveSlot()` calls
    - [ ] Route through `UFCUIManager->LoadSaveSlot()` via subsystem
  - [ ] `WBP_SaveSlotItem`
    - [ ] (Simple) Call `UFCUIManager` directly **or**
    - [ ] (Better) Use dispatcher: `OnSlotSelected(SlotName)` → handled by parent widget → `UFCUIManager`
- [ ] **Testing**
  - [ ] Start game → main menu appears as before
  - [ ] “New Legacy” button → gameplay transition still works
  - [ ] “Continue” button → loads latest save correctly
  - [ ] “Load Save” → opens save slot selector, selecting slot loads as expected
  - [ ] “Options” & “Quit” → expected behavior (even if placeholder)
  - [ ] Door interaction → return to main menu still works
  - [ ] No direct widget → PlayerController calls remain for menu logic
  - [ ] Logs: `UFCUIManager` methods are hit, not old `AFCPlayerController` ones

---

#### Refactoring 1C – Widget Lifecycle / Persistent Transition Widget

- [ ] **Implementation**
  - [ ] Update `UFCTransitionManager::CreateTransitionWidget()`
    - [ ] Use `GetGameInstance()` as widget outer, not `PlayerController`
    - [ ] Use `UGameViewportClient::AddViewportWidgetContent()` instead of `AddToViewport()`
  - [ ] Ensure `WBP_ScreenTransition` / `UFCScreenTransitionWidget` initializes in a fully black state
  - [ ] Implement logic so the widget persists across `OpenLevel` calls
- [ ] **Task 5.14 Implementation**
  - [ ] On game start, transition widget starts black and fades in
  - [ ] On cross-level load, fade out → load → fade in re-uses same widget
- [ ] **Testing**
  - [ ] PIE: Start game → see fade-in from black (no flash/pop)
  - [ ] Trigger same-level transition → correct blend behavior, no extra widgets
  - [ ] Trigger cross-level transition → widget persists, fade-out/in works
  - [ ] No duplicate transition widgets or orphaned UI
  - [ ] No crashes when quickly triggering multiple transitions

---

### Priority 2 – Data-Driven & State Systems (Before Week 4)

#### Refactoring 2A – Level Metadata System (Data-Driven Level Config)

- [ ] **Design & Implementation (C++)**
  - [ ] Create `FFCLevelMetadata` struct (row for DataTable)
  - [ ] Add metadata lookup API in `UFCLevelManager` (e.g. `GetLevelMetadata(LevelName)`)
- [ ] **Blueprint / Content**
  - [ ] Create `DT_LevelMetadata` Data Table using `FFCLevelMetadata`
  - [ ] Add rows for at least:
    - [ ] `L_Office`
    - [ ] `L_MainMenu`
    - [ ] `L_TopDown` / over­world test levels (if existing)
  - [ ] Configure:
    - [ ] `LevelType`
    - [ ] `DefaultInputMode`
    - [ ] `bRequiresFadeTransition`
    - [ ] `bShowCursor`
    - [ ] `DisplayName`
  - [ ] `BP_FC_GameInstance`: Reference `DT_LevelMetadata` on `UFCLevelManager`
- [ ] **Integration**
  - [ ] Replace remaining string name checks with metadata-based logic
    - [ ] No more `Contains("Office")` / `Contains("MainMenu")`
- [ ] **Testing**
  - [ ] Boot to main menu → correct metadata read, correct input mode / cursor
  - [ ] Transition to office → metadata drives correct mapping mode
  - [ ] Adding a new test level via Data Table requires **no** C++ changes

---

#### Refactoring 2B – Explicit Game State Machine

- [ ] **Design & Implementation**
  - [ ] Create `UFCGameStateManager` as `UGameInstanceSubsystem`
  - [ ] Define `EFCGameStateID` enum (MainMenu, Office_Exploration, Office_TableView, Overworld_Travel, Combat_PlayerTurn, etc.)
  - [ ] Implement:
    - [ ] `GetCurrentState()`, `GetPreviousState()`
    - [ ] `TransitionTo(State)`, `CanTransitionTo(State)`
    - [ ] Optional: state stack (`PushState`, `PopState`) for Pause / modal states
  - [ ] Define valid state transitions and initialize them
  - [ ] Add `OnStateChanged` multicast delegate
- [ ] **Integration**
  - [ ] Remove/replace old `CurrentGameState` from `AFCPlayerController`
  - [ ] Update all game state changes to go through `UFCGameStateManager`
- [ ] **Testing**
  - [ ] Log all state transitions and verify expected order (MainMenu → Office_Exploration → Pause → Office_Exploration → etc.)
  - [ ] Invalid transitions are blocked (and logged)
  - [ ] Optional debug widget: `WBP_StateDebugWidget` correctly reflects current state and history

---

### Priority 3 – Components & Polish (After Week 4)

#### Refactoring 3A – CameraManager Component

- [ ] **Design & Implementation**
  - [ ] Create `UFCCameraManager` as `UActorComponent`
  - [ ] Move camera-related responsibilities from `AFCPlayerController`
    - [ ] Camera mode state (`EFCPlayerCameraMode`)
    - [ ] View target switching (menu camera, first-person, table view, etc.)
    - [ ] Blend times and logic
  - [ ] Expose clean functions:
    - [ ] `BlendToMenuCamera()`
    - [ ] `BlendToFirstPerson()`
    - [ ] `BlendToTopDown()` (future)
- [ ] **Blueprint Integration**
  - [ ] Attach `UFCCameraManager` component to `BP_FC_PlayerController`
  - [ ] Ensure `BP_MenuCamera` references are routed through `UFCCameraManager`
- [ ] **Testing**
  - [ ] Main menu camera works as before
  - [ ] New Legacy → blend to gameplay camera works
  - [ ] Return to menu → blend back to menu camera works
  - [ ] No regressions in camera behavior

---

### Global Validation

- [ ] All Priority 1 refactorings merged into main branch
- [ ] All Priority 2 refactorings merged (when planned)
- [ ] Regression test suite (all menu, transition, save/load flows) passes
- [ ] Technical docs updated (UE_CodeConventions.md / architecture docs)
- [ ] This checklist updated with actual completion dates and notes

---

## Executive Summary

After 3 days of intensive development completing Task 5 (Main Menu System) and implementing Phase 2 (Smart Load Transitions), we have a **functionally complete and working system**. However, analyzing the implementation against:

- Project coding conventions (UE_CodeConventions.md)
- Future requirements (GDD § 3.1, 3.2, 4.1)
- Development roadmap (DRM Weeks 2-8)
- Unreal Engine best practices

...reveals **several architectural concerns** that should be addressed now while the codebase is small, before technical debt accumulates.

---

## Key Findings Summary

### ✅ What's Working Well

1. **Modular Transition System** - `UFCTransitionManager` as subsystem is excellent
2. **Game State Persistence** - `UFCGameInstance` correctly owns long-lived data
3. **Enhanced Input System** - Proper context-based mapping with clean separation
4. **Save/Load Foundation** - `UFCSaveGame` structure is clean and extensible

### ⚠️ Critical Issues Requiring Refactoring

1. **Level Tracking Responsibility Split** - Logic scattered across 3 classes
2. **PlayerController God Object** - Too many responsibilities violating SRP
3. **Widget Lifecycle Architecture** - Prevents persistent UI features (Task 5.14)
4. **Hard-coded Level Name Dependencies** - String matching will break at scale
5. **Missing State Machine** - Game state logic is implicit, not explicit

### 🔶 Medium Priority Issues

6. **Tight Coupling** - GameInstance → PlayerController → TransitionManager circular dependencies
7. **Blueprint Exposure** - Too much C++ logic exposed without clear API boundaries
8. **Input Restoration Fragility** - Manual coordination across multiple systems
9. **Save Data Versioning** - No migration strategy for future changes
10. **Timer Safety** - Some lambda captures may still cause issues

---

## Blueprint Architecture Analysis

### Current Blueprint Assets

Based on Technical Documentation review, we have these Blueprint implementations:

#### Core Blueprints

- **BP_FC_GameInstance** - Child of UFCGameInstance
  - Configures `TransitionWidgetClass` (points to WBP_ScreenTransition)
  - Currently minimal Blueprint logic (good!)
- **BP_FC_PlayerController** - Child of AFCPlayerController

  - Assigns MenuCamera reference in editor
  - Assigns MainMenuWidgetClass (points to WBP_MainMenu)
  - Assigns SaveSlotSelectorWidgetClass
  - Currently minimal Blueprint logic (good!)

- **BP_FC_GameMode** - Child of AFCGameMode
  - Minimal configuration
  - Could potentially be removed (using C++ directly)

#### World Blueprints

- **BP_MenuCamera** - ACameraActor
  - Placed in L_Office level
  - No custom logic (just positioned)
- **BP_MenuDoor** - Implements IFCInteractable

  - OnInteract() calls PlayerController->ReturnToMainMenu()
  - Has custom interaction prompt
  - Simple, focused responsibility ✅

- **BP_OfficeDesk** - Static mesh with SceneComponent
  - Has CameraTargetPoint for table view
  - No complex logic
- **BP_Candle** - Visual prop
  - Likely has particle effects/materials
  - Atmospheric detail

#### UI Widgets

- **WBP_MainMenu** - UUserWidget

  - Contains button layout
  - Buttons call AFCPlayerController methods directly
  - Clean separation: UI structure in BP, logic in C++ ✅

- **WBP_MainMenuButton** - UUserWidget

  - Reusable button template
  - Hover states, styling
  - No complex logic ✅

- **WBP_ScreenTransition** - Child of UFCScreenTransitionWidget

  - Inherits C++ fade logic
  - Blueprint adds visual design/layout
  - Good inheritance pattern ✅

- **WBP_SaveSlotSelector** - UUserWidget

  - List of save slots
  - Uses WBP_SaveSlotItem for each entry
  - Calls PlayerController->LoadSaveSlot()

- **WBP_SaveSlotItem** - UUserWidget
  - Individual save slot entry
  - Hover states, thumbnail, metadata display

### Blueprint Architecture Assessment

#### ✅ What's Good

1. **Minimal Blueprint Logic** - Most Blueprints are pure configuration/visual
   - BP_FC_GameInstance: Just sets class references
   - BP_FC_PlayerController: Just assigns references
   - UI widgets: Mostly layout/styling
2. **Clear Inheritance** - Blueprints extend C++ classes cleanly
   - WBP_ScreenTransition inherits UFCScreenTransitionWidget
   - Widget callbacks go to C++ methods
3. **Focused Responsibilities** - Each Blueprint has single purpose
   - BP_MenuDoor: Just interaction logic
   - WBP_MainMenuButton: Just button presentation
4. **Following Conventions** - Aligns with UE_CodeConventions.md
   - "Expose selectively - Only expose what designers need"
   - "Extend via Blueprint events - Allows designers to extend without modifying C++"

#### ⚠️ Concerns Found

1. **Direct PlayerController Method Calls from Widgets**

   Current: `WBP_MainMenu → AFCPlayerController::OnNewLegacyClicked()`

   **Problem**: This creates tight coupling Widget → PlayerController

   **Future Impact**:

   - If we refactor PlayerController (Priority 1), we break ALL widget bindings
   - Can't reuse widgets with different controllers
   - Hard to test widgets in isolation

   **Better Pattern** (matches Priority 1 refactoring):

   ```
   WBP_MainMenu → UFCUIManager::ShowMainMenu()
   UFCUIManager → PlayerController/GameState as needed
   ```

2. **BP_FC_GameMode May Be Redundant**

   If it only exists to be selectable in editor and has no Blueprint logic, we could:

   - Use C++ AFCGameMode directly in World Settings
   - Saves an asset, reduces indirection
   - Only keep BP if designers need to configure per-level overrides

3. **Widget Class References Scattered**

   Current locations:

   - BP_FC_PlayerController: MainMenuWidgetClass, SaveSlotSelectorWidgetClass
   - BP_FC_GameInstance: TransitionWidgetClass

   **Inconsistency**: Some UI config in PlayerController, some in GameInstance

   **Better** (with Priority 1 refactoring):

   - ALL widget classes configured in UFCUIManager (or GameInstance)
   - PlayerController doesn't own UI references
   - Single source of truth for UI configuration

4. **Save Slot Widget Coupling**

   `WBP_SaveSlotItem → PlayerController->LoadSaveSlot(SlotName)`

   **Problem**: Widget has game logic knowledge (what to do on click)

   **Better Pattern**:

   - Widget fires event dispatcher: `OnSlotSelected(SlotName)`
   - Parent widget (WBP_SaveSlotSelector) handles the event
   - SaveSlotSelector calls UIManager or GameInstance
   - More reusable, testable

### Blueprint Refactoring Recommendations

#### Priority 1 (With C++ Refactoring)

When implementing `UFCUIManager` subsystem:

1. **Move Widget Class References**

   ```cpp
   // FROM: BP_FC_PlayerController
   MainMenuWidgetClass
   SaveSlotSelectorWidgetClass

   // TO: UFCUIManager (configured in BP_FCGameInstance or data asset)
   TMap<EUIType, TSoftClassPtr<UUserWidget>> WidgetClasses;
   ```

2. **Update Widget Callbacks**
   - WBP_MainMenu buttons should call `UIManager->OnNewLegacyClicked()` (via interface/event)
   - Or better: Dispatch events that UIManager binds to
3. **Create BPI_UICallbacks Interface** (optional but recommended)

   ```cpp
   // Blueprint Interface for UI → Game communication
   UINTERFACE()
   class UUICallbackInterface : public UInterface { ... };

   class IUICallbackInterface {
       UFUNCTION(BlueprintNativeEvent)
       void OnMenuButtonClicked(EMenuButtonType ButtonType);
   };

   // UFCUIManager implements this
   // Widgets call the interface (no direct coupling)
   ```

#### Priority 2 (Nice to Have)

1. **Consolidate BP_FC_GameMode**
   - If no Blueprint logic needed, remove asset
   - Use C++ AFCGameMode directly
2. **Data-Driven Widget Registry**

   - Create DT_UIWidgets Data Table
   - Maps UI types to widget classes
   - Easier to manage than scattered references

3. **Widget Event System**
   - Instead of widgets calling methods directly
   - Widgets fire event dispatchers
   - UIManager binds and handles centrally

### Blueprint Changes Needed for Priority 1 Refactoring

When we implement the Priority 1 C++ refactorings, these Blueprint changes are required:

**BP_FC_GameInstance**:

- Add property: `UIManagerClass` (points to UFCUIManager)
- Move `MainMenuWidgetClass`, `SaveSlotSelectorWidgetClass` here from PlayerController
- Test: Verify subsystem initializes correctly

**BP_FC_PlayerController**:

- Remove: `MainMenuWidgetClass`, `SaveSlotSelectorWidgetClass` properties
- Keep: `MenuCamera` reference (needed for camera system)
- Test: Ensure no null reference errors

**WBP_MainMenu**:

- Update button click events to call UIManager instead of PlayerController
- May need to get UIManager reference in widget construction
- Test: All buttons still work after refactoring

**WBP_SaveSlotSelector**:

- Update slot selection to go through UIManager
- Test: Save loading still functional

### Testing Strategy for Blueprint Changes

After each Blueprint modification:

1. **Open BP_FC_GameInstance in editor** - verify no compilation errors
2. **Open BP_FC_PlayerController in editor** - verify references valid
3. **Open WBP_MainMenu in editor** - verify button bindings
4. **PIE Test**:
   - Start game → Main Menu appears ✅
   - Click each button → Correct behavior ✅
   - Load save → Works correctly ✅
   - Return to menu via door → Functions properly ✅

---

## Detailed Analysis

### Issue 1: Level Tracking Responsibility Split (CRITICAL)

**Current Implementation:**

```cpp
// UFCTransitionManager.h
FName CurrentLevelName;  // Tracks current level
bool IsSameLevelLoad(const FName& TargetLevelName) const;
void UpdateCurrentLevel(const FName& NewLevelName);

// UFCGameInstance.cpp - LoadGameAsync()
FString CurrentLevelName = GetWorld()->GetMapName();
if (CurrentLevelName.StartsWith("UEDPIE_0_")) {
    CurrentLevelName = CurrentLevelName.RightChop(9); // Strip PIE prefix
}
// ...compares level names, decides transition type

// UFCGameInstance.cpp - RestorePlayerPosition()
TransitionMgr->UpdateCurrentLevel(FName(*CurrentLevelName));

// AFCPlayerController.cpp - BeginPlay()
FString CurrentLevelName = GetWorld()->GetMapName();
if (CurrentLevelName.StartsWith("UEDPIE_0_")) {
    CurrentLevelName = CurrentLevelName.RightChop(9);
}
if (!CurrentLevelName.Contains("Office") && !CurrentLevelName.Contains("MainMenu")) {
    // Set up gameplay input
}
```

**Problems:**

- Level name parsing logic duplicated in 3 places
- PIE prefix stripping happens inconsistently
- Responsibility for "what is current level" is unclear
- Hard-coded string matching ("Office", "MainMenu") will break with:
  - Multiple office variants (L_Office_Act1, L_Office_Act2)
  - Localized level names
  - Future camp/POI levels
  - Combat levels

**GDD/DRM Future Requirements:**

- Week 2: Map Table Interaction with multiple startpoints
- Week 3-4: Overworld level (L_Overworld_Region1, etc.)
- Week 5: Camp scenes (L_Camp_Mountain, L_Camp_Desert, etc.)
- Week 6: Combat levels (L_Combat_Encounter1, etc.)
- Phase 2: Multiple regions with unique naming

**Violation of Conventions:**

> **UE_CodeConventions.md § 2.1**: "Encapsulation - Private by default. Clear inheritance intent."  
> **UE_CodeConventions.md § 4.2**: "Minimal Dependencies - Reduce cross-module dependencies."

**Recommended Solution:**

Create a dedicated `UFCLevelManager` subsystem (or integrate into GameInstance):

```cpp
// Proposed: UFCLevelManager.h
UENUM(BlueprintType)
enum class EFCLevelType : uint8
{
    MainMenu,      // L_MainMenu
    Office,        // L_Office*
    Overworld,     // L_Overworld*
    Camp,          // L_Camp*
    Combat,        // L_Combat*
    POI,           // L_POI*
    Village        // L_Village*
};

UCLASS()
class UFCLevelManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Central authority for level state
    FName GetCurrentLevelName() const;
    EFCLevelType GetCurrentLevelType() const;

    // Type checking (replaces string matching)
    bool IsMenuLevel() const;
    bool IsGameplayLevel() const;
    bool IsCampLevel() const;
    bool IsCombatLevel() const;

    // Transition logic
    bool RequiresFadeTransition(const FName& TargetLevel) const;
    bool RequiresInputReconfiguration(const FName& TargetLevel) const;

    // Level lifecycle tracking
    void OnLevelLoaded(const FName& LevelName);
    void OnLevelUnloaded(const FName& LevelName);

private:
    // Map level names to types (configured in data table or config)
    UPROPERTY()
    TMap<FName, EFCLevelType> LevelTypeRegistry;

    FName CurrentLevel;
    EFCLevelType CurrentLevelType;

    // Parse level name (handles PIE prefix, etc.)
    FName NormalizeLevelName(const FName& RawLevelName) const;
    EFCLevelType DetermineLevelType(const FName& LevelName) const;
};
```

**Benefits:**

- ✅ Single source of truth for level state
- ✅ No string matching - enum-based type checking
- ✅ Easily extensible for new level types
- ✅ Data-driven via registry (can load from DataTable)
- ✅ Encapsulates all PIE/path normalization logic
- ✅ Clear API for transition decisions

**Migration Path:**

1. Create `UFCLevelManager` subsystem with minimal API
2. Migrate `CurrentLevelName` from TransitionManager to LevelManager
3. Replace all `GetWorld()->GetMapName()` calls with `LevelManager->GetCurrentLevelName()`
4. Replace string matching with type checks
5. Test each step in PIE mode
6. Remove duplicate logic from GameInstance/PlayerController/TransitionManager

---

### Issue 2: PlayerController God Object (CRITICAL)

**Current Responsibilities:**

```cpp
// AFCPlayerController (223 lines, 40+ methods)
class AFCPlayerController : public APlayerController
{
    // 1. Input Management (Enhanced Input)
    void SetupInputComponent();
    void SetInputMappingMode(EFCInputMappingMode NewMode);
    void HandleInteractPressed();
    void HandlePausePressed();
    void HandleQuickSavePressed();
    void HandleQuickLoadPressed();

    // 2. Camera Management
    void SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime);
    EFCPlayerCameraMode GetCameraMode() const;
    TObjectPtr<ACameraActor> MenuCamera;

    // 3. Game State Management
    void InitializeMainMenu();
    void TransitionToGameplay();
    void ReturnToMainMenu();
    EFCGameState GetCurrentGameState() const;

    // 4. UI Management
    void OnNewLegacyClicked();
    void OnContinueClicked();
    void OnLoadSaveClicked();
    void OnOptionsClicked();
    void OnQuitClicked();
    void CloseSaveSlotSelector();
    void LoadSaveSlot(const FString& SlotName);
    TObjectPtr<UUserWidget> MainMenuWidget;
    TObjectPtr<UUserWidget> SaveSlotSelectorWidget;

    // 5. Save/Load Integration
    void DevQuickSave();
    void DevQuickLoad();
    void RestorePlayerPositionDeferred();
    void OnSaveGameLoaded(bool bSuccess);

    // 6. Transition Integration
    void FadeScreenOut(float Duration, bool bShowLoading);
    void FadeScreenIn(float Duration);
    void RestoreInputAfterBlend();

    // 7. Level Detection Logic
    // BeginPlay() has level name parsing and input setup
};
```

**Problems:**

- Violates Single Responsibility Principle (SRP)
- Massive class will become unmaintainable by Week 6 (combat, crew UI, etc.)
- UI logic in controller makes testing difficult
- Hard to extend for future perspectives (TopDown, Combat)
- Blueprint exposure is chaotic - 30+ callable functions

**GDD/DRM Future Requirements:**

- Week 2: Map Table UI with route planning
- Week 4: Overworld Map Widget with fog-of-war
- Week 5: Camp management UI
- Week 6: Combat UI (turn order, abilities, grid)
- Week 7: Time/Resource HUD
- Week 12: Camp task assignment UI
- Week 21: Reputation/Finance UI

Each new UI system would add 5-10 more methods to PlayerController!

**Violation of Conventions:**

> **UE_CodeConventions.md § 2.2**: "Modular Code Organization - Example modules: Combat, UI, Inventory, AI, etc."  
> **UE_CodeConventions.md § 4.3**: "Clean APIs - Design small, clear public interfaces."

**Recommended Solution:**

Split into specialized components/subsystems:

```cpp
// Proposed Architecture:

// 1. UFCUIManager - Game Instance Subsystem
class UFCUIManager : public UGameInstanceSubsystem
{
    // Owns all UI widget references
    // Manages UI stack (Main Menu → Save Selector → Options, etc.)
    // Handles UI transitions
    void ShowMainMenu();
    void ShowSaveSlotSelector();
    void ShowPauseMenu();
    void HideAllMenus();

    // UI state tracking
    TArray<UUserWidget*> UIStack;
    bool IsUIActive() const;
};

// 2. UFCCameraManager - Player State Component
class UFCCameraManager : public UActorComponent
{
    // Attached to PlayerController or Player State
    // Manages camera transitions
    void SetViewTarget(AActor* NewTarget, float BlendTime);
    void BlendToMenuCamera();
    void BlendToFirstPerson();
    void BlendToTopDown();

    ACameraActor* MenuCamera;
    UCameraComponent* FirstPersonCamera;
    // Future: TopDownCamera, CombatCamera
};

// 3. UFCInputManager - Player Controller Component
class UFCInputManager : public UActorComponent
{
    // Handles input context switching
    void ApplyInputContext(EFCInputMappingMode Mode);
    void SaveCurrentContext();
    void RestoreContext();

    // Bind/unbind actions
    void BindGameplayActions();
    void UnbindAll();
};

// 4. Simplified PlayerController
class AFCPlayerController : public APlayerController
{
    // ONLY handles low-level input routing
    virtual void SetupInputComponent() override;

    // Delegates to components
    UPROPERTY()
    UFCInputManager* InputManager;

    UPROPERTY()
    UFCCameraManager* CameraManager;

    // Input handlers just route to appropriate systems
    void HandleInteractPressed(); // → Character's InteractionComponent
    void HandlePausePressed();    // → UIManager->ShowPauseMenu()
};
```

**Benefits:**

- ✅ Each class has single, clear responsibility
- ✅ Easy to test in isolation
- ✅ Blueprint API is focused per system
- ✅ Components can be reused (e.g., TopDownPlayerController shares InputManager)
- ✅ Scales cleanly with new features

**Migration Path:**

1. Create `UFCUIManager` subsystem with MainMenu/SaveSlot methods
2. Move widget references from PlayerController to UIManager
3. Update button callbacks to call UIManager instead of PC directly
4. Create `UFCCameraManager` component
5. Move camera references and blend logic
6. Test in PIE after each step
7. Create `UFCInputManager` component (optional, can defer)

---

### Issue 3: Widget Lifecycle Architecture (CRITICAL for Task 5.14)

**Current Problem:**

Task 5.14 (deferred) requires persistent fade-in on all level loads, but:

```cpp
// TransitionWidget owned by PlayerController (destroyed on level change)
// UFCTransitionManager.cpp - CreateTransitionWidget()
APlayerController* PlayerController = World->GetFirstPlayerController();
TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(PlayerController, WidgetClass);
```

**Why This Fails:**

1. `OpenLevel` destroys current PlayerController
2. Widget's outer is destroyed → Widget garbage collected
3. New level loads → new PlayerController → new widget
4. Screen is already visible before new widget can initialize black

**Future Requirements:**

- Task 5.14: Persistent fade-in on all loads
- Week 7+: Persistent HUD showing time/resources (must survive level transitions)
- Week 8: Expedition report overlay (shows after combat → camp → office transition)
- Week 21: Reputation/Finance persistent UI elements

**Recommended Solution:**

Use GameInstance as widget owner + HUD layer architecture:

```cpp
// Proposed: UFCHUDManager - Game Instance Subsystem
class UFCHUDManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Persistent overlay management
    void CreatePersistentOverlay();
    void DestroyPersistentOverlay();

    // Add widgets to persistent layer (survives level transitions)
    void AddPersistentWidget(UUserWidget* Widget, int32 ZOrder);
    void RemovePersistentWidget(UUserWidget* Widget);

    // Query persistent state
    bool HasPersistentOverlay() const;
    UUserWidget* GetPersistentOverlay() const { return PersistentOverlay; }

private:
    // Root widget parented to GameViewportClient (not PlayerController!)
    UPROPERTY()
    UUserWidget* PersistentOverlay;

    // Widgets added to persistent layer
    UPROPERTY()
    TArray<UUserWidget*> PersistentWidgets;
};

// Modified: UFCTransitionManager
class UFCTransitionManager : public UGameInstanceSubsystem
{
    virtual void Initialize(FSubsystemCollectionBase& Collection) override
    {
        Super::Initialize(Collection);

        // Get HUD manager
        UFCHUDManager* HUDMgr = GetGameInstance()->GetSubsystem<UFCHUDManager>();
        if (HUDMgr)
        {
            HUDMgr->CreatePersistentOverlay();
            CreateTransitionWidget(); // Will now parent to persistent overlay
            HUDMgr->AddPersistentWidget(TransitionWidget, 1000);
        }
    }

    void CreateTransitionWidget()
    {
        // Create widget with GameInstance as outer (survives level loads!)
        TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(
            GetGameInstance(),
            WidgetClass
        );

        // Add to viewport via GameViewportClient (not PlayerController)
        if (UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient())
        {
            ViewportClient->AddViewportWidgetContent(TransitionWidget->TakeWidget(), 1000);
        }
    }
};
```

**Alternative (Simpler):**

Use `UGameViewportClient::AddViewportWidgetContent()` directly:

```cpp
// UFCTransitionManager::CreateTransitionWidget()
void UFCTransitionManager::CreateTransitionWidget()
{
    UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
    if (!ViewportClient)
    {
        UE_LOG(LogFCTransitions, Error, TEXT("No GameViewportClient available"));
        return;
    }

    // Create with GameInstance as outer
    TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(GetGameInstance(), WidgetClass);
    if (!TransitionWidget)
    {
        return;
    }

    // Add directly to viewport (bypasses PlayerController ownership)
    ViewportClient->AddViewportWidgetContent(
        TransitionWidget->TakeWidget(),
        1000 // High Z-order
    );

    // Widget now persists across level transitions!
}
```

**Benefits:**

- ✅ Widget survives level transitions
- ✅ Can start black before new level loads
- ✅ Enables Task 5.14 implementation
- ✅ Foundation for persistent HUD elements
- ✅ Follows Epic's recommended pattern for persistent UI

**Migration Path:**

1. Test current widget with `GetGameInstance()` as outer (verify no crashes)
2. Switch to `AddViewportWidgetContent()` instead of `AddToViewport()`
3. Verify widget persists across `OpenLevel` call
4. Implement fade-in initialization (start black on create)
5. Test all transition scenarios

---

### Issue 4: Hard-coded Level Name Dependencies (HIGH PRIORITY)

**Current Implementation:**

```cpp
// AFCPlayerController::BeginPlay()
if (!CurrentLevelName.Contains("Office") && !CurrentLevelName.Contains("MainMenu")) {
    // Set up gameplay
}

// Multiple places use string matching for level detection
```

**Problems:**

- Brittle - breaks with new level names
- Not localization-friendly
- No support for level variants (L_Office_Act1, L_Office_Tutorial, etc.)
- Hard to maintain as project grows

**Future Requirements:**

- DRM Week 2+: Multiple office variants for different acts
- DRM Week 25: Demo region with curated content (custom level names)
- DRM Phase 3: Region 2 with different biomes/themes

**Recommended Solution:**

Data-driven level metadata system:

```cpp
// Proposed: DT_LevelMetadata (Data Table)
USTRUCT(BlueprintType)
struct FFCLevelMetadata : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFCLevelType LevelType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFCInputMappingMode DefaultInputMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRequiresFadeTransition;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bShowCursor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName; // For UI/save slots
};

// Usage: Query metadata instead of string matching
UFCLevelManager* LevelMgr = GetGameInstance()->GetSubsystem<UFCLevelManager>();
const FFCLevelMetadata* Metadata = LevelMgr->GetLevelMetadata(CurrentLevelName);
if (Metadata && Metadata->DefaultInputMode == EFCInputMappingMode::FirstPerson) {
    SetupGameplayInput();
}
```

**Benefits:**

- ✅ All level configuration in one place (Data Table)
- ✅ Non-programmers can add/configure levels
- ✅ Supports variants without code changes
- ✅ Localization-ready (DisplayName)

---

### Issue 5: Missing State Machine (MEDIUM-HIGH PRIORITY)

**Current State Management:**

```cpp
// State is implicit, scattered across multiple classes

// AFCPlayerController
EFCGameState CurrentGameState; // MainMenu, Gameplay, TableView, Paused, Loading
EFCPlayerCameraMode CameraMode; // FirstPerson, TableView, MainMenu, SaveSlotView
EFCInputMappingMode CurrentMappingMode; // FirstPerson, TopDown, Fight, StaticScene

// State transitions happen via direct function calls
void InitializeMainMenu();
void TransitionToGameplay();
void ReturnToMainMenu();

// No validation - can get into invalid states
// No history - can't "go back" reliably
// No explicit state graph
```

**Problems:**

- State transitions are implicit (function calls)
- No validation of valid state changes
- Can't visualize state flow
- Hard to debug "how did I get here?" issues
- No support for state stack (e.g., Pause while in TableView)

**Future Complexity:**

By Week 8, we'll have:

- MainMenu → Office → TableView → Overworld → Camp → Combat
- Each with substates (Combat: PlayerTurn, EnemyTurn, Victory, Defeat)
- Nested states (Camp → TaskAssignment, Camp → Rest, Camp → Inventory)
- State history for "back" navigation

**Recommended Solution:**

Explicit state machine (can be simple enum-based at first):

```cpp
// Proposed: UFCGameStateManager
UENUM(BlueprintType)
enum class EFCGameStateID : uint8
{
    None,
    MainMenu,
    Office_Exploration,
    Office_TableView,
    Overworld_Travel,
    Camp_Management,
    Combat_PlayerTurn,
    Combat_EnemyTurn,
    Paused,
    Loading
};

UCLASS()
class UFCGameStateManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // State queries
    EFCGameStateID GetCurrentState() const;
    EFCGameStateID GetPreviousState() const;
    bool IsInState(EFCGameStateID StateID) const;

    // State transitions (with validation)
    bool TransitionTo(EFCGameStateID NewState);
    bool CanTransitionTo(EFCGameStateID NewState) const;

    // State stack (for pause/modal/nested states)
    void PushState(EFCGameStateID NewState);
    void PopState();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, EFCGameStateID, OldState, EFCGameStateID, NewState);
    UPROPERTY(BlueprintAssignable)
    FOnStateChanged OnStateChanged;

private:
    EFCGameStateID CurrentState;
    TArray<EFCGameStateID> StateHistory;
    TArray<EFCGameStateID> StateStack;

    // Validation logic
    TMap<EFCGameStateID, TArray<EFCGameStateID>> ValidTransitions;
    void InitializeValidTransitions();
};
```

**Benefits:**

- ✅ Explicit state graph (can visualize)
- ✅ Validation prevents invalid transitions
- ✅ State history enables "back" navigation
- ✅ Events allow UI/systems to react to state changes
- ✅ Easy to debug (log all transitions)

**Migration Path:**

1. Create simple state manager with current states
2. Migrate `CurrentGameState` from PlayerController to StateManager
3. Replace direct function calls with `StateManager->TransitionTo()`
4. Add validation incrementally
5. Expand states as new features added

---

## Testing Strategy

### Comprehensive Test Suite (After Each Refactoring)

Run this full test suite after completing each refactoring to ensure no regressions:

**1. Main Menu Flow**:

- ✅ PIE starts → MenuCamera active, WBP_MainMenu visible
- ✅ Mouse cursor visible, can click buttons
- ✅ Atmospheric effects working (rain, dust, candles)

**2. New Legacy Flow**:

- ✅ Click "New Legacy" → 2s camera blend to FirstPerson
- ✅ Menu disappears, input switches to gameplay
- ✅ Can move (WASD) and look (mouse)
- ✅ No crashes or black screens

**3. Save System**:

- ✅ F6 QuickSave → Save confirmation in logs
- ✅ Walk to different position
- ✅ F9 QuickLoad → Loads to saved position

**4. Return to Menu**:

- ✅ Walk to door, press E → "Return to Menu" prompt
- ✅ Press E → Fade to black (1s)
- ✅ Level reloads → Back at main menu
- ✅ Menu camera and UI correct

**5. Continue Flow**:

- ✅ Click "Continue" → Loads most recent save
- ✅ Correct transition (same-level: blend, cross-level: fade)
- ✅ Player at correct position/rotation
- ✅ Full controls working

**6. Load Save Flow**:

- ✅ Click "Load Save" → WBP_SaveSlotSelector appears
- ✅ Save slots populated with correct data
- ✅ Click slot → Loading works
- ✅ Click "Back" → Returns to main menu

**7. Cross-Level Test** (if test level available):

- ✅ Save in Office
- ✅ Manually load different level
- ✅ Click Continue → Fade out with spinner
- ✅ Level loads → Fade in reveals new location
- ✅ Full controls working in new level

**8. Edge Cases**:

- ✅ Click Continue with no saves → Button disabled or message shown
- ✅ Multiple quick saves → Most recent loads
- ✅ Rapid button clicks → No duplicate widgets or crashes

---

## Risks if We Don't Refactor

### Priority 1: Core Architecture (Before Week 2) - **C++ + Blueprint**

Each refactoring includes both C++ implementation and corresponding Blueprint updates.

#### Refactoring 1A: UFCLevelManager Subsystem

**C++ Changes**:

- Create `UFCLevelManager` subsystem
- Move `CurrentLevelName` from TransitionManager
- Add enum-based level type system
- Replace string matching with type checks

**Blueprint Changes**:

- **BP_FC_GameInstance**: No changes needed (subsystem auto-initialized)
- **Test**: Level detection works in PIE

**Impact**: Eliminates level tracking duplication across 3 C++ classes

---

#### Refactoring 1B: UFCUIManager Subsystem + Blueprint Decoupling

**C++ Changes**:

- Create `UFCUIManager` subsystem
- Move all UI management from PlayerController
- Centralize widget lifecycle (creation, stacking, transitions)
- Add clean public API for UI operations

**Blueprint Changes**:

- **BP_FC_GameInstance**:
  - Add `UIManagerClass` property
  - Move `MainMenuWidgetClass` from PlayerController
  - Move `SaveSlotSelectorWidgetClass` from PlayerController
- **BP_FC_PlayerController**:
  - Remove `MainMenuWidgetClass` property
  - Remove `SaveSlotSelectorWidgetClass` property
  - Keep `MenuCamera` reference (still needed for camera system)
- **WBP_MainMenu**:
  - Update button click events:
    - FROM: `Get Player Controller → OnNewLegacyClicked()`
    - TO: `Get Game Instance → Get Subsystem (UIManager) → OnNewLegacyClicked()`
  - Same for all buttons (Continue, Load, Options, Quit)
- **WBP_SaveSlotSelector**:
  - Update slot selection callback:
    - FROM: `Get Player Controller → LoadSaveSlot()`
    - TO: `Get Game Instance → Get Subsystem (UIManager) → LoadSaveSlot()`
- **WBP_SaveSlotItem**:
  - Change OnClicked event to fire event dispatcher instead of direct call
  - Parent widget (WBP_SaveSlotSelector) handles the event

**Impact**:

- Reduces PlayerController from 223 lines to ~80 lines
- Fixes widget coupling issues
- All UI configuration in one place
- Easier to test and maintain

---

#### Refactoring 1C: Widget Lifecycle Fix

**C++ Changes**:

- Modify `UFCTransitionManager::CreateTransitionWidget()`
- Use `GetGameInstance()` as widget outer (not PlayerController)
- Use `UGameViewportClient::AddViewportWidgetContent()` instead of `AddToViewport()`

**Blueprint Changes**:

- **WBP_ScreenTransition**: No changes needed (C++ handles lifecycle)
- **Test**: Widget persists across level loads, can start black on create

**Impact**:

- Unblocks Task 5.14 (persistent fade-in)
- Foundation for persistent HUD (Week 7+)
- Widget survives `OpenLevel` calls

---

### Priority 2: Data-Driven Systems (Before Week 4) - **C++ + Blueprint**

#### Refactoring 2A: Level Metadata System

**C++ Changes**:

- Create `FFCLevelMetadata` struct
- Add metadata query API to UFCLevelManager

**Blueprint Changes**:

- **New Asset**: `DT_LevelMetadata` Data Table
  - Add rows for L_Office, L_TopDown, future levels
  - Configure: LevelType, DefaultInputMode, bRequiresFadeTransition, DisplayName
- **BP_FC_GameInstance**: Configure `LevelMetadataTable` reference

**Impact**:

- Eliminates all hard-coded level names
- Designers can add/configure levels without C++ changes

---

#### Refactoring 2B: State Machine

**C++ Changes**:

- Create `UFCGameStateManager` subsystem
- Define state enum and valid transitions
- Add state stack for pause/modal states

**Blueprint Changes**:

- **BP_FC_PlayerController**: Remove `CurrentGameState` property (now in StateManager)
- **Update all state transitions**: Use StateManager API
- **Optional**: Create `BP_FCGameStateDebugWidget` to visualize state in PIE

**Impact**:

- Explicit state validation
- Easy to add combat/camp states
- Better debugging

---

### Priority 3: Polish & Components (After Week 4) - **C++ + Blueprint**

#### Refactoring 3A: CameraManager Component

**C++ Changes**:

- Create `UFCCameraManager` component
- Move camera blending logic from PlayerController

**Blueprint Changes**:

- **BP_FC_PlayerController**: Add CameraManager component
- **BP_MenuCamera**: No changes (still referenced by component)

---

## Integrated Testing Strategy (C++ + Blueprint)

### Refactoring 1A: Level Manager (2-3 hours)

**Implementation Steps**:

1. Create `UFCLevelManager.h/.cpp` with basic API
2. Compile and verify no errors
3. Migrate `CurrentLevelName` from TransitionManager
4. Update TransitionManager to use LevelManager
5. Replace string matching in PlayerController BeginPlay
6. Update GameInstance LoadGameAsync
7. Compile and test

**Blueprint Updates**:

- None required (subsystem auto-initializes)

**PIE Test Checklist**:

- ✅ Start game → Main Menu appears (level detection works)
- ✅ Click "New Legacy" → Transition to gameplay
- ✅ Check logs for level type detection (should show enum, not strings)
- ✅ F6 QuickSave
- ✅ Return to menu via door
- ✅ Click "Continue" → Loads correctly
- ✅ Cross-level test (if test level available)

---

### Refactoring 1B: UI Manager + Blueprint Decoupling (4-6 hours)

**Implementation Steps (Interleaved C++ + BP)**:

#### Step 1: Create UFCUIManager Skeleton (C++)

- Create `UFCUIManager.h/.cpp` with basic structure
- Add widget class properties (MainMenu, SaveSelector)
- Add stub methods: `ShowMainMenu()`, `ShowSaveSlotSelector()`, `LoadSaveSlot()`
- Compile and verify

#### Step 2: Configure BP_FC_GameInstance (Blueprint)

- Open `BP_FC_GameInstance` in editor
- Add property: `UIManagerClass` (set to UFCUIManager)
- Add property: `MainMenuWidgetClass` (move from PlayerController)
- Add property: `SaveSlotSelectorWidgetClass` (move from PlayerController)
- Save and compile Blueprint

#### Step 3: Update BP_FC_PlayerController (Blueprint)

- Open `BP_FC_PlayerController` in editor
- Delete: `MainMenuWidgetClass` variable
- Delete: `SaveSlotSelectorWidgetClass` variable
- Keep: `MenuCamera` variable
- Save and compile Blueprint
- **Expect warnings** about broken references in widgets (we'll fix next)

#### Step 4: Migrate PlayerController UI Methods (C++)

- Move `OnNewLegacyClicked()` from PlayerController to UIManager
- Move `OnContinueClicked()` from PlayerController to UIManager
- Move all button callback methods
- Move widget management code
- Update references to use `GetGameInstance()->GetSubsystem<UFCUIManager>()`
- Compile

#### Step 5: Update WBP_MainMenu (Blueprint)

- Open `WBP_MainMenu` in UMG editor
- For each button (New Legacy, Continue, Load, Options, Quit):
  - Select button
  - Open OnClicked event graph
  - **Replace**:
    ```
    Get Player Controller → Cast to FC_PlayerController → OnButtonClicked()
    ```
  - **With**:
    ```
    Get Game Instance → Get Subsystem (UIManager) → OnButtonClicked()
    ```
- Save and compile widget

#### Step 6: Update WBP_SaveSlotSelector (Blueprint)

- Open `WBP_SaveSlotSelector` in UMG editor
- Find slot selection logic
- **Replace**:
  ```
  Get Player Controller → LoadSaveSlot()
  ```
- **With**:
  ```
  Get Game Instance → Get Subsystem (UIManager) → LoadSaveSlot()
  ```
- Save and compile

#### Step 7: Update WBP_SaveSlotItem (Blueprint)

- Open `WBP_SaveSlotItem` in UMG editor
- **Option A** (Simple - keep for now):
  - Update OnClicked to call UIManager directly
- **Option B** (Better - implement later):
  - Change OnClicked to fire event dispatcher
  - Parent widget binds and handles

#### Step 8: Final PlayerController Cleanup (C++)

- Remove unused includes
- Remove widget properties
- Keep only essential player control logic
- Compile

**Full PIE Test Checklist**:

- ✅ Start game → Main Menu appears correctly
- ✅ Click "New Legacy" → Gameplay transition works
- ✅ Click "Continue" → Load system works
- ✅ Click "Load Save" → Save selector appears
- ✅ Select save slot → Loading works
- ✅ Click "Options" → (Placeholder behavior correct)
- ✅ Click "Quit" → Game quits or returns to editor
- ✅ F6 QuickSave → Still functional
- ✅ F9 QuickLoad → Still functional
- ✅ Door interaction → Return to menu works
- ✅ All transitions smooth (no crashes, no black screens)
- ✅ Check logs → UIManager methods being called, not PlayerController

**Rollback Plan**:

- If issues occur, revert Blueprint changes first
- Verify C++ compiles standalone
- Fix Blueprint bindings individually
- Test after each widget update

---

### Refactoring 1C: Widget Lifecycle (2-3 hours)

**Implementation Steps**:

#### Step 1: Backup Current Implementation

- Commit current state: `git commit -m "checkpoint: before widget lifecycle refactor"`

#### Step 2: Modify TransitionManager (C++)

- Update `CreateTransitionWidget()`:
  - Change outer from `PlayerController` to `GetGameInstance()`
  - Replace `AddToViewport()` with `UGameViewportClient::AddViewportWidgetContent()`
- Compile and verify

#### Step 3: Test Widget Persistence (Blueprint)

- **No Blueprint changes needed**
- PIE Test:
  - ✅ Start game → Transition widget exists
  - ✅ Trigger cross-level load
  - ✅ Check if widget survives (use `VisualizeWidget` command or log)
  - ✅ Verify widget can start in black state

#### Step 4: Implement Task 5.14 Fade-In (C++)

- Initialize widget to black on creation
- Add fade-in trigger in PlayerController BeginPlay
- Test timing and visual result

**PIE Test Checklist**:

- ✅ PIE starts → Screen fades in from black (not instant)
- ✅ Cross-level load → Widget persists, stays black, fades in
- ✅ Same-level load → Existing camera blend (no change)
- ✅ No flicker or pop-in
- ✅ All previous transitions still work

---

### Refactoring 2A: Level Metadata System (2-3 hours)

**Implementation Steps**:

#### Step 1: Create Metadata Struct (C++)

- Create `FFCLevelMetadata` struct in LevelManager
- Add properties: LevelType, InputMode, bRequiresFade, DisplayName
- Compile

#### Step 2: Create Data Table (Blueprint)

- Right-click Content Browser → Miscellaneous → Data Table
- Select `FFCLevelMetadata` as row structure
- Name: `DT_LevelMetadata`
- Add rows:
  - L_Office: LevelType=Office, InputMode=FirstPerson, etc.
  - L_TopDown: LevelType=Overworld, InputMode=TopDown, etc.
- Save

#### Step 3: Update LevelManager (C++)

- Add `TObjectPtr<UDataTable> LevelMetadataTable` property
- Add `GetLevelMetadata(FName LevelName)` query method
- Replace type detection with table lookup
- Compile

#### Step 4: Configure GameInstance (Blueprint)

- Open `BP_FC_GameInstance`
- Set `LevelMetadataTable` to `DT_LevelMetadata`
- Save

#### Step 5: Replace String Matching (C++)

- Find all `CurrentLevelName.Contains("Office")` calls
- Replace with `LevelMgr->GetLevelType() == EFCLevelType::Office`
- Compile and test

**PIE Test Checklist**:

- ✅ All level detection still works
- ✅ Can add new level to data table without C++ changes
- ✅ DisplayName appears correctly in save slots

---

### Refactoring 2B: State Machine (3-4 hours)

**Implementation Steps**:

#### Step 1: Create StateManager (C++)

- Create `UFCGameStateManager` subsystem
- Define `EFCGameStateID` enum
- Add transition validation logic
- Compile

#### Step 2: Remove Old State Property (Blueprint)

- Open `BP_FC_PlayerController`
- Note: `CurrentGameState` will be removed from C++ base class
- No action needed in BP (will be handled by C++)

#### Step 3: Migrate State Transitions (C++)

- Replace `CurrentGameState = X` with `StateManager->TransitionTo(X)`
- Add validation and logging
- Compile

#### Step 4: Optional Debug Widget (Blueprint)

- Create `WBP_StateDebugWidget`
- Shows current state, history, valid transitions
- Add to viewport in PIE for debugging

**PIE Test Checklist**:

- ✅ All state transitions work
- ✅ Invalid transitions blocked (check logs)
- ✅ State history tracked correctly
- ✅ Debug widget shows correct state

---

## Complete Refactoring Timeline

### Day 1 (6-8 hours)

- **Morning**: Refactoring 1A (LevelManager) - 2-3h
- **Afternoon**: Refactoring 1B Part 1 (UIManager C++ + BP Config) - 2-3h
- **Evening**: Refactoring 1B Part 2 (Widget Updates) - 2-3h

### Day 2 (4-6 hours)

- **Morning**: Refactoring 1B Part 3 (Testing + Fixes) - 2h
- **Afternoon**: Refactoring 1C (Widget Lifecycle + Task 5.14) - 2-3h

### Day 3 (Optional - Priority 2)

- **Morning**: Refactoring 2A (Level Metadata) - 2-3h
- **Afternoon**: Refactoring 2B (State Machine) - 3-4h

**Total for Priority 1**: 10-14 hours (1.5-2 days)  
**Total for Priority 2**: 5-7 hours (1 day)

---

## Per-Refactoring Commit Strategy

After each refactoring is complete and tested:

```bash
# Refactoring 1A
git add -A
git commit -m "refactor: create UFCLevelManager subsystem

- Centralize level tracking from TransitionManager, GameInstance, PlayerController
- Replace string matching with enum-based level types
- Add IsSameLevelLoad and GetLevelType APIs
- Update all consumers to use LevelManager
- No Blueprint changes required

Eliminates duplication, prepares for data-driven level config"

# Refactoring 1B
git add -A
git commit -m "refactor: extract UFCUIManager subsystem

C++ Changes:
- Create UFCUIManager subsystem for all UI management
- Move 30+ methods from AFCPlayerController to UIManager
- Reduce PlayerController from 223 to ~80 lines
- Centralize widget lifecycle and UI state

Blueprint Changes:
- BP_FC_GameInstance: Add UIManagerClass, widget class properties
- BP_FC_PlayerController: Remove MainMenuWidgetClass, SaveSlotSelectorWidgetClass
- WBP_MainMenu: Update button callbacks to call UIManager
- WBP_SaveSlotSelector: Update slot selection to use UIManager
- WBP_SaveSlotItem: Update OnClicked routing

Fixes widget coupling, enables clean UI expansion"

# Refactoring 1C
git add -A
git commit -m "refactor: fix widget lifecycle for persistence

- Parent TransitionWidget to GameInstance (not PlayerController)
- Use UGameViewportClient::AddViewportWidgetContent for persistence
- Widget survives OpenLevel calls and level transitions
- Implement Task 5.14: persistent fade-in on all level loads

Unblocks persistent HUD and smooth level transitions"

# Refactoring 2A
git add -A
git commit -m "refactor: implement data-driven level metadata

C++ Changes:
- Create FFCLevelMetadata struct with level configuration
- Add metadata query API to UFCLevelManager

Blueprint Changes:
- Create DT_LevelMetadata Data Table with level configs
- BP_FC_GameInstance: Configure LevelMetadataTable reference

Eliminates hard-coded level names, designers can add levels without C++"

# Refactoring 2B
git add -A
git commit -m "refactor: implement explicit game state machine

- Create UFCGameStateManager subsystem
- Define EFCGameStateID enum with valid transitions
- Add state stack for pause/modal states
- Migrate CurrentGameState from PlayerController to StateManager
- Add validation and logging for all state changes

Prevents invalid states, easier debugging, scales for combat/camp"
```

---

## Proposed Implementation Order

**Recommend: Complete Priority 1 refactorings (1A, 1B, 1C) before starting Week 2 tasks**

This will:

- ✅ Unblock Task 5.14 (persistent fade-in)
- ✅ Create solid foundation for Week 2+ features
- ✅ Prevent PlayerController from becoming unmaintainable
- ✅ Fix Blueprint coupling issues now (easier than later)
- ✅ Align with UE coding conventions
- ✅ Make future development faster and safer

**Estimated Impact**:

- 1.5-2 days of refactoring now
- Saves 1-2 weeks of technical debt later (Week 6-8)
- Reduces bug risk significantly
- Easier onboarding for future developers

---

## Testing Strategy

1. **Create feature branch** (e.g., `refactor/level-manager`)
2. **Implement incrementally** - Small, testable steps
3. **Test in PIE after each step**:
   - Start game → Main Menu appears
   - Click "New Legacy" → Transition to gameplay
   - Walk around office
   - QuickSave (F6)
   - Return to menu via door
   - Click "Continue" → Loads save correctly
   - Cross-level load test (if available)
4. **Verify no regressions** before merging
5. **Update tech documentation** with new architecture

---

## Proposed Implementation Order

### Step 1: UFCLevelManager (2-3 hours)

- Create subsystem with basic API
- Migrate CurrentLevelName from TransitionManager
- Replace string matching in PlayerController BeginPlay
- Test all transitions

### Step 2: UIManager Subsystem (4-5 hours)

- Create UFCUIManager
- Move MainMenuWidget/SaveSlotSelector to UIManager
- Move button callback methods
- Update PlayerController to delegate to UIManager
- Test all UI flows

### Step 3: Widget Lifecycle Fix (2-3 hours)

- Modify TransitionManager::CreateTransitionWidget()
- Use GameInstance as outer + AddViewportWidgetContent
- Test widget persistence across level loads
- Implement Task 5.14 fade-in feature

### Step 4: Level Metadata (2-3 hours)

- Create FFCLevelMetadata struct
- Create DT_LevelMetadata Data Table
- Add entries for L_Office, L_TopDown, etc.
- Update LevelManager to query metadata
- Replace remaining string matching

### Step 5: State Machine (3-4 hours)

- Create UFCGameStateManager
- Define states and valid transitions
- Migrate CurrentGameState
- Update all state changes to use StateManager

**Total Estimated Time: 14-18 hours (2-3 days)**

---

## Risks if We Don't Refactor

1. **Technical Debt Spiral**: By Week 4-5, PlayerController will be 500+ lines and unmaintainable
2. **Feature Blocks**: Can't implement Task 5.14 (fade-in), persistent HUD without widget fix
3. **Scaling Issues**: Adding new levels/states becomes exponentially harder
4. **Team Friction**: Hard to work in parallel on UI features when everything is in PlayerController
5. **Bug Multiplication**: Current architecture makes it easy to introduce state bugs

---

## Recommendation

**Proceed with Priority 1 refactorings (Steps 1-3) before starting Week 2 tasks.**

This will:

- ✅ Unblock Task 5.14 (fade-in)
- ✅ Create foundation for Week 2+ features
- ✅ Prevent PlayerController from becoming unmaintainable
- ✅ Align with project coding conventions
- ✅ Make future development faster and safer

**Estimated Impact:**

- 2-3 days of refactoring now
- Saves 1-2 weeks of refactoring pain later (Week 6-8)
- Reduces bug risk significantly
- Makes onboarding new developers easier

---

## Next Steps

1. **Review this analysis** with team/stakeholders
2. **Prioritize** which refactorings to do now vs. later
3. **Create feature branches** for approved refactorings
4. **Test rigorously** - NO REGRESSIONS allowed
5. **Update documentation** after each refactoring

---

**Document Status**: CONCEPT - AWAITING APPROVAL  
**Last Updated**: November 16, 2025
