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

---

### Priority 1 – Core Architecture (Before Week 2)

#### Refactoring 1A – UFCLevelManager Subsystem (6-8 hours)

##### Step 1A.1: Create UFCLevelManager Skeleton

- [x] **Implementation**
  - [x] Create `Source/FC/Core/FCLevelManager.h`
  - [x] Create `Source/FC/Core/FCLevelManager.cpp`
  - [x] Inherit from `UGameInstanceSubsystem`
  - [x] Add `UPROPERTY() FName CurrentLevelName`
  - [x] Add `FName GetCurrentLevelName() const { return CurrentLevelName; }`
  - [x] Add `void UpdateCurrentLevel(const FName& NewLevelName)`
  - [x] Add `FName NormalizeLevelName(const FName& RawLevelName) const` (stub, returns input)
  - [x] Compile successfully
- [x] **Testing After Step 1A.1**
  - [x] PIE starts successfully
  - [x] No compilation errors
  - [x] Check logs: UFCLevelManager should not appear yet (not integrated)

##### Step 1A.2: Implement Level Type Detection

- [x] **Implementation**
  - [x] Add `EFCLevelType` enum (Unknown, MainMenu, Office, Overworld, Camp, Combat, POI, Village)
  - [x] Add `UPROPERTY() EFCLevelType CurrentLevelType`
  - [x] Implement `NormalizeLevelName()` - strip PIE prefix (UEDPIE*N*), trim whitespace
  - [x] Implement `EFCLevelType DetermineLevelType(const FName& LevelName) const`
    - Check exact "L_MainMenu" → MainMenu
    - Check Contains("MainMenu") → MainMenu
    - Check Contains("Office") → Office
    - Check Contains("Overworld") → Overworld
    - Check Contains("Camp") → Camp
    - Check Contains("Combat") → Combat
    - Check Contains("POI") → POI
    - Check Contains("Village") → Village
    - Default → Unknown
  - [x] Add `bool IsMenuLevel() const { return CurrentLevelType == EFCLevelType::MainMenu; }`
  - [x] Add `bool IsGameplayLevel() const` (returns true for Office, Overworld, Camp, POI, Village)
  - [x] Add `EFCLevelType GetCurrentLevelType() const { return CurrentLevelType; }`
  - [x] Compile successfully
- [x] **Testing After Step 1A.2**
  - [x] PIE starts successfully
  - [x] Main menu appears
  - [x] "New Legacy" button works
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" button loads save
  - [x] No crashes or errors in logs

##### Step 1A.3: Initialize LevelManager on Game Start

- [x] **Implementation**
  - [x] Override `void Initialize(FSubsystemCollectionBase& Collection)`
  - [x] Add `DECLARE_LOG_CATEGORY_EXTERN(LogFCLevelManager, Log, All);` to header
  - [x] Add `DEFINE_LOG_CATEGORY(LogFCLevelManager);` to .cpp
  - [x] In Initialize():
    - Get `UWorld* World = GetWorld()`
    - Safety check: `if (!World || !World->IsGameWorld()) return;`
    - Get raw level name: `FString RawMapName = World->GetMapName()`
    - Normalize: `CurrentLevelName = NormalizeLevelName(FName(*RawMapName))`
    - Detect type: `CurrentLevelType = DetermineLevelType(CurrentLevelName)`
    - Log: `UE_LOG(LogFCLevelManager, Log, TEXT("Initialized: Level=%s, Type=%s"), ...)`
  - [x] Compile successfully
- [ ] **Testing After Step 1A.3**
  - [x] PIE starts successfully
  - [x] Main menu appears
  - [x] Check Output Log: Should see "LogFCLevelManager: Initialized: Level=L_Office, Type=Office"
  - [x] "New Legacy" button works
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" button loads save
  - [x] No crashes or errors

##### Step 1A.4: Integrate with UFCGameInstance Save/Load

- [x] **Implementation**
  - [x] Open `UFCGameInstance.cpp`
  - [x] Add `#include "Core/FCLevelManager.h"` at top
  - [x] In `LoadGameAsync()`:
    - Find code that strips PIE prefix manually (likely `RightChop(9)` or similar)
    - Replace with: `UFCLevelManager* LevelMgr = GetSubsystem<UFCLevelManager>();`
    - Replace manual comparison with: `if (LevelMgr && LevelMgr->IsSameLevelLoad(TargetLevelFName))`
  - [x] In `RestorePlayerPosition()`:
    - After restoring position, add:
    ```cpp
    UFCLevelManager* LevelMgr = GetSubsystem<UFCLevelManager>();
    if (LevelMgr)
    {
        LevelMgr->UpdateCurrentLevel(FName(*CurrentLevelName));
    }
    ```
  - [x] Compile successfully
- [x] **Testing After Step 1A.4** ⚠️ CRITICAL TEST POINT
  - [x] PIE starts successfully
  - [x] Main menu appears
  - [x] "New Legacy" → walk around office
  - [x] F6 QuickSave
  - [x] Check logs: Save should succeed
  - [x] Door interaction → return to menu
  - [x] "Continue" button → should load save correctly
  - [x] Player spawns at saved position (not spawn point)
  - [x] F9 QuickLoad → also works
  - [x] Check logs: LevelManager should show "UpdateCurrentLevel" calls
  - [x] No crashes or errors

##### Step 1A.5: OPTIONAL - Integrate with AFCPlayerController::BeginPlay (Only If Needed)

