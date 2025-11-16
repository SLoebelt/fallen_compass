# Transition System Implementation Concept

**Date**: November 16, 2025  
**Tasks**: 5.11, 5.12, 5.13  
**Focus**: Modular, reusable screen transitions and level load flows

---

## Overview

This document outlines the implementation of a comprehensive, modular transition system that handles:

1. **Screen fade transitions** (black fade in/out with optional loading indicator)
2. **Smart load transitions** (same-level smooth blend vs. cross-level fade)
3. **Delayed menu fade-in** (polished initial level load experience)

**Design Principles**:

- **Modularity**: All components are reusable across different contexts
- **Separation of Concerns**: Transition logic separate from game logic
- **Extensibility**: Easy to add new transition types (e.g., blur, wipe effects)
- **Performance**: Minimal overhead, efficient widget management
- **Consistency**: Unified API for all transition needs

---

## Task 5.11: Modular Screen Transition System

### Architecture

```
UFCGameInstance
    └── UFCTransitionManager (Subsystem)
            ├── UFCScreenTransitionWidget (persistent widget)
            └── Manages all screen fades/transitions
```

### Component 1: UFCScreenTransitionWidget (UMG)

**Location**: `/Content/FC/UI/Transitions/WBP_ScreenTransition`

**Purpose**: Full-screen overlay with configurable fade animations and loading indicator.

**Widget Hierarchy**:

```
CanvasPanel (Root)
    └── Overlay_Main
        ├── Image_Fade (full-screen black, controls opacity)
        └── Overlay_Loading (visibility controlled)
            ├── Image_LoadingBG (semi-transparent panel)
            └── CircularThrobber or ProgressBar
```

**Properties**:

```cpp
// Animation state
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float CurrentOpacity = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bIsFading = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bShowLoadingIndicator = false;

// Animation curves (optional for smooth easing)
UPROPERTY(EditAnywhere, BlueprintReadWrite)
UCurveFloat* FadeCurve;
```

**Functions**:

```cpp
// Start fade to black (0 → 1)
UFUNCTION(BlueprintCallable, Category = "Transition")
void BeginFadeOut(float Duration, bool bShowLoading);

// Start fade from black (1 → 0)
UFUNCTION(BlueprintCallable, Category = "Transition")
void BeginFadeIn(float Duration);

// Tick function for smooth lerping
void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

// Delegates
UPROPERTY(BlueprintAssignable, Category = "Transition")
FOnFadeComplete OnFadeOutComplete;

UPROPERTY(BlueprintAssignable, Category = "Transition")
FOnFadeComplete OnFadeInComplete;
```

**Implementation Notes**:

- Use `SetRenderOpacity()` on root overlay for smooth fading
- Loading indicator visibility toggled independently of fade
- Timer-based or tick-based animation (recommend tick for smoothness)
- Lerp opacity using `FMath::FInterpTo()` or curve evaluation

---

### Component 2: UFCTransitionManager (Subsystem)

**Location**: `/Source/FC/Core/FCTransitionManager.h/cpp`

**Purpose**: Game instance subsystem that manages persistent transition widget and provides unified transition API.

**Class Definition**:

```cpp
UCLASS()
class FC_API UFCTransitionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Primary API
    UFUNCTION(BlueprintCallable, Category = "Transition")
    void BeginFadeOut(float Duration = 1.0f, bool bShowLoadingIndicator = false);

    UFUNCTION(BlueprintCallable, Category = "Transition")
    void BeginFadeIn(float Duration = 1.0f);

    // Delegates for callbacks
    UPROPERTY(BlueprintAssignable, Category = "Transition")
    FOnTransitionComplete OnFadeOutComplete;

    UPROPERTY(BlueprintAssignable, Category = "Transition")
    FOnTransitionComplete OnFadeInComplete;

    // State query
    UFUNCTION(BlueprintPure, Category = "Transition")
    bool IsFading() const { return bCurrentlyFading; }

protected:
    // Widget management
    UPROPERTY()
    TSubclassOf<UFCScreenTransitionWidget> TransitionWidgetClass;

    UPROPERTY()
    UFCScreenTransitionWidget* TransitionWidget;

    bool bCurrentlyFading = false;

    void CreateTransitionWidget();
    void OnWidgetFadeOutComplete();
    void OnWidgetFadeInComplete();
};
```

