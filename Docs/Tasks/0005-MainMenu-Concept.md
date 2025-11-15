# Task 5: In-World Main Menu - Feasibility & Implementation Concept

**Date:** 2025-11-15  
**Status:** Concept Phase  
**Complexity:** Medium-High

---

## Executive Summary

Creating an in-world main menu within the Office level (L_Office) is **highly feasible** with UE5's existing systems. The concept leverages camera blending, widget overlays, and state management already scaffolded in `AFCPlayerController`. This approach provides a seamless, immersive experience while maintaining clean separation between menu and gameplay states.

**Estimated Implementation Time:** 6-8 hours  
**Risk Level:** Low-Medium (camera blending may need iteration for feel)

---

## 1. Feasibility Analysis

### 1.1 Technical Feasibility ✅ HIGH

| Component                | Feasibility  | Notes                                                       |
| ------------------------ | ------------ | ----------------------------------------------------------- |
| **Single Level Design**  | ✅ Very High | No technical barriers; L_Office already exists              |
| **Static Camera Menu**   | ✅ Very High | Standard UE camera blending + widget overlay                |
| **State Management**     | ✅ Very High | Can extend existing `EFCPlayerCameraMode` enum              |
| **UI Overlay**           | ✅ Very High | UMG widget system + Z-order control                         |
| **Diegetic Logo**        | ✅ High      | Blueprint actor with static mesh/decal + optional overlay   |
| **Save/Load System**     | ⚠️ Medium    | Needs implementation but well-documented in UE5             |
| **Async Level Loading**  | ✅ High      | `UGameplayStatics::OpenLevelBySoftObjectPtr` with streaming |
| **Door Return Behavior** | ✅ Very High | Simple level reload with state reset                        |

### 1.2 Design Feasibility ✅ HIGH

**Advantages:**

- Eliminates jarring level transitions (menu ↔ office)
- Reinforces the "office as home base" narrative
- Logo placement as in-world object enhances immersion
- Desk focus creates natural visual anchor

**Challenges:**

- Menu must feel distinct from gameplay (solved via camera + input lock)
- Save slot UI needs careful UX design (map projection on desk is excellent thematic fit)
- Performance: single level always loaded (minimal concern for office-scale environment)

### 1.3 Flow Feasibility ✅ HIGH

All requested flows are achievable:

```
Game Start → L_Office (Menu Camera) → UI Overlay
├─ New Legacy → Intro Sequence → L_Office (Gameplay)
├─ Continue → Load Save → Transition to Save Location
├─ Load Save → Map UI → Select Slot → Load Save
├─ Options → Settings Widget
└─ Quit → Exit Application

Door Interaction (Gameplay) → Reload L_Office (Menu Camera)
```

---

## 2. Architecture Design

### 2.1 State Management

Extend existing `AFCPlayerController` enums:

```cpp
UENUM(BlueprintType)
enum class EFCPlayerCameraMode : uint8
{
    FirstPerson = 0,
    TableView,
    MainMenu,      // NEW: Static menu camera
    SaveSlotView   // NEW: Map view for save slots
};

UENUM(BlueprintType)
enum class EFCGameState : uint8
{
    MainMenu,      // Menu overlay active
    Gameplay,      // Normal first-person play
    TableView,     // Table interaction mode
    Paused,        // Pause menu
    Loading        // Async loading screen
};
```

### 2.2 Camera System

**Implementation:** Two `ACameraActor` instances in L_Office

1. **MenuCamera** (Static/Slow Movement)

   - Position: ~300-500 units from desk, slight angle
   - Look At: Desk center (CameraTargetPoint on BP_OfficeDesk)
   - FOV: 60-75° (tighter for intimate feel)
   - Post Process: Slight vignette, subtle depth of field on background
   - Optional: Slow oscillation (±2° rotation) for "breathing" effect

2. **FirstPersonCamera** (Already exists on character)
   - Standard FPS camera
   - Used during gameplay

**Transition:**