- [x] **Check Current Code First**
  - [x] Open `AFCPlayerController.cpp`
  - [x] Read `BeginPlay()` implementation
  - [x] Does it have string matching like `Contains("Office")` or `Contains("MainMenu")`?
    - **YES**: String matching found - proceeding with this step
- [x] **Implementation** (String matching found)
  - [x] Add `#include "Core/FCLevelManager.h"` at top
  - [x] In `BeginPlay()`:
    - Found string matching code: `if (!CurrentLevelName.Contains("Office") && !CurrentLevelName.Contains("MainMenu"))`
    - Replaced with: `if (LevelMgr && LevelMgr->IsGameplayLevel())`
  - [x] Removed manual level name string parsing (PIE prefix stripping)
  - [x] Compile successfully
- [x] **Testing After Step 1A.5**
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works
  - [x] Can walk around office
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" loads save correctly
  - [x] Check logs: No more string Contains() for level detection
  - [x] No crashes or errors

##### Post-Implementation Documentation

- [x] **Update Technical_Documentation.md**
  - [x] Add UFCLevelManager to directory structure
  - [x] Add UFCLevelManager subsystem section (2.1.5) after UFCGameInstance
  - [x] Document EFCLevelType enum and public API
  - [x] Document PIE prefix normalization logic
  - [x] Update UFCGameInstance section with LevelManager integration details
  - [x] Update AFCPlayerController section with BeginPlay simplification
  - [x] Update high-level architecture diagram to include LevelManager subsystem
  - [x] Update class interaction flow diagram
  - [x] Document L_Office dual-purpose architecture insight

**COMMIT POINT 1A**: `git add -A && git commit -m "feat: Add UFCLevelManager subsystem with level type detection"`

---

#### Refactoring 1B: UFCUIManager Subsystem (8-10 hours)

**Goal**: Centralize all UI widget creation and lifecycle management in a Game Instance Subsystem. Migrate UI logic from AFCPlayerController to UFCUIManager, eliminating Blueprint coupling and ensuring widgets persist across level transitions.

##### Step 1B.1: Create UFCUIManager Skeleton

- [x] **Implementation**

  - [x] Create `Source/FC/Core/FCUIManager.h`

    - Inherit from `UGameInstanceSubsystem`
    - Add `DECLARE_LOG_CATEGORY_EXTERN(LogFCUIManager, Log, All);`
    - Declare properties for widget class references:

      ```cpp
      UPROPERTY()
      TSubclassOf<UUserWidget> MainMenuWidgetClass;

      UPROPERTY()
      TSubclassOf<UUserWidget> SaveSlotSelectorWidgetClass;
      ```

    - Declare cached widget instance pointers:

      ```cpp
      UPROPERTY()
      TObjectPtr<UUserWidget> MainMenuWidget;

      UPROPERTY()
      TObjectPtr<UUserWidget> SaveSlotSelectorWidget;
      ```

    - Declare public methods (stubs only):
      ```cpp
      void ShowMainMenu();
      void HideMainMenu();
      void ShowSaveSlotSelector();
      void HideSaveSlotSelector();
      void HandleNewLegacyClicked();
      void HandleContinueClicked();
      void HandleLoadSaveClicked();
      void HandleOptionsClicked();
      void HandleQuitClicked();
      void HandleBackFromSaveSelector();
      void HandleSaveSlotSelected(const FString& SlotName);
      ```

  - [x] Create `Source/FC/Core/FCUIManager.cpp`
    - Add `DEFINE_LOG_CATEGORY(LogFCUIManager);`
    - Implement all methods as stubs with logging:
      ```cpp
      void UFCUIManager::ShowMainMenu()
      {
          UE_LOG(LogFCUIManager, Log, TEXT("ShowMainMenu() called (stub)"));
      }
      ```
  - [x] Add to `FC.Build.cs` if needed
  - [x] Compile successfully

- [x] **Testing After Step 1B.1**
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works
  - [x] Can walk around office
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" loads save correctly
  - [x] No crashes or errors

##### Step 1B.2: Implement Widget Lifecycle Methods

- [x] **Implementation**
  - [x] In `FCUIManager.cpp`, implement `ShowMainMenu()`:
    - Get GameInstance: `UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());`
    - Safety checks for `GI` and `MainMenuWidgetClass`
    - Create widget if not cached: `MainMenuWidget = CreateWidget<UUserWidget>(GetGameInstance(), MainMenuWidgetClass);`
    - Add to viewport: `MainMenuWidget->AddToViewport()`
    - Log success
  - [x] Implement `HideMainMenu()`:
    - Check if `MainMenuWidget` exists
    - Remove from parent: `MainMenuWidget->RemoveFromParent();`
    - Log success
  - [x] Implement `ShowSaveSlotSelector()`:
    - Similar to ShowMainMenu, create if needed
    - Add to viewport
    - Log success
  - [x] Implement `HideSaveSlotSelector()`:
    - Remove from parent
    - Log success
  - [x] Compile successfully
- [x] **Testing After Step 1B.2**
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works
  - [x] Can walk around office
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" loads save correctly
  - [x] No crashes or errors

##### Step 1B.3: Implement Button Callback Methods