**Implementation Details**:

1. **Initialize()**: Load widget class reference, create widget instance, add to viewport with Z-order 1000 (always on top)
2. **BeginFadeOut()**: Delegate to widget, bind to widget's OnFadeOutComplete, set internal state
3. **BeginFadeIn()**: Same pattern for fade in
4. **Callbacks**: Re-broadcast widget callbacks as subsystem delegates, clean up bindings

**Advantages of Subsystem Approach**:

- Persistent across level loads (game instance subsystem)
- Single widget instance (no repeated creation/destruction)
- Accessible from any context: `UGameInstance::GetSubsystem<UFCTransitionManager>()`
- Automatic lifecycle management
- Blueprint-friendly with `GetGameInstance()` node

---

### Component 3: AFCPlayerController Helper Functions

**Purpose**: Convenient access to transition system without repeated subsystem lookups.

**Functions**:

```cpp
UFUNCTION(BlueprintCallable, Category = "Transition")
void FadeScreenOut(float Duration = 1.0f, bool bShowLoading = false);

UFUNCTION(BlueprintCallable, Category = "Transition")
void FadeScreenIn(float Duration = 1.0f);
```

**Implementation**:

```cpp
void AFCPlayerController::FadeScreenOut(float Duration, bool bShowLoading)
{
    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        UFCTransitionManager* TransitionMgr = GameInstance->GetSubsystem<UFCTransitionManager>();
        if (TransitionMgr)
        {
            TransitionMgr->BeginFadeOut(Duration, bShowLoading);
        }
    }
}
```

---

### Testing Strategy

**Test Case 1**: Basic Fade Out/In

1. Create test button in debug menu
2. Click → trigger `FadeScreenOut(1.0f, false)`
3. Verify screen smoothly fades to black over 1 second
4. After fade completes, trigger `FadeScreenIn(1.0f)`
5. Verify screen smoothly fades back to clear

**Test Case 2**: Loading Indicator

1. Trigger `FadeScreenOut(1.5f, true)`
2. Verify loading spinner/indicator appears during fade
3. Verify indicator remains visible while faded out
4. Trigger `FadeScreenIn(1.0f)`
5. Verify indicator disappears before/during fade in

**Test Case 3**: Rapid Calls

1. Trigger fade out, immediately trigger fade in
2. Verify system handles interruption gracefully (no flicker)
3. Second call should override first or queue appropriately

**Test Case 4**: Cross-Level Persistence

1. Trigger fade out in one level
2. Load different level during fade
3. Verify transition widget persists and completes
4. Trigger fade in in new level
5. Verify widget still functional

---

## Task 5.12: Smart Load Transitions Based on Level Context

### Problem Statement

Currently, loading a save always uses the same transition (camera blend). We need:

- **Same-level loads** (Office → Office): Smooth camera blend, no loading screen
- **Cross-level loads** (Office → Future Level): Fade to black with loading indicator

### Architecture Changes

```
UFCSaveGame
    └── Add FName SavedLevelName

UFCGameInstance
    └── Add GetCurrentLevelName()
    └── Modify LoadGameAsync() to compare levels

AFCPlayerController::OnSaveGameLoaded()
    └── Branch logic:
        ├── Same Level → TransitionToGameplay() (existing)
        └── Different Level → FadeOut → LoadLevel → FadeIn
```

---

### Component 1: Extend UFCSaveGame

**Location**: `/Source/FC/Core/FCSaveGame.h`

**Changes**:

```cpp
UPROPERTY(BlueprintReadWrite, Category = "Save Game")
FName SavedLevelName;
```

**Modifications to SaveGame()**:

```cpp
void UFCGameInstance::SaveGame(const FString& SlotName)
{
    UFCSaveGame* SaveGameInstance = Cast<UFCSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UFCSaveGame::StaticClass())
    );

    // Existing save logic...
    SaveGameInstance->PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    SaveGameInstance->PlayerRotation = PlayerController->GetPawn()->GetActorRotation();

    // NEW: Capture current level name
    SaveGameInstance->SavedLevelName = GetCurrentLevelName();

    // Continue saving...
}
```