```cpp
void AFCPlayerController::SetCameraMode(EFCPlayerCameraMode NewMode)
{
    // Blend time: 1.5-2.0s for smooth, cinematic transition
    float BlendTime = 2.0f;
    FViewTargetTransitionParams BlendParams;
    BlendParams.BlendTime = BlendTime;
    BlendParams.BlendFunction = VTBlend_Cubic; // Smooth ease-in-out

    switch (NewMode)
    {
        case EFCPlayerCameraMode::MainMenu:
            SetViewTargetWithBlend(MenuCameraActor, BlendTime, BlendParams);
            DisableInput(this); // Block all gameplay input
            break;
        case EFCPlayerCameraMode::FirstPerson:
            SetViewTargetWithBlend(GetPawn(), BlendTime, BlendParams);
            EnableInput(this);
            break;
    }

    CameraMode = NewMode;
}
```

### 2.3 Widget Architecture

**WBP_MainMenu** (Primary Menu Widget)

```
Canvas Panel
├─ Background Tint (optional darkening)
├─ Logo Image (Top Center or overlay)
│   └─ Texture: T_FallenCompass_Logo (to be created)
├─ Button Container (Vertical Box)
│   ├─ Btn_NewLegacy ("New Legacy")
│   ├─ Btn_Continue ("Continue") [Disabled if no saves]
│   ├─ Btn_LoadSave ("Load Save Game")
│   ├─ Btn_Options ("Options")
│   └─ Btn_Quit ("Quit")
└─ Version Text (Bottom Right, small)
```

**WBP_SaveSlotSelector** (Map-Based UI)

```
Canvas Panel
├─ Map Background Image (parchment/desk texture)
├─ Scroll Box (Save Slots)
│   └─ WBP_SaveSlotItem (Repeated)
│       ├─ Slot Number
│       ├─ Save Timestamp
│       ├─ Location Name
│       ├─ Expedition Progress
│       └─ Click Detection
└─ Back Button
```

### 2.4 Logo Implementation

**Option A: Non-Diegetic Overlay (Easiest - Recommended for MVP)**

- Create `T_FallenCompass_Logo` texture (PNG with alpha)
- Display in UMG widget at top center
- Drop shadow for readability
- Fade in/out with menu

**Option B: Diegetic + Overlay (Enhanced Immersion)**

- Create `BP_LogoPlaque` blueprint actor
  - Static mesh: brass/wooden plaque
  - Material: M_LogoPlaque with emissive logo texture
  - Location: On wall above desk (visible in menu camera)
- Overlay: Semi-transparent logo in UI for readability
- Menu camera frames both desk AND logo plaque

**Recommendation:** Start with Option A, iterate to Option B in polish phase.

---

## 3. Implementation Plan

### 3.1 Phase 1: Core Menu State (2-3 hours)

**Tasks:**

1. Create Menu Camera Actor in L_Office

   - Position above/behind desk, angled down
   - Add to `BP_OfficeDesk` as child component or separate actor
   - Set up CineCameraComponent for professional framing

2. Extend `AFCPlayerController`

   ```cpp
   UPROPERTY(EditAnywhere, Category = "Camera")
   TObjectPtr<ACameraActor> MenuCameraActor;

   UPROPERTY(VisibleAnywhere, Category = "State")
   EFCGameState CurrentGameState;

   void InitializeMainMenu();      // Called on level load
   void TransitionToGameplay();    // Called after "New Legacy" or load
   void ReturnToMainMenu();        // Called from door interaction
   ```

3. Create `WBP_MainMenu` widget

   - Basic button layout
   - Placeholder logo (text or simple image)
   - Hook up button events to controller functions

4. Modify L_Office to start in menu state
   - PlayerStart disabled or repositioned away from view
   - Menu camera active on BeginPlay
   - Widget spawned automatically

**Acceptance Criteria:**

- Game starts with menu camera view
- Buttons visible and clickable
- "Quit" button works
- No input during menu state

### 3.2 Phase 2: New Legacy Flow (1-2 hours)

**Tasks:**

1. Create intro sequence level `L_Intro` (placeholder)

   - Simple fade-to-black with text
   - Or cinematic camera sequence
   - Ends with `OpenLevel(L_Office, "?StartMode=Gameplay")`