- [x] **Implementation**
  - [x] In `FCUIManager.cpp`, implement `HandleNewLegacyClicked()`:
    - Get PlayerController: `AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());`
    - Safety check for PC
    - Call `PC->TransitionToGameplay();`
    - Call `HideMainMenu();`
    - Log action
  - [x] Implement `HandleContinueClicked()`:
    - Get GameInstance: `UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());`
    - Get most recent save: `FString SlotName = GI->GetMostRecentSave();`
    - Check if slot exists
    - Call `GI->LoadGameAsync(SlotName);`
    - Call `HideMainMenu();`
    - Log action
  - [x] Implement `HandleLoadSaveClicked()`:
    - Call `HideMainMenu();`
    - Call `ShowSaveSlotSelector();`
    - Log action
  - [x] Implement `HandleOptionsClicked()`:
    - Log TODO (placeholder for future options menu)
  - [x] Implement `HandleQuitClicked()`:
    - Get PlayerController
    - Call `UKismetSystemLibrary::QuitGame()`
    - Log action
  - [x] Implement `HandleBackFromSaveSelector()`:
    - Call `HideSaveSlotSelector();`
    - Call `ShowMainMenu();`
    - Log action
  - [x] Implement `HandleSaveSlotSelected(const FString& SlotName)`:
    - Get GameInstance
    - Call `GI->LoadGameAsync(SlotName);`
    - Call `HideSaveSlotSelector();`
    - Log action with slot name
  - [x] Compile successfully
- [x] **Testing After Step 1B.3**
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works
  - [x] Can walk around office
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" loads save correctly
  - [x] No crashes or errors

##### Step 1B.4: Wire UFCGameInstance to UIManager

- [x] **Implementation**

  - [x] Open `Source/FC/Core/UFCGameInstance.h`
  - [x] Add widget class properties (will be set in Blueprint):

    ```cpp
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> SaveSlotSelectorWidgetClass;
    ```

  - [x] Open `Source/FC/Core/UFCGameInstance.cpp`
  - [x] In `Init()` method, configure UIManager:
    ```cpp
    UFCUIManager* UIManager = GetSubsystem<UFCUIManager>();
    if (UIManager)
    {
        UIManager->MainMenuWidgetClass = MainMenuWidgetClass;
        UIManager->SaveSlotSelectorWidgetClass = SaveSlotSelectorWidgetClass;
        UE_LOG(LogFCGameInstance, Log, TEXT("UIManager configured with widget classes"));
    }
    ```
  - [x] Compile successfully

- [x] **Testing After Step 1B.4**
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works
  - [x] Can walk around office
  - [x] F6 QuickSave works
  - [x] Door interaction returns to menu
  - [x] "Continue" loads save correctly
  - [x] No crashes or errors

##### Step 1B.5: Update AFCPlayerController to Use UIManager

- [x] **Implementation**
  - [x] Open `Source/FC/Core/FCPlayerController.h`
  - [x] Remove widget class properties (moved to GameInstance)
  - [x] Remove widget instance pointers (moved to UIManager)
  - [x] Keep state transition methods: `InitializeMainMenu()`, `TransitionToGameplay()`, `ReturnToMainMenu()`
  - [x] Open `Source/FC/Core/FCPlayerController.cpp`
  - [x] Update `InitializeMainMenu()`:
    - Get UIManager: `UFCUIManager* UIManager = GetGameInstance()->GetSubsystem<UFCUIManager>();`
    - Replace widget creation with: `UIManager->ShowMainMenu();`
    - Keep input mode setup code
  - [x] Update `TransitionToGameplay()`:
    - Get UIManager
    - Call `UIManager->HideMainMenu();`
    - Keep camera blend and input mode setup code
  - [x] Remove old button callback methods (will be called from UIManager):
    - Remove `HandleNewLegacyClicked()`
    - Remove `HandleContinueClicked()`
    - Remove `HandleLoadSaveClicked()`
    - Remove `HandleOptionsClicked()`
    - Remove `HandleQuitClicked()`
    - Remove `CloseSaveSlotSelector()`
    - Remove `LoadSaveSlot()`
  - [x] Compile successfully

##### Step 1B.6: Update Blueprint - BP_FC_GameInstance

- [x] **Blueprint Configuration**
  - [x] Open `Content/FC/Blueprints/BP_FC_GameInstance`
  - [x] Find "UI" category in Details panel
  - [x] Set `MainMenuWidgetClass` → `/Game/FC/UI/Menus/WBP_MainMenu`
  - [x] Set `SaveSlotSelectorWidgetClass` → `/Game/FC/UI/Menus/SaveMenu/WBP_SaveSlotSelector`
  - [x] Compile Blueprint
  - [x] Save Blueprint

##### Step 1B.7: Update Blueprint - WBP_MainMenu

- [x] **Blueprint Rewiring**
  - [x] Open `Content/FC/UI/Menus/WBP_MainMenu`
  - [x] Find "New Legacy" button OnClicked event
    - Remove existing PlayerController call
    - Get Game Instance → Cast to FC Game Instance → Get Subsystem (UI Manager)
    - Call `HandleNewLegacyClicked()`
  - [x] Find "Continue" button OnClicked event
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleContinueClicked()`
  - [x] Find "Load Save" button OnClicked event
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleLoadSaveClicked()`
  - [x] Find "Options" button OnClicked event
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleOptionsClicked()`
  - [x] Find "Quit" button OnClicked event
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleQuitClicked()`
  - [x] Compile Blueprint
  - [x] Save Blueprint