---

### Component 2: Implement GetCurrentLevelName()

**Location**: `/Source/FC/Core/FCGameInstance.h/cpp`

**Function**:

```cpp
UFUNCTION(BlueprintPure, Category = "Game Instance")
FName GetCurrentLevelName() const;
```

**Implementation**:

```cpp
FName UFCGameInstance::GetCurrentLevelName() const
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Get persistent level name (e.g., "L_Office", "L_Expedition")
        FString LevelName = World->GetMapName();

        // Remove "UEDPIE_0_" prefix if in PIE mode
        LevelName.RemoveFromStart(World->StreamingLevelsPrefix);

        return FName(*LevelName);
    }
    return NAME_None;
}
```

**Notes**:

- Returns clean level name without PIE prefix
- Works in both editor and packaged builds
- Persistent level only (ignores streaming sub-levels)

---

### Component 3: Refactor OnSaveGameLoaded()

**Location**: `/Source/FC/Core/FCPlayerController.cpp`

**Current Implementation** (simplified):

```cpp
void AFCPlayerController::OnSaveGameLoaded(bool bSuccess)
{
    if (!bSuccess) return;

    // Close menus
    if (SaveSlotSelectorWidget)
        SaveSlotSelectorWidget->RemoveFromParent();

    // Transition to gameplay
    TransitionToGameplay();

    // Restore position after 2.1s
    FTimerHandle PositionRestoreTimer;
    GetWorldTimerManager().SetTimer(PositionRestoreTimer, [this]() {
        UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
        GameInstance->RestorePlayerPosition();
    }, 2.1f, false);
}
```

**New Implementation** (smart branching):

```cpp
void AFCPlayerController::OnSaveGameLoaded(bool bSuccess)
{
    if (!bSuccess) return;

    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
    if (!GameInstance) return;

    // Get saved level and current level
    FName SavedLevel = GameInstance->GetPendingLoadData().SavedLevelName;
    FName CurrentLevel = GameInstance->GetCurrentLevelName();

    // Close any open menus
    if (SaveSlotSelectorWidget && SaveSlotSelectorWidget->IsInViewport())
    {
        SaveSlotSelectorWidget->RemoveFromParent();
    }

    // Branch based on level match
    if (SavedLevel == CurrentLevel)
    {
        // SAME LEVEL: Smooth camera transition (existing behavior)
        HandleSameLevelLoad();
    }
    else
    {
        // DIFFERENT LEVEL: Fade transition with level load
        HandleCrossLevelLoad(SavedLevel);
    }
}
```

---

### Component 4: Same-Level Load Handler

**Function**:

```cpp
void AFCPlayerController::HandleSameLevelLoad()
{
    // Existing smooth transition
    TransitionToGameplay();

    // Delayed position restoration (after camera blend)
    FTimerHandle PositionRestoreTimer;
    GetWorldTimerManager().SetTimer(PositionRestoreTimer, [this]()
    {
        UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
        if (GameInstance)
        {
            GameInstance->RestorePlayerPosition();
            UE_LOG(LogTemp, Log, TEXT("Position restored after same-level load"));
        }
    }, 2.1f, false);
}
```

**No changes from current implementation** - this is the existing polished flow.

---

### Component 5: Cross-Level Load Handler

**Function**:

```cpp
void AFCPlayerController::HandleCrossLevelLoad(FName TargetLevelName)
{
    // Start fade to black with loading indicator
    FadeScreenOut(1.0f, true);

    // Bind to fade complete callback
    UFCTransitionManager* TransitionMgr = GetGameInstance()->GetSubsystem<UFCTransitionManager>();
    if (TransitionMgr)
    {
        // Lambda capture for level name
        FOnTransitionComplete::FDelegate FadeCompleteDelegate;
        FadeCompleteDelegate.BindLambda([this, TargetLevelName]()
        {
            // Fade complete, now load the level
            LoadTargetLevel(TargetLevelName);
        });

        TransitionMgr->OnFadeOutComplete.Add(FadeCompleteDelegate);
    }
}

void AFCPlayerController::LoadTargetLevel(FName LevelName)
{
    UGameplayStatics::OpenLevel(this, LevelName);

    // Note: Position restoration happens in new level's BeginPlay
    // GameInstance carries pending load data across level transition
}
```