2. Implement "New Legacy" button

   ```cpp
   void AFCPlayerController::OnNewLegacyClicked()
   {
       // Option A: Direct to gameplay
       TransitionToGameplay();

       // Option B: Play intro first (Week 2+)
       // UGameplayStatics::OpenLevel(this, "L_Intro");
   }

   void AFCPlayerController::TransitionToGameplay()
   {
       CurrentGameState = EFCGameState::Gameplay;

       // Remove menu widget
       if (MainMenuWidget)
       {
           MainMenuWidget->RemoveFromParent();
       }

       // Blend to first-person camera
       SetCameraMode(EFCPlayerCameraMode::FirstPerson);

       // Spawn/teleport player to gameplay start position
       if (AFCFirstPersonCharacter* Character = Cast<AFCFirstPersonCharacter>(GetPawn()))
       {
           Character->SetActorLocation(GameplayStartLocation);
           Character->SetActorRotation(GameplayStartRotation);
       }

       // Enable input
       EnableInput(this);
   }
   ```

**Acceptance Criteria:**

- "New Legacy" button transitions to first-person gameplay
- Player spawns at correct location (not menu camera position)
- All movement/look/interact inputs function
- Menu widget removed from viewport

### 3.3 Phase 3: Door Return Flow (1 hour)

**Tasks:**

1. Update `BP_OfficeDoor` interaction

   ```cpp
   void ABP_OfficeDoor::OnInteract(AActor* Interactor)
   {
       if (AFCPlayerController* PC = Cast<AFCPlayerController>(Interactor->GetInstigatorController()))
       {
           PC->ReturnToMainMenu();
       }
   }
   ```

2. Implement `ReturnToMainMenu()`
   ```cpp
   void AFCPlayerController::ReturnToMainMenu()
   {
       // Fade out
       UGameplayStatics::GetPlayerCameraManager(this, 0)->StartCameraFade(
           0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true
       );

       // Delay for fade, then reload level
       FTimerHandle TimerHandle;
       GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
       {
           UGameplayStatics::OpenLevel(this, FName("L_Office"));
       }, 1.0f, false);
   }
   ```

**Acceptance Criteria:**

- Door interaction in gameplay triggers fade-to-black
- Level reloads cleanly
- Menu state reinitialized (camera, widget, input)
- No gameplay state carries over

### 3.4 Phase 4: Save/Load System Foundation (2-3 hours)

**Tasks:**

1. Create `UFCSaveGame` class

   ```cpp
   UCLASS()
   class UFCSaveGame : public USaveGame
   {
       GENERATED_BODY()

   public:
       UPROPERTY()
       FString SaveSlotName;

       UPROPERTY()
       FDateTime Timestamp;

       UPROPERTY()
       FString CurrentLevelName;

       UPROPERTY()
       FVector PlayerLocation;

       UPROPERTY()
       FRotator PlayerRotation;

       // Expedition data (Week 2+)
       UPROPERTY()
       TArray<FExpeditionData> ActiveExpeditions;
   };
   ```

2. Implement save/load manager in `UFCGameInstance`

   ```cpp
   void UFCGameInstance::SaveGame(const FString& SlotName)
   {
       UFCSaveGame* SaveGameInstance = Cast<UFCSaveGame>(
           UGameplayStatics::CreateSaveGameObject(UFCSaveGame::StaticClass())
       );

       // Populate data
       SaveGameInstance->CurrentLevelName = GetWorld()->GetName();
       // ... gather player state

       UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
   }

   void UFCGameInstance::LoadGame(const FString& SlotName)
   {
       if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
       {
           UFCSaveGame* LoadedGame = Cast<UFCSaveGame>(
               UGameplayStatics::LoadGameFromSlot(SlotName, 0)
           );

           // Show loading screen
           // Async load level
           FLatentActionInfo LatentInfo;
           UGameplayStatics::LoadStreamLevel(
               this, FName(*LoadedGame->CurrentLevelName),
               true, true, LatentInfo
           );

           // Restore player state after level load
       }
   }
   ```