##### Step 1B.8: Update Blueprint - WBP_SaveSlotSelector

- [x] **Blueprint Rewiring**
  - [x] Open `Content/FC/UI/Menus/SaveMenu/WBP_SaveSlotSelector`
  - [ ] Find "Back" button OnClicked event
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleBackFromSaveSelector()`
  - [x] Find save slot item OnClicked event (likely in WBP_SaveSlotItem or custom event)
    - Replace with: Get Game Instance → Get Subsystem (UI Manager) → `HandleSaveSlotSelected(SlotName)`
  - [x] Compile Blueprint
  - [x] Save Blueprint
- [x] **Testing After Step 1B.8** ⚠️ FULL REGRESSION TEST
  - [x] PIE starts successfully
  - [x] Main menu appears correctly
  - [x] "New Legacy" button works and transitions to gameplay
  - [x] Can walk around office with WASD + mouse
  - [x] F6 QuickSave works (check logs)
  - [x] Door interaction returns to menu (fade + reload)
  - [x] "Continue" button loads most recent save
  - [x] Player spawns at saved position
  - [x] "Load Save" button shows save slot selector
  - [x] Save slot selector shows available saves
  - [x] Clicking a save slot loads that save
  - [x] "Back" button returns to main menu
  - [x] "Quit" button closes PIE session
  - [x] No crashes or errors in Output Log
  - [x] No "Accessed None" Blueprint errors
  - [x] All UI interactions work smoothly

##### Post-Implementation Documentation

- [x] **Update Technical_Documentation.md**
  - [x] Add UFCUIManager to directory structure
  - [x] Add UFCUIManager subsystem section after UFCLevelManager
  - [x] Document widget lifecycle management
  - [x] Document button callback methods
  - [x] Update UFCGameInstance section with UIManager integration
  - [x] Update AFCPlayerController section with simplified UI responsibilities
  - [x] Update architecture diagrams to show UIManager subsystem
  - [x] Document Blueprint decoupling pattern

**COMMIT POINT 1B**: `git add -A && git commit -m "feat: Add UFCUIManager subsystem and migrate UI logic from PlayerController"`

---

#### Refactoring 1C – Widget Lifecycle / Persistent Transition Widget (4-6 hours)

##### Step 1C.1: Understand Current CreateTransitionWidget Implementation

- [x] **Analysis**
  - [x] Open `Source/FC/Core/FCTransitionManager.cpp`
  - [x] Find `CreateTransitionWidget()` method (lines 49-109)
  - [x] Document exactly what it does:
    - **Widget Outer**: `PlayerController` (line 93: `CreateWidget<UFCScreenTransitionWidget>(PlayerController, WidgetClass)`)
    - **Viewport Addition**: `AddToViewport(1000)` with Z-order 1000 (line 105)
    - **When Created**: Lazy initialization - only created on first `BeginFadeOut()` or `BeginFadeIn()` call (lines 123, 150)
    - **Not created in Initialize()** - widget is null until first transition
  - [x] Open `Source/FC/UI/FCScreenTransitionWidget.h/.cpp`
  - [x] Document widget's initial state:
    - **Initial Visibility**: `ESlateVisibility::HitTestInvisible` (line 108 FCTransitionManager.cpp)
    - **Initial Opacity**: Set to 0 in widget (starts transparent)
    - **BindWidget Components**: `Overlay_Main`, `Image_Fade`, `Overlay_Loading` (required Blueprint bindings)
    - **Fade Logic**: Tick-based animation, updates opacity over FadeDuration
    - **Delegates**: `OnFadeOutComplete`, `OnFadeInComplete` bound to manager callbacks (lines 102-103)
- [x] **Findings Documented**
  - **Current Pattern**: Widget is created with PlayerController as outer, added to viewport with high Z-order
  - **Problem**: Widget is destroyed when PlayerController is destroyed during level transitions
  - **Solution Needed**: Change outer to GameInstance for persistence across level loads
  - **Additional Issue**: `AddToViewport()` ties widget to specific player's viewport - need `AddViewportWidgetContent()` for true persistence

##### Step 1C.2: Change Widget Outer to GameInstance

- [x] **Implementation (FCTransitionManager.cpp)**
  - [x] In `CreateTransitionWidget()`, found lines 76-90:
    ```cpp
    // OLD CODE:
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: No valid PlayerController"));
        return;
    }
    TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(PlayerController, WidgetClass);
    ```
  - [x] Replaced with (lines 76-81):
    ```cpp
    // NEW CODE:
    // Create widget instance with GameInstance as outer for persistence across level loads
    // Using GameInstance instead of PlayerController ensures widget survives controller destruction
    TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(GetGameInstance(), WidgetClass);
    ```
  - [x] KEPT the `AddToViewport(1000)` call unchanged (line 92)
  - [x] Compile successfully
- [x] **Testing After Step 1C.2**
  - [x] PIE starts successfully
  - [x] Main menu appears
  - [x] "New Legacy" button works
  - [x] Door interaction → fade to black works
  - [x] Return to menu works
  - [x] F6 save → F9 load → works without fade
  - [x] Widget lifetime slightly extended (but still destroyed on level change)
  - [x] No visual differences yet
  - [x] No crashes or errors

##### Step 1C.3: Switch to AddViewportWidgetContent for Persistence

- [x] **Implementation (FCTransitionManager.cpp)**
  - [x] In `CreateTransitionWidget()`, found lines 92-95:
    ```cpp
    // OLD CODE:
    TransitionWidget->AddToViewport(1000);
    ```
  - [x] Replaced with (lines 92-109):
    ```cpp
    // NEW CODE:
    UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
    if (ViewportClient)
    {
        ViewportClient->AddViewportWidgetContent(
            TransitionWidget->TakeWidget(),
            1000  // ZOrder - very high to be on top
        );
        UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: TransitionWidget added to viewport (persistent)"));
    }
    else
    {
        UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: Failed to get GameViewportClient"));
        return;
    }
    ```
  - [x] Updated `Deinitialize()` to use RemoveViewportWidgetContent (lines 36-52)
  - [x] BeginFadeOut() and BeginFadeIn() already have null checks and CreateTransitionWidget() calls
  - [x] Compile successfully
- [x] **Testing After Step 1C.3** ⚠️ CRITICAL TEST - WIDGET PERSISTENCE
  - [x] PIE starts successfully
  - [x] Main menu appears
  - [x] "New Legacy" button works
  - [x] Door interaction → fade to black → reload level
  - [x] **KEY TEST**: Widget should persist across level reload (no flicker)
  - [x] Main menu appears after fade-in completes
  - [x] Trigger another transition (Continue → gameplay)
  - [x] Fade-out → fade-in should use SAME widget (check logs: should NOT see "creating widget" during transition)
  - [x] No duplicate transition widgets
  - [x] No crashes or errors

##### Step 1C.4: Initialize Widget in Fully Black State

- [x] **C++ Implementation (UFCScreenTransitionWidget)**

  - [x] Added `InitializeToBlack()` method to UFCScreenTransitionWidget.h:
    ```cpp
    /** Initialize widget to fully black state (for clean startup) */
    UFUNCTION(BlueprintCallable, Category = "Transition")
    void InitializeToBlack();
    ```
  - [x] Implemented in UFCScreenTransitionWidget.cpp:
    - Sets widget visibility to HitTestInvisible
    - Sets render opacity to 1.0 (fully opaque)
    - Initializes Image_Fade to visible, black, opacity 1.0
    - Hides loading indicator overlay
    - Sets CurrentOpacity = 1.0, bIsFading = false
  - [x] Compile successfully

- [x] **C++ Implementation (UFCTransitionManager)**

  - [x] Modified `Initialize()` to create widget immediately instead of lazy initialization
  - [x] Added viewport availability check and timer-based retry logic:
    - Widget created during Initialize() but ViewportClient is NULL initially
    - Set up 100ms timer to retry adding widget to viewport
    - Timer calls `EnsureWidgetInViewport()` when viewport becomes ready
  - [x] Created `EnsureWidgetInViewport()` helper method in FCTransitionManager.h/.cpp:
    - Checks if widget is already in viewport (via IsInViewport())
    - If not, attempts to add via AddViewportWidgetContent
    - Once added, calls `TransitionWidget->InitializeToBlack()`
    - Logs success or failure
  - [x] Updated `BeginFadeOut()` and `BeginFadeIn()` to call `EnsureWidgetInViewport()` first
    - Ensures widget is in viewport before any fade operation
    - Handles deferred addition from Initialize() timing issue
  - [x] Compile successfully

- [x] **Root Cause Analysis**

  - [x] Problem: ViewportClient not available during GameInstanceSubsystem::Initialize()
  - [x] Solution: Create widget during Initialize(), defer viewport addition with timer
  - [x] Widget gets added to viewport ~100ms after game start when viewport is ready
  - [x] InitializeToBlack() called once widget is successfully in viewport

- [x] **Testing After Step 1C.4** ⚠️ EXPECT BLACK SCREEN ON START
  - [x] PIE starts → screen should be FULLY BLACK (this is expected)
  - [x] Wait a few seconds → should stay black (widget initialized correctly)
  - [x] Screen is black from startup (timer successfully adds widget and initializes to black)
  - [x] No crashes or errors
  - [x] Logs show: "Widget added to viewport and initialized to black"

##### Step 1C.5: Implement Automatic Fade-In on Game Start

- [x] **Implementation (FCTransitionManager.cpp)**

  - [x] Modified `Initialize()` to use nested timer for fade-in:

    - First timer (100ms): Add widget to viewport via `EnsureWidgetInViewport()`
    - Second timer (200ms after first): Call `BeginFadeIn(1.5f)` for smooth reveal
    - Total delay: 300ms before fade-in starts

  - [x] **Additional Implementation** (to fix quick load issue):

    - Added `IsBlack()` method to `UFCScreenTransitionWidget` (checks opacity >= 0.95)
    - Added `IsBlack()` method to `UFCTransitionManager` (delegates to widget)
    - Modified `UFCGameInstance::OnPostLoadMapWithWorld()` to check `IsBlack()` instead of `IsFading()`
    - **Problem Fixed**: When quick loading from another level, screen stayed black because:
      - Fade-out completes → sets `bCurrentlyFading = false`
      - Level loads → `OnPostLoadMapWithWorld()` checks `IsFading()` → returns false
      - No fade-in triggered → screen stays black with loading spinner
    - **Solution**: Check if screen is actually black (`IsBlack()`) instead of if fade is in progress

  - [x] Compile successfully

- [x] **Testing After Step 1C.5** ⚠️ FINAL TEST - COMPLETE FEATURE
  - [x] PIE starts → screen starts BLACK
  - [x] After ~0.3 seconds → smooth fade-in to main menu (1.5 seconds)
  - [x] Main menu fully visible and functional
  - [x] "New Legacy" → fade works
  - [x] Door interaction → fade-out → level reload → fade-in (using same widget)
  - [x] "Continue" → fade-out → level load → fade-in works
  - [x] F9 Quick Load from another level → fade-in works correctly (no black screen)
  - [x] Cross-level transitions smooth with no flicker
  - [x] No duplicate widgets
  - [x] No screen pops or flashes
  - [x] Widget persists across ALL level changes
  - [x] No crashes or errors

**COMMIT POINT 1C**: `git add -A && git commit -m "feat: Persistent transition widget with automatic fade-in on game start (Task 5.14)"`

---

### ✅ PRIORITY 1 COMPLETE - FINAL VALIDATION

- [x] **Code Quality Check**
  - [x] No remaining string matching for level detection (search for `Contains("Office")`)
    - ✅ No `Contains("Office")` found
    - ✅ No `Contains("MainMenu")` found
    - ✅ No string-based level matching in codebase
    - ✅ All level detection uses `EFCLevelType` enum via `UFCLevelManager`
  - [x] No UI logic in AFCPlayerController (only delegation)
    - ✅ No `CreateWidget` calls in `AFCPlayerController`
    - ✅ No `AddToViewport` calls in `AFCPlayerController`
    - ✅ Only delegates to `UFCUIManager` subsystem (17 references, all delegation)
    - ✅ PlayerController maintains state but delegates all widget operations
  - [x] Widget outer is GameInstance (not PlayerController)
    - ✅ `UFCUIManager`: Widgets use `GetGameInstance()` as outer
    - ✅ `UFCTransitionManager`: TransitionWidget uses `GetGameInstance()` as outer
    - ✅ Transition widget uses `AddViewportWidgetContent()` for persistence
    - ✅ All UI widgets survive PlayerController destruction during level loads
  - [x] All new code follows UE_CodeConventions.md
    - ✅ Proper copyright headers (FCLevelManager, FCUIManager, FCTransitionManager)
    - ✅ Consistent class naming with FC prefix
    - ✅ Comprehensive documentation comments on all public methods
    - ✅ UFUNCTION/UPROPERTY specifiers properly used
    - ✅ BlueprintCallable/BlueprintType decorators where appropriate
  - [x] All new classes properly logged with DECLARE_LOG_CATEGORY
    - ✅ `LogFCLevelManager` (declared in .h, defined in .cpp)
    - ✅ `LogFCUIManager` (declared in .h, defined in .cpp)
    - ✅ `LogFCTransitions` (declared in .h, defined in .cpp)
    - ✅ All 8 log categories properly declared and defined
    - ✅ Consistent logging throughout all subsystems

**FINAL COMMIT**: `git add -A && git commit -m "refactor: Complete Priority 1 - Core Architecture (LevelManager, UIManager, Persistent Widgets)"`

**MERGE TO MASTER**: After all tests pass, merge `refactoring` branch into `master`

---

### Priority 2 – Data-Driven & State Systems (Before Week 4)

#### Refactoring 2A – Level Metadata System (Data-Driven Level Config)

- [x] **Design & Implementation (C++)**
  - [x] Create `FFCLevelMetadata` struct (row for DataTable)
  - [x] Add metadata lookup API in `UFCLevelManager` (e.g. `GetLevelMetadata(LevelName)`)
- [x] **Blueprint / Content**
  - [x] Create `DT_LevelMetadata` Data Table using `FFCLevelMetadata`
  - [x] Add rows for at least:
    - [x] `L_Office`
    - [x] `L_TopDown` / over­world test levels (if existing)
  - [x] Configure:
    - [x] `LevelType`
    - [x] `DefaultInputMode`
    - [x] `bRequiresFadeTransition`
    - [x] `bShowCursor`
    - [x] `DisplayName`
  - [x] `BP_FC_GameInstance`: Reference `DT_LevelMetadata` on `UFCLevelManager`
- [x] **Integration**
  - [x] Replace remaining string name checks with metadata-based logic
    - [x] No more `Contains("Office")` / `Contains("MainMenu")`
- [x] **Testing**
  - [x] Boot to main menu → correct metadata read, correct input mode / cursor
  - [x] Transition to office → metadata drives correct mapping mode
  - [x] Adding a new test level via Data Table requires **no** C++ changes

---

#### Refactoring 2B – Explicit Game State Machine

- [x] **Design & Implementation**
  - [x] Create `UFCGameStateManager` as `UGameInstanceSubsystem`
  - [x] Define `EFCGameStateID` enum (MainMenu, Office_Exploration, Office_TableView, Overworld_Travel, Combat_PlayerTurn, etc.)
  - [x] Implement:
    - [x] `GetCurrentState()`, `GetPreviousState()`
    - [x] `TransitionTo(State)`, `CanTransitionTo(State)`
    - [x] Optional: state stack (`PushState`, `PopState`) for Pause / modal states (deferred)
  - [x] Define valid state transitions and initialize them
  - [x] Add `OnStateChanged` multicast delegate
- [x] **Integration**
  - [x] Remove/replace old `CurrentGameState` from `AFCPlayerController` (kept as deprecated for backward compatibility)
  - [x] Update all game state changes to go through `UFCGameStateManager`
- [x] **Testing**
  - [x] Log all state transitions and verify expected order (MainMenu → Office_Exploration → Paused → Office_Exploration → etc.)
  - [x] Invalid transitions are blocked (and logged)

**COMMIT POINT 2A+2B**: `git add -A && git commit -m "refactor: Complete Priority 2 - Data-Driven Level Metadata & Game State Machine"`

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

**UPDATED ANALYSIS (Week 2 - November 20, 2025):**

**Current Status:**

```cpp
// AFCPlayerController.h - 220 lines
// AFCPlayerController.cpp - 1020 lines (grew from ~223 in Week 1)
// Total: 1240 lines across 2 files
// Public methods: 30+
// State properties: 15+
```

**Current Responsibilities After Priority 1 Refactorings:**

```cpp
class AFCPlayerController : public APlayerController
{
    // 1. Input Management ✅ APPROPRIATE
    void SetupInputComponent();
    void SetInputMappingMode(EFCInputMappingMode NewMode);
    void HandleInteractPressed();
    void HandlePausePressed();
    void HandleQuickSavePressed();
    void HandleQuickLoadPressed();
    void HandleTableObjectClick();