**Flow**:

1. Fade to black (1s) with loading indicator visible
2. After fade completes, trigger level load
3. New level streams in (loading happens during black screen)
4. New level's BeginPlay checks for pending restore data
5. Apply position/rotation
6. Fade in (1s) to reveal new level

---

### Component 6: Pending Load Data System

**Purpose**: Carry restore data across level loads (cross-level only).

**Location**: `/Source/FC/Core/FCGameInstance.h`

**Structure**:

```cpp
USTRUCT(BlueprintType)
struct FFCPendingLoadData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FName SavedLevelName;

    UPROPERTY(BlueprintReadWrite)
    FVector PlayerLocation;

    UPROPERTY(BlueprintReadWrite)
    FRotator PlayerRotation;

    UPROPERTY(BlueprintReadWrite)
    bool bHasPendingRestore = false;
};
```

**GameInstance Changes**:

```cpp
UPROPERTY()
FFCPendingLoadData PendingLoadData;

UFUNCTION(BlueprintPure)
FFCPendingLoadData GetPendingLoadData() const { return PendingLoadData; }

UFUNCTION(BlueprintCallable)
void SetPendingLoadData(const FFCPendingLoadData& Data) { PendingLoadData = Data; }

UFUNCTION(BlueprintCallable)
void ClearPendingLoadData() { PendingLoadData.bHasPendingRestore = false; }
```

**Modified LoadGameAsync()**:

```cpp
void UFCGameInstance::LoadGameAsync(const FString& SlotName)
{
    // Existing load logic...
    UFCSaveGame* LoadedSave = Cast<UFCSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    // Store data for cross-level access
    PendingLoadData.SavedLevelName = LoadedSave->SavedLevelName;
    PendingLoadData.PlayerLocation = LoadedSave->PlayerLocation;
    PendingLoadData.PlayerRotation = LoadedSave->PlayerRotation;
    PendingLoadData.bHasPendingRestore = true;

    // Broadcast success (controller handles same/cross-level logic)
    OnGameLoaded.Broadcast(true);
}
```

---

### Component 7: Cross-Level Restore in BeginPlay

**Location**: `/Source/FC/Core/FCPlayerController.cpp` (or GameMode)

**Function**:

```cpp
void AFCPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
    if (GameInstance && GameInstance->GetPendingLoadData().bHasPendingRestore)
    {
        // We just loaded into a new level from a save
        FFCPendingLoadData LoadData = GameInstance->GetPendingLoadData();

        // Small delay to ensure level is fully initialized
        FTimerHandle CrossLevelRestoreTimer;
        GetWorldTimerManager().SetTimer(CrossLevelRestoreTimer, [this, LoadData]()
        {
            // Apply position/rotation
            APawn* ControlledPawn = GetPawn();
            if (ControlledPawn)
            {
                ControlledPawn->SetActorLocation(LoadData.PlayerLocation);
                ControlledPawn->SetActorRotation(LoadData.PlayerRotation);
            }

            // Fade in from black
            FadeScreenIn(1.0f);

            // Clear pending data
            UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
            GI->ClearPendingLoadData();

            UE_LOG(LogTemp, Log, TEXT("Cross-level position restored and faded in"));
        }, 0.5f, false); // Small delay for level initialization
    }
}
```

**Flow**:

1. New level loads
2. BeginPlay checks for pending restore flag
3. If set, wait 0.5s for full initialization
4. Apply position/rotation
5. Fade in from black
6. Clear pending data

---

### Testing Strategy

**Test Case 1**: Same-Level Load (Office → Office)

1. Save game in office at specific position
2. Return to menu (door interaction)
3. Click "Continue" or load from save selector
4. **Expected**: Smooth camera blend from MenuCamera to FirstPerson over 2s
5. **Expected**: Position/rotation restored after 2.1s delay
6. **Expected**: No fade to black, no loading screen
7. **Verify**: Player at exact saved position