3. Create `WBP_SaveSlotSelector` widget

   - List available save slots (scan slot names)
   - Display metadata (timestamp, location)
   - Highlight most recent auto-save
   - Back button returns to main menu

4. Hook up "Continue" button

   - Loads most recent save (auto-save or quicksave)
   - Disabled if no saves exist

5. Hook up "Load Save Game" button
   - Opens `WBP_SaveSlotSelector` widget
   - Camera pans to map/desk area (optional enhancement)

**Acceptance Criteria:**

- Can create and load save games
- Save slots display in UI
- Continue button loads most recent save
- Load Save Game shows slot selector
- Loading preserves player location and level
- Async loading prevents hitches

### 3.5 Phase 5: Polish & Ambience (1-2 hours)

**Tasks:**

1. Add ambient details to L_Office menu camera view

   - Particle system: Dust motes in light shafts
   - Animated material: Candle flicker (emissive pulse)
   - Window lighting: Subtle animated caustics
   - Sound: Ambient loop (clock ticking, distant wind, paper rustle)

2. Camera enhancements

   - Add subtle camera shake (0.1-0.2 intensity, low frequency)
   - Or slow rotation/float (Sine wave ±1-2° over 10-15s)
   - Depth of field focusing on desk

3. Menu widget polish

   - Button hover effects (highlight, sound)
   - Smooth fade-in on menu appear
   - Font selection (period-appropriate serif)
   - Sound effects for button clicks

4. Transition polish
   - Audio fade during camera transitions
   - Ambient sound crossfade (menu ambience → gameplay ambience)
   - Loading screen with theme-appropriate art

**Acceptance Criteria:**

- Menu feels alive and atmospheric
- Transitions feel smooth and intentional
- Audio enhances immersion
- Visual effects don't distract from UI readability

---

## 4. Technical Considerations

### 4.1 Performance

**Concerns:**

- Level always loaded (no unload between menu/gameplay)

**Mitigation:**

- Office is small (~2000 UU²), minimal perf impact
- Menu widgets are lightweight
- Camera transitions use standard UE blending (optimized)

**Recommendation:** No special optimization needed for MVP.

### 4.2 Input Management

**Current System:**

- `EFCInputMappingMode` enum with contexts
- `SetInputMappingMode()` already implemented

**Menu State Handling:**

```cpp
void AFCPlayerController::InitializeMainMenu()
{
    // Disable ALL input contexts
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
    }

    // UI-only input mode
    SetInputMode(FInputModeUIOnly());
    bShowMouseCursor = true;
}

void AFCPlayerController::TransitionToGameplay()
{
    // Restore first-person input context
    SetInputMappingMode(EFCInputMappingMode::FirstPerson);
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}
```

### 4.3 Save System Architecture

**Slot Naming Convention:**

```
AutoSave_001    // Auto-save on expedition start/return
AutoSave_002    // Rotating auto-saves (3-5 slots)
QuickSave       // Single quick-save slot
Manual_001      // Manual save slots (unlimited or cap at 10)
Manual_002
...
```

**Async Loading Flow:**

```cpp
void UFCGameInstance::LoadGameAsync(const FString& SlotName)
{
    // 1. Show loading widget
    ShowLoadingScreen();

    // 2. Load save data
    UFCSaveGame* SaveData = Cast<UFCSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    // 3. Async load level
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnLevelLoadComplete");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = FGuid::NewGuid().A;

    UGameplayStatics::LoadStreamLevel(
        this, FName(*SaveData->CurrentLevelName),
        true, false, LatentInfo
    );

    // 4. Store save data for post-load restoration
    PendingSaveData = SaveData;
}

void UFCGameInstance::OnLevelLoadComplete()
{
    // 5. Restore player state
    if (AFCPlayerController* PC = Cast<AFCPlayerController>(
        GetWorld()->GetFirstPlayerController()))
    {
        PC->RestoreFromSaveData(PendingSaveData);
    }

    // 6. Hide loading screen
    HideLoadingScreen();

    // 7. Transition to gameplay
    PC->TransitionToGameplay();
}
```