    // 2. Camera State & View Management ⚠️ GROWING (167 lines in SetCameraModeLocal alone)
    void SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime);
    EFCPlayerCameraMode CameraMode;
    ACameraActor* MenuCamera;
    ACameraActor* TableViewCamera;  // NEW in Week 2
    AActor* OriginalViewTarget;     // NEW in Week 2
    bool bIsInTableView;            // NEW in Week 2

    // 3. UI Widget Lifecycle 🔄 PARTIALLY REFACTORED
    // ✅ Main menu widgets → UFCUIManager (Week 1)
    // ✅ Pause menu widgets → UFCUIManager (Week 1)
    // ❌ Table widgets → STILL IN PlayerController (Week 2)
    void OnTableObjectClicked(AActor* TableObject);      // NEW in Week 2 (120 lines total)
    void ShowTableWidget(AActor* TableObject);           // NEW in Week 2
    void CloseTableWidget();                             // NEW in Week 2
    UUserWidget* CurrentTableWidget;                     // NEW in Week 2

    // 4. Game State Management ✅ APPROPRIATE
    void InitializeMainMenu();
    void TransitionToGameplay();
    void ReturnToMainMenu();
    void ResumeGame();
    EFCGameState CurrentGameState;
    bool bIsPauseMenuDisplayed;

    // 5. Transition Coordination ✅ CORRECTLY DELEGATED
    void FadeScreenOut(float Duration, bool bShowLoading);  // Delegates to UFCTransitionManager
    void FadeScreenIn(float Duration);                      // Delegates to UFCTransitionManager

    // 6. Save/Load Integration ✅ APPROPRIATE DELEGATION
    void DevQuickSave();                    // Delegates to UFCGameInstance
    void DevQuickLoad();                    // Delegates to UFCGameInstance
    void RestorePlayerPositionDeferred();

    // 7. Timer Management ⚠️ MULTIPLE TIMER PATTERNS
    void RestoreInputAfterBlend();
    // Inline lambda timers in:
    // - SetCameraModeLocal (line 467): Camera cleanup
    // - OnTableObjectClicked (line 940): Widget show delay
    // - ReturnToMainMenu (line 715): Level reload delay
};
```

**Week 2 Changes - Growth Analysis:**

**Removed (via Priority 1 refactorings):**

- ✅ Level name parsing logic → UFCLevelManager (~30 lines)
- ✅ Main menu widget lifecycle → UFCUIManager (~50 lines)
- ✅ Pause menu widget lifecycle → UFCUIManager (~40 lines)
- ✅ Transition widget management → UFCTransitionManager (~30 lines)
  **Total Removed: ~150 lines**

**Added (Week 2 - Table Interaction System):**

- ❌ Table interaction system (OnTableObjectClicked, ShowTableWidget, CloseTableWidget): ~120 lines
- ❌ Complex camera blending for table view (SetCameraModeLocal expansion): ~167 lines
- ❌ Widget-per-table-object management pattern
  **Total Added: ~287 lines**

**Net Growth: +137 lines** (from ~883 to 1020 lines)

**Problems Identified:**

1. **Camera Management Complexity** (Lines 431-598, 167 lines)

   - Handles 4 camera modes: FirstPerson, TableView, MainMenu, SaveSlotView
   - Dynamically spawns/destroys temporary cameras
   - Complex cleanup logic with lambda timers
   - Hardcoded 2.0f blend time throughout
   - **Should be**: UFCCameraManager component

2. **Table Widget Lifecycle Inconsistency** (Lines 900-1020, 120 lines)

   - Main menu uses UFCUIManager ✅
   - Pause menu uses UFCUIManager ✅
   - **Table widgets use PlayerController directly** ❌
   - Creates widgets with `CreateWidget(this, ...)` - tight coupling
   - **Should be**: Delegated to UFCUIManager like other UI

3. **State Tracking Fragmentation**

   - `CameraMode` (EFCPlayerCameraMode) - camera system state
   - `CurrentGameState` (EFCGameState) - game flow state
   - `bIsPauseMenuDisplayed` (bool) - UI state
   - `bIsInTableView` (bool) - camera/mode state
   - Multiple overlapping state variables without clear state machine
   - **Should be**: Unified state machine in UFCGameStateManager

4. **Timer Safety Risks**
   - Line 467 (`SetCameraModeLocal`): `[CurrentViewTarget]()` - raw pointer capture
   - Line 940 (`OnTableObjectClicked`): `[this, TableObject]()` - weak pointer risk
   - Line 715 (`ReturnToMainMenu`): `[this]()` - safe with UObject delegate
   - **Should be**: Use FTimerDelegate::CreateUObject consistently

**Problems:**

- Violates Single Responsibility Principle (SRP)
- Inconsistent refactoring: Some UI in subsystems, some still in controller
- Camera management growing complex (will get worse with overworld/combat)
- Timer safety patterns inconsistent
- Hard to extend for future perspectives (TopDown, Combat)

**Growth Projection:**

At current growth rate: 137 lines/week × 19 remaining weeks = +2603 lines
**Projected size by Week 21: 3623 lines** (unmaintainable)

**GDD/DRM Future Requirements:**

- Week 3: Overworld camera (topdown, zoom, pan) → +150 lines
- Week 4: Overworld UI widgets (fog-of-war) → +100 lines
- Week 5: Camp management UI + cameras → +120 lines
- Week 6: Combat camera (grid view) + UI → +200 lines
- Week 7: Time/Resource HUD → +80 lines
- Week 12: Camp task assignment UI → +100 lines
- Week 21: Reputation/Finance UI → +90 lines

Each new UI system adds 5-15 methods to PlayerController if not refactored!

**Violation of Conventions:**

> **UE_CodeConventions.md § 2.2**: "Modular Code Organization - Example modules: Combat, UI, Inventory, AI, etc."  
> **UE_CodeConventions.md § 4.3**: "Clean APIs - Design small, clear public interfaces."

**Recommended Solution (Updated for Week 2):**

Split into specialized components/subsystems:

```cpp
// PRIORITY: Refactor Table Widget Management to UFCUIManager