**Test Case 2**: Cross-Level Load (Office → Mock Level)

1. Create mock second level (e.g., `L_TestLevel`) for testing
2. Mock save file with `SavedLevelName = "L_TestLevel"`
3. Load save from office menu
4. **Expected**: Screen fades to black over 1s
5. **Expected**: Loading indicator appears and spins
6. **Expected**: Level loads in background (UGameplayStatics::OpenLevel)
7. **Expected**: New level's BeginPlay detects pending restore
8. **Expected**: Position applied, then fade in over 1s
9. **Verify**: Player appears in new level at correct position

**Test Case 3**: Edge Cases

- Load fails mid-fade → Verify graceful error handling
- Level load fails → Verify error message, fade in to menu
- Rapid save/load cycles → No transition stacking or flicker

---

## Task 5.13: Delayed Menu Fade-In on Level Start

### Problem Statement

Currently, when the game starts or player returns to menu via door, the menu UI appears instantly. We want:

1. Level loads and renders (MenuCamera active)
2. 1.5-2.0 second delay showing the atmospheric scene
3. Menu UI smoothly fades in over 1 second

**Requirements**:

- Only trigger on **fresh level load** (game start or door return)
- Do NOT trigger when loading a save game
- Reuse transition system for consistency

---

### Architecture

```
AFCPlayerController::InitializeMainMenu()
    └── Check: Is this a fresh load or save load?
        ├── Fresh Load → Spawn menu hidden, delay, fade in
        └── Save Load → Skip fade (menu already handled)
```

---

### Component 1: Load Context Tracking

**Purpose**: Distinguish between fresh menu load and save game load.

**Location**: `/Source/FC/Core/FCPlayerController.h`

**Property**:

```cpp
UPROPERTY()
bool bIsReturningFromSaveLoad = false;
```

**Usage**:

- Set to `true` in `OnSaveGameLoaded()` before any transitions
- Set to `false` in `InitializeMainMenu()` after checking
- Used to skip menu fade-in animation

---

### Component 2: Refactor InitializeMainMenu()

**Current Implementation** (simplified):

```cpp
void AFCPlayerController::InitializeMainMenu()
{
    // Set up input mode
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
    SetInputMode(InputMode);
    bShowMouseCursor = true;

    // Create and show menu
    if (MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
        MainMenuWidget->AddToViewport(0);
    }

    // Set camera
    SetCameraMode(EFCPlayerCameraMode::MainMenu);
    CurrentGameState = EFCGameState::MainMenu;
}
```

**New Implementation** (with delayed fade):

```cpp
void AFCPlayerController::InitializeMainMenu()
{
    // Set up input mode (but don't focus yet if fading)
    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = true;

    // Create menu widget
    if (MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

        if (bIsReturningFromSaveLoad)
        {
            // Skip fade animation - instant display
            MainMenuWidget->AddToViewport(0);
            InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
            SetInputMode(InputMode);
            bIsReturningFromSaveLoad = false; // Reset flag
        }
        else
        {
            // Fresh load - use delayed fade-in
            StartMenuFadeIn();
        }
    }

    // Set camera and state
    SetCameraMode(EFCPlayerCameraMode::MainMenu);
    CurrentGameState = EFCGameState::MainMenu;
}
```

---

### Component 3: Implement StartMenuFadeIn()

**Function**:

```cpp
void AFCPlayerController::StartMenuFadeIn()
{
    if (!MainMenuWidget) return;

    // Add widget to viewport but invisible
    MainMenuWidget->SetRenderOpacity(0.0f);
    MainMenuWidget->AddToViewport(0);

    // Delay before starting fade (1.5-2.0s to show atmospheric scene)
    float DelayDuration = 1.5f; // Configurable
    FTimerHandle MenuFadeDelayTimer;

    GetWorldTimerManager().SetTimer(MenuFadeDelayTimer, [this]()
    {
        // Start smooth fade-in animation
        FadeInMenuWidget();
    }, DelayDuration, false);
}
```