### 4.4 Level Reload vs Persistent Level

**Option A: Level Reload (Recommended for MVP)**

- Door interaction: `OpenLevel(L_Office)` (full reload)
- Pros: Clean state reset, simpler to implement
- Cons: Brief loading screen (minimal for office size)

**Option B: Persistent Level + State Management**

- Level stays loaded, controller manages state transitions
- Pros: No loading screen
- Cons: Must carefully reset all gameplay state manually

**Recommendation:** Use Option A for MVP. Level reload time will be <1s for office environment.

---

## 5. User Experience Flow

### 5.1 First Launch

```
1. Launch game
2. UE splash screen
3. L_Office loads
4. Camera: Static menu view of desk
5. Widget: WBP_MainMenu fades in (1s delay)
6. Audio: Ambient office sounds start
7. Player sees:
   - Logo (top center or on wall)
   - Menu buttons
   - Atmospheric scene (dust, candle flicker)
8. Mouse cursor visible
```

### 5.2 New Legacy Flow

```
User clicks "New Legacy"
1. Button: Highlight + click sound
2. Widget: Menu fades out (0.5s)
3. Camera: Blend to first-person (2s, smooth cubic ease)
4. Audio: Ambience crossfade
5. Input: Mouse cursor hidden, movement enabled
6. Player at desk, facing room
7. Tutorial prompt (optional): "Press E to interact with desk"
```

### 5.3 Door Return Flow

```
Player walks to door and presses E
1. Interaction prompt: "Leave office"
2. Camera: Fade to black (1s)
3. Audio: Door open sound + fade out
4. Level: Reload L_Office
5. Camera: Fade in to menu view (1s)
6. Widget: Menu appears
7. Player back in menu state
```

### 5.4 Load Save Flow

```
User clicks "Load Save Game"
1. Widget: Menu fades out
2. Camera: Optional pan to map on desk (1s)
3. Widget: WBP_SaveSlotSelector fades in
4. Player sees list of save slots with metadata
5. User selects slot
6. Loading screen: Thematic art + progress bar
7. Level loads asynchronously
8. Camera: Fade in to gameplay at saved location
9. Player resumes from exact save point
```

---

## 6. Risks & Mitigations

### 6.1 Camera Blending Feel

**Risk:** Menu→gameplay transition may feel sluggish or disorienting  
**Mitigation:**

- Iterate on blend time (1.5-3s range)
- Test VTBlend functions (Linear, Cubic, EaseIn, EaseOut)
- Add slight FOV transition (menu: 65° → gameplay: 90°)
- User testing for motion sensitivity

### 6.2 Save/Load Complexity

**Risk:** Async loading errors, state desync, save corruption  
**Mitigation:**

- Implement comprehensive error handling
- Auto-backup before saves
- Clear loading state visualization
- Log all save/load operations
- Version save data format for future compatibility

### 6.3 Performance on Older Hardware

**Risk:** Persistent office level + save system may impact low-end PCs  
**Mitigation:**

- Profile on minimum spec hardware
- Implement texture streaming
- LOD meshes for office props
- Scalability settings for particles/lighting

### 6.4 Scope Creep

**Risk:** "Diegetic map UI" and ambience polish can expand indefinitely  
**Mitigation:**

- Implement in phases (MVP → Polish → Enhanced)
- Timebox each phase
- Mark enhancements as "post-MVP"
- Focus on functional correctness first

---

## 7. Testing Checklist

### 7.1 Functional Tests

- [ ] Game starts in menu state
- [ ] All menu buttons respond to clicks
- [ ] "New Legacy" transitions to gameplay
- [ ] Player can move/look/interact after transition
- [ ] Door interaction returns to menu
- [ ] Menu state resets correctly after door return
- [ ] "Continue" loads most recent save
- [ ] "Load Save Game" shows slot selector
- [ ] Loading saves restores correct level and position
- [ ] "Options" opens settings (future)
- [ ] "Quit" exits game cleanly