// 1. UFCUIManager - Game Instance Subsystem (EXTEND EXISTING)
class UFCUIManager : public UGameInstanceSubsystem
{
    // Already handles main menu, pause menu, save selector ✅

    // ADD: Table widget management (Week 2+)
    void ShowTableWidget(AActor* TableObject);
    void CloseTableWidget();
    UUserWidget* GetCurrentTableWidget() const;
    bool IsTableWidgetOpen() const;

    // Widget registry
    TMap<TSubclassOf<AActor>, TSubclassOf<UUserWidget>> TableWidgetMap;

    // Current table widget state
    UPROPERTY()
    TObjectPtr<UUserWidget> CurrentTableWidget;
};

// 2. UFCCameraManager - Player Controller Component (NEW - PRIORITY 2)
class UFCCameraManager : public UActorComponent
{
    // Handles all camera transitions and state
    void BlendToCamera(AActor* Target, float BlendTime = 2.0f, EViewTargetBlendFunction BlendFunc = VTBlend_Cubic);
    void BlendToMenuCamera(float BlendTime = 0.0f);
    void BlendToFirstPerson(float BlendTime = 2.0f);
    void BlendToTableObject(AActor* TableObject, float BlendTime = 2.0f);
    void RestorePreviousViewTarget(float BlendTime = 2.0f);