---

### Component 4: Implement FadeInMenuWidget()

**Option A**: Manual Tick-Based Lerp

```cpp
void AFCPlayerController::FadeInMenuWidget()
{
    if (!MainMenuWidget) return;

    MenuFadeAlpha = 0.0f;
    MenuFadeDuration = 1.0f;
    MenuFadeElapsed = 0.0f;
    bIsFadingInMenu = true;

    // Enable tick for smooth lerping
    // In Tick():
    if (bIsFadingInMenu)
    {
        MenuFadeElapsed += DeltaTime;
        float Alpha = FMath::Clamp(MenuFadeElapsed / MenuFadeDuration, 0.0f, 1.0f);

        // Smooth curve (ease in-out)
        Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

        MainMenuWidget->SetRenderOpacity(Alpha);

        if (Alpha >= 1.0f)
        {
            bIsFadingInMenu = false;
            OnMenuFadeInComplete();
        }
    }
}

void AFCPlayerController::OnMenuFadeInComplete()
{
    // Set input focus to menu
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
    SetInputMode(InputMode);
}
```

**Option B**: UMG Animation (Recommended)

Create UMG animation in `WBP_MainMenu`:

1. Open `WBP_MainMenu` in UMG editor
2. Create new animation track "Anim_FadeIn"
3. Add track for root panel opacity: 0 → 1 over 1 second
4. Use ease curve for smooth transition

**Controller Code**:

```cpp
void AFCPlayerController::FadeInMenuWidget()
{
    if (!MainMenuWidget) return;

    // Call Blueprint function to play animation
    // (Requires Blueprint exposure)
    UFunction* PlayAnimFunc = MainMenuWidget->FindFunction(FName("PlayFadeInAnimation"));
    if (PlayAnimFunc)
    {
        MainMenuWidget->ProcessEvent(PlayAnimFunc, nullptr);
    }

    // Or use direct UMG animation binding (if exposed to C++)
    // MainMenuWidget->PlayAnimation(FadeInAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);

    // Set focus after animation completes (1s timer)
    FTimerHandle FocusTimer;
    GetWorldTimerManager().SetTimer(FocusTimer, [this]()
    {
        OnMenuFadeInComplete();
    }, 1.0f, false);
}
```

---

### Component 5: Ensure Save Loads Skip Fade

**Modifications**:

In `OnSaveGameLoaded()` (same-level branch):

```cpp
void AFCPlayerController::HandleSameLevelLoad()
{
    // Set flag to skip menu fade on any menu return
    bIsReturningFromSaveLoad = true;

    // Existing transition logic...
    TransitionToGameplay();
    // ... position restore timer ...
}
```

In `ReturnToMainMenu()` (door interaction):

```cpp
void AFCPlayerController::ReturnToMainMenu()
{
    // Fade to black
    FadeScreenOut(1.0f, false);

    // Ensure flag is FALSE for door return (we want fade-in)
    bIsReturningFromSaveLoad = false;

    // After fade, reload level
    // InitializeMainMenu() will trigger fade-in since flag is false
}
```

---

### Testing Strategy

**Test Case 1**: Fresh Game Start

1. Launch game (PIE or standalone)
2. L_Office loads, MenuCamera activates
3. **Expected**: Menu UI is invisible for 1.5 seconds
4. **Expected**: Atmospheric scene visible (dust, candles, rain sounds)
5. **Expected**: After 1.5s, menu smoothly fades in over 1 second
6. **Expected**: After fade completes, buttons are focusable/clickable
7. **Verify**: No abrupt UI pop-in

**Test Case 2**: Door Return to Menu

1. Start new game, enter gameplay
2. Walk to door, press E to interact
3. **Expected**: Fade to black, level reloads
4. **Expected**: Menu fade-in animation triggers (same as fresh start)
5. **Verify**: Consistent experience with game start

**Test Case 3**: Save Load (Same Level)

1. Save game in office
2. Return to menu via door
3. Click "Continue"
4. **Expected**: Smooth camera blend to gameplay
5. **Expected**: NO menu fade animation during this process
6. **Verify**: Instant transition to gameplay, no menu flicker