### 7.2 Visual Tests

- [ ] Menu camera frames desk well
- [ ] Logo visible and readable
- [ ] Buttons have clear hover states
- [ ] Camera blend is smooth (no jarring cuts)
- [ ] Ambient effects visible but not distracting
- [ ] Save slot UI is readable and clear
- [ ] Loading screen displays during async loads

### 7.3 Audio Tests

- [ ] Ambient sounds loop smoothly
- [ ] Button clicks have feedback
- [ ] Audio fades during transitions
- [ ] No audio pops or glitches
- [ ] Volume levels balanced

### 7.4 Edge Cases

- [ ] Clicking buttons rapidly doesn't break state
- [ ] Loading non-existent save handled gracefully
- [ ] Corrupted save file shows error message
- [ ] Save during camera transition works correctly
- [ ] Menu works with gamepad input
- [ ] Resizing window doesn't break UI layout

---

## 8. Deliverables

### 8.1 Code

- [ ] `AFCPlayerController` extensions (camera mode, state management)
- [ ] `UFCGameInstance` save/load system
- [ ] `UFCSaveGame` class
- [ ] `ABP_OfficeDoor` updated interaction logic
- [ ] Input mapping mode handling for menu state

### 8.2 Blueprints

- [ ] `BP_MenuCamera` actor in L_Office
- [ ] `BP_LogoPlaque` (if using diegetic option)
- [ ] Updated `BP_OfficeDesk` with camera target

### 8.3 Widgets

- [ ] `WBP_MainMenu`
- [ ] `WBP_SaveSlotSelector`
- [ ] `WBP_SaveSlotItem` (list entry)
- [ ] `WBP_LoadingScreen` (optional for MVP)

### 8.4 Assets

- [ ] `T_FallenCompass_Logo` texture
- [ ] Ambient sound: `A_Office_Ambience`
- [ ] SFX: `S_ButtonClick`, `S_DoorOpen`, `S_MenuTransition`
- [ ] Particle system: `P_DustMotes`
- [ ] Material: `M_CandleFlicker` (animated emissive)

### 8.5 Documentation

- [ ] Update `Technical_Documentation.md` with menu system architecture
- [ ] Update `0001-tasks.md` marking Task 5 complete
- [ ] Save file format specification
- [ ] Git commit: "feat: in-world main menu with save/load system"

---

## 9. Future Enhancements (Post-MVP)

### 9.1 Enhanced Diegetic UI

- Map projection on desk for save slot visualization
- Physical save "ledger" book on desk
- Logo as embossed leather/brass plaque with lighting
- Interactive globe for level selection

### 9.2 Dynamic Ambience

- Time-of-day lighting changes in menu
- Weather effects visible through windows
- Seasonal variations (snow, rain, autumn leaves)
- Expedition memorabilia appears after completions

### 9.3 Advanced Transitions

- Custom camera paths (not just blends)
- Character-initiated door animation
- Parallax scrolling for UI elements
- Cinematic letterboxing during transitions

### 9.4 Social Features

- Cloud saves integration
- Save sharing (export/import save files)
- Leaderboards for expedition completion times
- Community challenges

---

## 10. Conclusion

The in-world main menu concept is **technically sound and design-appropriate** for Fallen Compass. It enhances narrative cohesion by eliminating artificial menu screens and reinforces the office as the player's home base.

**Key Success Factors:**

1. Smooth camera transitions (iteration required)
2. Clear visual hierarchy in UI despite atmospheric scene
3. Robust save/load system with error handling
4. Performance monitoring on target hardware

**Recommendation:** Proceed with phased implementation, prioritizing functional MVP over visual polish. The architecture is extensible for future enhancements without refactoring.

**Go/No-Go Decision:** ✅ **GO** - All major risks have clear mitigations, and the concept aligns with the game's immersive design philosophy.

---

**Next Steps:**

1. Review concept with team/stakeholders
2. Create task breakdown in 0001-tasks.md (Task 5 subtasks)
3. Begin Phase 1 implementation (Core Menu State)
4. Schedule playtest after Phase 2 completion