    // Camera state management
    EFCPlayerCameraMode GetCameraMode() const;
    AActor* GetOriginalViewTarget() const;

private:
    UPROPERTY()
    TObjectPtr<ACameraActor> MenuCamera;

    UPROPERTY()
    TObjectPtr<ACameraActor> TableViewCamera;

    UPROPERTY()
    TObjectPtr<AActor> OriginalViewTarget;

    EFCPlayerCameraMode CurrentCameraMode;
    bool bIsInTableView;
};

// 3. AFCPlayerController - SIMPLIFIED (Post-refactoring target)
class AFCPlayerController : public APlayerController
{
    // Core responsibilities only:

    // Input handling (entry points)
    void HandleTableObjectClick();
    void HandlePausePressed();
    void HandleInteractPressed();

    // Game state transitions
    void InitializeMainMenu();
    void TransitionToGameplay();
    void ReturnToMainMenu();

    // Component references
    UPROPERTY()
    TObjectPtr<UFCCameraManager> CameraManager;  // NEW

    // Delegates to subsystems/components:
    // - Camera → UFCCameraManager
    // - UI → UFCUIManager
    // - Transitions → UFCTransitionManager
    // - Level → UFCLevelManager
};
```

**Migration Priority for Week 2+:**

**IMMEDIATE (Before Week 3):**

1. Move table widget lifecycle to UFCUIManager
   - `ShowTableWidget()` → `UFCUIManager::ShowTableWidget()`
   - `CloseTableWidget()` → `UFCUIManager::CloseTableWidget()`
   - `CurrentTableWidget` property → UFCUIManager
   - Consistency with main menu/pause menu pattern

**HIGH PRIORITY (Before Week 4):** 2. Create UFCCameraManager component

- Extract camera blend logic from PlayerController
- Centralize camera state tracking
- Data-driven blend times (no more hardcoded 2.0f)

**MEDIUM PRIORITY (Before Week 6):** 3. Create UFCGameStateManager subsystem

- Unify `CurrentGameState`, `CameraMode`, `bIsPauseMenuDisplayed`, `bIsInTableView`
- Explicit state machine with transitions
- Prevents invalid state combinations
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

````

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
````

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