**Test Case 4**: Save Load (Cross Level)

1. Mock save in different level
2. Load from office menu
3. **Expected**: Fade to black, level loads, fade in to gameplay
4. **Expected**: NO menu appears at any point
5. **Verify**: Direct to gameplay in new level

---

## Implementation Order

### Phase 1: Foundation (Task 5.11)

1. Create `UFCScreenTransitionWidget` UMG widget
2. Implement fade in/out animations (tick-based lerp)
3. Create `UFCTransitionManager` subsystem class
4. Wire up widget creation and lifecycle in subsystem
5. Add helper functions to `AFCPlayerController`
6. Test basic fade out/in functionality

**Estimated Time**: 2-3 hours

---

### Phase 2: Smart Loads (Task 5.12)

1. Add `SavedLevelName` to `UFCSaveGame`
2. Implement `GetCurrentLevelName()` in game instance
3. Modify `SaveGame()` to capture level name
4. Create pending load data structure
5. Refactor `OnSaveGameLoaded()` with branching logic
6. Implement `HandleSameLevelLoad()` (extract existing code)
7. Implement `HandleCrossLevelLoad()` (new fade flow)
8. Implement cross-level restore in BeginPlay
9. Test same-level loads (existing behavior preserved)
10. Test cross-level loads (new fade behavior)

**Estimated Time**: 3-4 hours

---

### Phase 3: Menu Fade-In (Task 5.13)

1. Add `bIsReturningFromSaveLoad` flag to controller
2. Create UMG fade-in animation in `WBP_MainMenu`
3. Refactor `InitializeMainMenu()` with branching
4. Implement `StartMenuFadeIn()` with delay timer
5. Implement `FadeInMenuWidget()` calling UMG animation
6. Set flag correctly in save load flows
7. Test fresh game start (fade-in triggers)
8. Test door return (fade-in triggers)
9. Test save loads (fade-in skipped)

**Estimated Time**: 2 hours

---

### Phase 4: Integration Testing

1. Full playthrough test:
   - Game start → menu fade-in
   - New Legacy → gameplay transition
   - Save game → door return → menu fade-in
   - Continue → smooth camera blend (same-level)
   - Mock cross-level load → fade out/in with loading
2. Edge case testing:
   - Rapid button clicks during fades
   - Level load failures
   - Save corruption handling
3. Performance profiling (fade tick costs)

**Estimated Time**: 1-2 hours

---

## Reusability and Extension

### Reusable Components

- **UFCTransitionManager**: Use for any screen transition need (not just loads)
  - Future: Add blur fade, wipe transitions, custom effects
  - Callable from any gameplay context (GameMode, HUD, Blueprints)
- **UFCScreenTransitionWidget**: Template for other transition widgets
  - Can create variants (colored fades, custom shapes)
- **Pending Load Data System**: Template for cross-level data persistence
  - Future: Expedition state, inventory, quests

### Extension Points

1. **Custom Transition Types**:

   ```cpp
   enum class EFCTransitionType
   {
       Fade,      // Current implementation
       Blur,      // Gaussian blur fade
       Wipe,      // Directional wipe
       Circular   // Iris in/out
   };

   void UFCTransitionManager::BeginTransition(EFCTransitionType Type, float Duration);
   ```

2. **Audio Integration**:

   ```cpp
   UPROPERTY(EditAnywhere, Category = "Audio")
   USoundCue* TransitionSound;

   // Play whoosh/impact sound on fade complete
   ```

3. **Save Slot Preview Images**:
   - Capture screenshot on save
   - Display in `WBP_SaveSlotItem`
   - Use fade system for preview transitions

---

## Summary

This implementation provides:

- ✅ **Modular transition system** usable throughout the project
- ✅ **Smart load detection** (same-level smooth vs. cross-level fade)
- ✅ **Polished menu presentation** (delayed fade-in on fresh loads)
- ✅ **Extensible architecture** for future transition types
- ✅ **Consistent player experience** across all load scenarios

All components are decoupled, testable, and reusable for future features (expedition transitions, map loads, etc.).
