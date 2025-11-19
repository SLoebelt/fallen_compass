## Relevant Files

- `/Docs/UE_NamingConventions.md` - Unreal Engine naming convention document and desired folder structure.
- `/Docs/UE_CodeConventions.md` - Unreal Engine MUST FOLLOW coding conventions.
- `/Docs/Fallen_Compass_GDD_v0.2.md` - Game Design Document
- `/Docs/Fallen_Compass_DRM.md` - Development Roadmap

---

## Week 1 – Project Skeleton & First-Person Office Basics

> Naming: All custom classes use the **FC** prefix (e.g. `UFCGameInstance`, `AFCGameMode`, `AFCFirstPersonCharacter`).  
> Primary platform: **Windows**, Linux/macOS enabled but only tested at phase ends.

---

### 1.0 Create Unreal Engine 5.7 C++ project and configure Git

- [x] **1.1 Review project documents**

  - [x] Open `/Docs/UE_NamingConventions.md` and note:
    - [x] Required folder structure (e.g. `/Source/FC`, `/Content/FC`).
    - [x] Class naming rules (FC prefix).
  - [x] Skim `/Docs/Fallen_Compass_GDD_v0.2.md` for high-level context of office & perspective.
  - [x] Skim `/Docs/Fallen_Compass_DRM.md` for Week 1 goals.

- [x] **1.2 Create Unreal C++ project**

  - [x] Create a new Unreal Engine 5.7 **C++** project (e.g. "FallenCompass" with FC prefix).
  - [x] Ensure the project is configured for **Desktop / Console – Maximum Quality** and **Windows**.
  - [x] Verify that a basic starter level (Top Down Template) runs in PIE without errors.

- [ ] **1.3 Configure additional platforms**

  - [x] Enable **Linux** as a target platform in Project Settings.
  - [x] Enable **macOS** as a target platform in Project Settings.
  - [x] Disable **Android** as a target platform in Project Settings.
  - [x] Disable **iOS** as a target platform in Project Settings.
  - [x] Disable **TVOS** as a target platform in Project Settings.
  - [x] Disable **VisionOS** as a target platform in Project Settings.
  - [x] Confirm no build errors appear after enabling platforms (no need to actually build yet).

- [x] **1.4 Initialize Git repository**

  - [x] Initialize Git in the project root.
  - [x] Add a UE-focused `.gitignore` (exclude `Binaries/`, `Intermediate/`, `DerivedDataCache/`, most of `Saved/`).
  - [x] Ensure large auto-generated files and caches are ignored.

- [x] **1.5 Create README & branching conventions**
  - [x] Create `/README.md` with:
    - [x] Short project description ("Fallen Compass").
    - [x] Engine version (UE 5.7).
    - [x] Branching strategy (e.g., `master` + `feature/<name>`).
  - [x] Commit initial project state to `master` with message like `chore: initial UE5 C++ project setup`.

---

### 2.0 Implement core FC framework classes (GameInstance, GameMode, PlayerController)

- [x] **2.1 Create custom GameInstance class**

  - [x] In C++, create `UFCGameInstance` derived from `UGameInstance`.
  - [x] Register `UFCGameInstance` in Project Settings → Maps & Modes → Game Instance Class.
  - [x] Add placeholder fields/comments for future global data (e.g. current expedition, settings).

- [x] **2.2 Create custom GameMode class**

  - [x] Create `AFCGameMode` derived from `AGameModeBase`.
  - [x] Configure it as the default GameMode for the Office level (later, once Office level exists).
  - [x] Add basic logging in `BeginPlay()` to confirm it's being used.

- [x] **2.3 Create custom PlayerController class**

  - [x] Create `AFCPlayerController` derived from `APlayerController`.
  - [x] Set it as the default PlayerController Class in `AFCGameMode`.
  - [x] Add empty stubs / placeholders for:
    - [x] Handling interaction input.
    - [x] Handling ESC/pause menu logic.
    - [x] Managing camera state (first-person vs table-view).

- [x] **2.4 Verify framework wiring**
  - [x] Launch project in a simple test level using `AFCGameMode` and `AFCPlayerController`.
  - [x] Confirm via logs or debugger that:
    - [x] `UFCGameInstance` is constructed.
    - [x] `AFCGameMode` and `AFCPlayerController` are active.

---

### 3.0 Implement first-person character pawn with input mappings (movement, look, interact, ESC)

- [x] **3.1 Create first-person character class**

  - [x] Create `AFCFirstPersonCharacter` derived from `ACharacter`.
  - [x] Add a camera component as a child of the character (for first-person view).
  - [x] Optionally add a basic mesh/arms or placeholder component (not required in Week 1).

- [x] **3.2 Configure input mappings**

  - [x] In Project Settings → Input, configure:
    - [x] Axis mappings:
      - [x] `MoveForward` (W = +1, S = -1).
      - [x] `MoveRight` (D = +1, A = -1).
      - [x] `Look` (Mouse 2dAxis).
    - [x] Action mappings:
      - [x] `Interact` (e.g. E key).
      - [x] `Pause` (ESC key).
  - [x] Save and verify no Input mapping conflicts.

- [x] **3.3 Implement movement & look in character**

  - [x] In `AFCFirstPersonCharacter`, bind Axis and Action mappings in `SetupPlayerInputComponent`.
  - [x] Implement:
    - [x] Forward/backward movement for `MoveForward`.
    - [x] Right/left movement for `MoveRight`.
    - [x] Yaw rotation for `Turn`.
    - [x] Pitch rotation for `LookUp`.
  - [x] Ensure the camera rotates with the character and feels like a standard FPS.

- [x] **3.4 Connect character to GameMode**

  - [x] Set `AFCFirstPersonCharacter` as the **Default Pawn Class** in `AFCGameMode`.
  - [x] Launch PIE and confirm that the player spawns as `AFCFirstPersonCharacter`.

- [x] **3.5 Expose hooks for interaction and ESC**
  - [x] In `AFCFirstPersonCharacter` or `AFCPlayerController`:
    - [x] Bind `Interact` action to a function that will later call interactables (e.g. trace forward).
    - [x] Bind `Pause` action (ESC) to a function that will delegate pause/menu handling (eventually to controller/UI).

---

### 4.0 Create greybox Office level with spawn, table & door interactables, and camera positions

- [x] **4.1 Create Office level**

  - [x] Create a new level asset (e.g. `/Content/Levels/L_Office`).
  - [x] Assign `AFCGameMode` as the GameMode for this level (World Settings).
  - [x] Place a `PlayerStart` at a reasonable position in the room.

- [x] **4.2 Build basic greybox geometry**

  - [x] Add static meshes or BSP/geometry for:

    - [x] **Verify Clean State**
      - [x] From repo root, run: `cd W:\GameDev\FallenCompass; git status`
      - [x] Check for untracked files:
        - [x] No loose `.uasset` or `.umap` files in unexpected locations ✅
        - [x] No editor temporary files (`.tmp`, `.log` outside `/Saved`) ✅
        - [x] No build artifacts outside `/Binaries`, `/Intermediate` ✅
      - [x] Check modified files:
        - [x] Only intentional changes staged ✅
        - [x] No accidental modifications to engine content ✅

    - [x] **Stage All Week 1 Changes**
      - [x] Run: `git add Source/ Content/ Config/ Docs/`
      - [x] Run: `git status` → verify all intended changes staged
      - [x] Commit: `git commit -m "feat: Complete Week 1 - Office prototype, menu system, save/load, table view, pause menu"`

    - [x] **Create Milestone Tag**
      - [x] Run: `git tag -a milestone-week1 -m "Week 1 Complete: Core framework, office level, diegetic menu, interaction system"`
      - [x] Run: `git push origin master --tags`

    - [x] **Update Development Roadmap**
      - [x] Open `/Docs/Fallen_Compass_DRM.md`
      - [x] Mark Week 1 as complete:

        ```markdown
        ## Week 1 – Project Skeleton & First-Person Office Basics ✅ COMPLETE

        - Core framework (GameInstance, GameMode, PlayerController, Character)
        - Diegetic menu system in L_Office
        - Save/load system with slot selector
        - Table view interaction with ESC return
        - Pause menu with resume/quit functionality
        - Interface-based interaction system (C++ interface + BP implementations)
        ```

      - [x] Commit: `git commit -am "docs: Mark Week 1 milestone complete in DRM"`

  - [x] Create `ACameraActor` named "MenuCamera" in L_Office
    - [x] Position ~300-500 units from desk, angled down at desk
    - [x] Configure as `CineCameraComponent` with FOV 60-75°
    - [ ] Optional: Add slow oscillation (±2° over 10-15s) for subtle movement
  - [x] Add reference to MenuCamera in `AFCPlayerController`
  - [x] Extend `EFCPlayerCameraMode` enum:
    - [x] Add `MainMenu` state
    - [x] Add `SaveSlotView` state (for future map UI)
  - [x] Implement `SetCameraMode()` with smooth blend (2s, VTBlend_Cubic)
  - [x] Test: Camera blends smoothly between menu and first-person views

- [x] **5.2 Extend State Management in AFCPlayerController**

  - [x] Create `EFCGameState` enum:
    - [x] `MainMenu`, `Gameplay`, `TableView`, `Paused`, `Loading`
  - [x] Add `CurrentGameState` property to `AFCPlayerController`
  - [x] Implement `InitializeMainMenu()`:
    - [x] Disable all input contexts
    - [x] Set `FInputModeUIOnly()`
    - [x] Show mouse cursor
    - [x] Spawn and add `WBP_MainMenu` to viewport (Task 5.3)
    - [x] Set camera to MenuCamera
  - [x] Implement `TransitionToGameplay()`:
    - [x] Remove menu widget (Task 5.3)
    - [x] Blend camera to first-person
    - [ ] Teleport player to gameplay start position (Task 5.4)
    - [x] Restore FirstPerson input mapping context
    - [x] Set `FInputModeGameOnly()`, hide cursor
  - [x] Modify L_Office to call `InitializeMainMenu()` on BeginPlay

- [x] **5.3 Create Main Menu Widget (WBP_MainMenu)**

  - [x] Create UMG widget `WBP_MainMenu` at `/Game/FC/UI/Menus/`
  - [x] Add logo placeholder (top left)
  - [x] Add vertical button container with styled buttons:
    - [x] "New Legacy" (starts new game)
    - [x] "Continue" (loads most recent save, disabled if no saves)
    - [x] "Load Game" (opens save slot selector)
    - [x] "Options" (placeholder for settings)
    - [x] "Quit" (exits application)
  - [x] Add version text (bottom right, small font)
  - [x] Implement button hover effects (highlight, scale, sound)
  - [x] Hook buttons to `AFCPlayerController` functions

- [x] **5.4 Implement New Legacy flow**

  - [x] Implement `AFCPlayerController::OnNewLegacyClicked()`:
    - [x] Set `CurrentGameState = EFCGameState::Gameplay`
    - [x] Call `TransitionToGameplay()`
    - [x] Store gameplay start transform (position + rotation)
  - [x] Test flow:
    - [x] Game starts in menu state (static camera, UI visible)
    - [x] Click "New Legacy" → smooth camera blend to first-person
    - [x] Player spawns at correct location in office
    - [x] Movement, look, and interact inputs all functional
    - [x] Menu widget removed from screen

- [x] **5.5 Implement Door Return-to-Menu flow**

  - [x] Update `BP_OfficeDoor::Interact()`:
    - [x] Call `AFCPlayerController::ReturnToMainMenu()`
  - [x] Implement `ReturnToMainMenu()`:
    - [x] Fade camera to black (1s using CameraManager)
    - [x] Play door open sound effect
    - [x] After fade completes, reload L_Office level
  - [x] On level reload, ensure `InitializeMainMenu()` is called
  - [x] Test flow:
    - [x] In gameplay, walk to door and press E
    - [x] Screen fades to black smoothly
    - [x] Level reloads
    - [x] Menu state reinitialized (camera, widget, input mode)
    - [x] No gameplay state carries over

- [x] **5.6 Implement basic Save/Load system foundation**

  - [x] Create `UFCSaveGame` class derived from `USaveGame`:
    - [x] Add properties: `SaveSlotName`, `Timestamp`, `CurrentLevelName`
    - [x] Add properties: `PlayerLocation`, `PlayerRotation`
    - [x] Add placeholder for expedition data (Week 2+)
  - [x] Implement save/load manager in `UFCGameInstance`:
    - [x] `SaveGame(const FString& SlotName)` - serialize current state
    - [x] `LoadGameAsync(const FString& SlotName)` - async load with callback
    - [x] `GetAvailableSaveSlots()` - scan and return save metadata
    - [x] `GetMostRecentSave()` - return latest auto/quick save
  - [x] Implement slot naming convention:
    - [x] `AutoSave_001`, `AutoSave_002`, `AutoSave_003` (rotating)
    - [x] `QuickSave` (single slot)
    - [x] `Manual_001`, `Manual_002`, etc. (player saves)
  - [x] Test: Can create and load save games, data persists correctly

- [x] **5.7 Create Save Slot Selector Widget (WBP_SaveSlotSelector)**

  - [x] Create UMG widget `WBP_SaveSlotSelector` at `/Game/FC/UI/Menus/SaveMenu/`
  - [x] Add map/parchment background texture
  - [x] Add ScrollBox for save slot list
  - [x] Create `WBP_SaveSlotItem` widget template:
    - [x] Display: Slot name, timestamp, location, expedition progress
    - [x] Click detection for selection
    - [x] Hover state highlighting
  - [x] Add "Back" button to return to main menu
  - [x] Populate list with `GetAvailableSaveSlots()` results
  - [x] Hook selection to `LoadGameAsync()`

- [x] **5.8 Hook up Continue and Load Save Game buttons**

  - [x] Implement `OnContinueClicked()`:
    - [x] Get most recent save from GameInstance
    - [x] If no saves exist, disable button (grayed out)
    - [x] If save exists, call `LoadGameAsync()` with that slot
  - [x] Implement `OnLoadSaveClicked()`:
    - [x] Hide `WBP_MainMenu`
    - [x] Show `WBP_SaveSlotSelector`
    - [ ] Optional: Pan camera to map area on desk
  - [x] Implement `LoadSaveSlot()`:
    - [x] Blueprint-callable function for save slot selection
    - [x] Binds to OnGameLoaded delegate
    - [x] Calls LoadGameAsync with selected slot
  - [x] Implement `OnSaveGameLoaded()` callback:
    - [x] Triggered when save finishes loading
    - [x] Closes save slot selector if open
    - [x] Transitions to gameplay state
    - [x] Restores player position after camera transition (2.1s delay)
  - [x] Implement async loading flow:
    - [x] Show loading screen widget during level stream
    - [x] Restore player state after level loads
    - [x] Transition to gameplay state at saved location
  - [x] Implement `CloseSaveSlotSelector()`:
    - [x] Hide save slot selector widget
    - [x] Show main menu widget
  - [x] Test flow:
    - [x] Create manual save in office
    - [x] Return to menu via door
    - [x] Click "Continue" → loads into saved position
    - [x] Verify: Player location, rotation, level all correct
    - [x] Click "Load Game" → save slot selector appears
    - [x] Select slot → loads correctly
    - [x] Verify: All UI transitions work correctly

- [x] **5.9 Add atmospheric details and polish**

  - [x] Add ambient effects to menu camera view:
    - [x] Particle system: Dust via Niagara System (`NS_Dust`)
    - [x] Dust decals on wall and floor (`BP_DustDecal`)
  - [x] Add audio:
    - [x] Ambient loop: Sound cue with Rain sounds (`Cue_Rain`)
    - ~~Door open sound (`S_DoorOpen`)~~ - Deferred to polish phase
  - ~~Camera enhancements~~ - Deferred to polish phase
    - ~~Depth of field focusing on desk area~~
  - ~~Widget polish~~ - Will be handled by Task 5.11 and 5.12
    - ~~Smooth fade-in on menu appear (1s delay after level load)~~
    - ~~Drop shadows for text readability~~
  - [x] Test: Menu feels atmospheric but UI remains readable
  - **Status**: Atmospheric effects exceed expectations for current dev phase

- [x] **5.10 Implement modular screen transition system**

  - [x] Create `UFCScreenTransitionWidget` UMG widget:
    - [x] Full-screen black overlay with configurable opacity animation
    - [x] Event dispatcher for transition complete callbacks
  - [x] Implement `UFCTransitionManager` subsystem (UGameInstanceSubsystem):
    - [x] `BeginFadeOut(float Duration, bool bShowLoadingIndicator)` - Start fade to black
    - [x] Manages single persistent `UFCScreenTransitionWidget` instance
  - [x] Add helper functions to `AFCPlayerController`:
    - [x] `FadeScreenOut(float Duration = 1.0f, bool bShowLoading = false)`
    - [x] Both delegate to transition manager for actual work
  - [x] Test modular system:
    - [x] Can trigger fade from any context (controller, game mode, etc.)
    - [x] Loading indicator appears/disappears correctly

- [x] **5.11 Implement smart load transitions based on level context**

  - [x] Add level tracking to `UFCTransitionManager`:
    - [x] Add `FName CurrentLevelName` property to track loaded level
    - [x] Add `UpdateCurrentLevel(FName NewLevelName)` to update tracking after loads
  - [x] Modify `UFCGameInstance::LoadGameAsync()` for smart transitions:
    - [x] Strip UEDPIE prefix from level names for comparison
  - [x] Fix cross-level input restoration:
    - [x] Modified `AFCPlayerController::BeginPlay()` to detect non-office/non-menu levels
    - [x] Sets `FInputModeGameOnly`, hides cursor
  - [x] Test same-level load:
    - [x] Save in office, load from menu
    - [x] No jarring cuts or loading screens ✅
  - [x] Test cross-level load:
    - [x] Save in office, continue to test level
    - [x] Player has full controls (move/look) ✅
  - **Status**: Smart transitions fully functional

---

### 6.0 Implement in-game Office flow (table camera, ESC behaviour, pause menu, door → Main Menu)

> **Architecture**: Follow separation of concerns — PlayerController delegates to UIManager for UI logic; actor interaction uses a C++ BlueprintNativeEvent interface `IFCInteractable` and a character-attached `UFCInteractionComponent` (UE_CodeConventions.md §2.3, §5.1).

---

#### Step 6.1: Use C++ Interaction Interface and Component

##### Step 6.1.1: Confirm IFCInteractable C++ Interface

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation (`/Docs/Technical_Documentation.md`) and project file system (`/FC/*`)

- [x] **Implementation**
  - [x] Verify C++ interface exists at `/FC/Source/FC/Interaction/IFCInteractable.h` (already present)
  - [x] Confirm functions are declared as `BlueprintNativeEvent`:
    - [x] `OnInteract(AActor* Interactor)`
    - [x] `FText GetInteractionPrompt() const`
    - [x] `bool CanInteract(AActor* Interactor) const`
    - [x] `float GetInteractionRange() const`
    - [x] `int32 GetInteractionPriority() const`
  - [x] Confirm `UINTERFACE(Blueprintable)` on `UIFCInteractable` and that Blueprints can implement it
  - [x] In a Blueprint (e.g., `BP_MenuDoor`), open Class Settings → Interfaces and verify `IFCInteractable` appears and can be added
  - [x] Compile
- [x] **Testing After Step 6.1.1**
  - [x] PIE starts successfully without crashes
  - [x] Interface visible in Blueprint class settings "Implemented Interfaces" list
  - [x] No compile errors in Blueprint editor
  - [x] Main menu still functions correctly
  - [x] Can walk around office with WASD + mouse

##### Step 6.1.2: Verify UFCInteractionComponent and Input Routing

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Implementation (Character & Controller)**

  - [x] Confirm `UFCInteractionComponent` is attached to `AFCFirstPersonCharacter` and initialized (see Technical_Documentation.md → Office Flow & Interactions)
  - [x] Verify key properties on the component:
    - [x] `InteractionTraceDistance` (e.g., 500.0f)
    - [x] `InteractionCheckFrequency` (e.g., 0.1f)
    - [x] `bShowDebugTrace` (enable temporarily for debugging)
  - [x] Verify input routing: `AFCPlayerController::HandleInteractPressed()` delegates to the character’s `InteractionComponent->Interact()`
  - [x] Ensure the component performs the line trace and calls `IIFCInteractable::Execute_OnInteract(...)` on the focused actor
  - [x] Compile successfully (Live Coding or manual rebuild)

- [x] **Testing After Step 6.1.2**
  - [x] PIE starts successfully without crashes
  - [x] Main menu appears correctly
  - [x] "New Legacy" transitions to gameplay
  - [x] Can walk around office with WASD + mouse
  - [x] Press E (Interact key) - no crashes (nothing implements interface yet)
  - [x] No errors in Output Log
  - [x] No "Accessed None" Blueprint errors

---

#### Step 6.2: Implement Table View Interaction

##### Step 6.2.1: Add IFCInteractable to BP_OfficeDesk

- [x] **Analysis**

  - [x] Verify `BP_OfficeDesk` exists at `/Game/FC/World/Blueprints/BP_OfficeDesk`
  - [x] Confirm it has `CameraTargetPoint` SceneComponent (already present)
  - [x] Check Technical_Documentation.md §4.3 for current setup

- [x] **Implementation**
  - [x] Open Blueprint: `/Content/FC/World/Blueprints/BP_OfficeDesk.uasset`
  - [x] Add Interface: Class Settings → Interfaces → Click "Add" → Select `IFCInteractable` (C++ interface, Blueprint-implementable)
  - [x] Implement `OnInteract` event:
    - [x] Event Graph → Right-click → "Add Event" → "Event On Interact"
    - [x] Cast `Interactor` to `AFCFirstPersonCharacter`
    - [x] Get Controller → Cast to `AFCPlayerController`
    - [x] Call `SetCameraMode(TableView)`
  - [x] Implement `GetInteractionPrompt` function:
    - [x] Event Graph → Right-click → "Add Function" → "Get Interaction Prompt"
    - [x] Return FText: "Press E to view table"
  - [x] Implement `CanInteract` function:
    - [x] Event Graph → Right-click → "Add Function" → "Can Interact"
    - [x] Return Value: true (always allow interaction)
  - [x] Compile and save Blueprint
- [x] **Verify Placement**
  - [x] Open L_Office level
  - [x] Confirm `BP_OfficeDesk` is placed (already present from Task 4)
  - [x] Verify collision is enabled on Static Mesh Component
  - [x] Check distance from player spawn (within ~500 units)
  - [x] Save level if changes made
- [x] **Testing After Step 6.2.1**
  - [x] PIE starts successfully
  - [x] Walk up to desk
  - [x] Interaction prompt appears: "Press E to view table"
  - [x] Press E → camera blends to table view using CameraTargetPoint (smooth 2s transition)
  - [x] Movement inputs disabled in table view
  - [x] ESC currently returns to main menu (expected - Step 6.3 will fix)
  - [x] No crashes or errors
  - [x] Check Output Log for interaction logs (LogFCInteraction category)

##### Step 6.2.2: Add bIsInTableView State to PlayerController

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Implementation (FCPlayerController.h)**
  - [x] Add protected property:
    ```cpp
    protected:
        /** Tracks whether player is currently in table view mode */
        UPROPERTY(BlueprintReadOnly, Category = "Camera")
        bool bIsInTableView = false;
    ```
  - [x] Update `SetCameraMode()` signature (renamed from `SetFallenCompassCameraMode()`)
- [x] **Implementation (FCPlayerController.cpp)**

  - [x] Created `SetCameraModeLocal()` helper method with smooth camera blending logic
  - [x] Implemented table view camera system:
    - [x] Spawns temporary `ACameraActor` at `BP_OfficeDesk::CameraTargetPoint` transform
    - [x] Uses component world transform (location + rotation) for accurate camera positioning
    - [x] 2-second cubic blend to table view camera
    - [x] Switches input mapping from `FirstPersonMappingContext` to `StaticSceneMappingContext`
    - [x] `StaticSceneMappingContext` allows UI inputs (ESC) but blocks movement (WASD, mouse look)
  - [x] Implemented first-person return from table view:
    - [x] Blends back to pawn camera over 2 seconds (cubic interpolation)
    - [x] Delays cleanup of temporary camera actor until after blend completes (BlendTime + 0.1s)
    - [x] Restores `FirstPersonMappingContext` immediately for responsive controls
    - [x] Uses `ClearAllMappings()` before adding context to ensure clean switching
  - [x] Compile successfully

- [x] **Testing After Step 6.2.2**
  - [x] PIE starts successfully
  - [x] Interact with table → camera blends smoothly to table view (2s cubic)
  - [x] Camera rotation correctly matches CameraTargetPoint orientation
  - [x] WASD keys disabled (no movement)
  - [x] Mouse look disabled
  - [x] ESC transitions smoothly back to first-person (2s cubic blend)
  - [x] Movement restored immediately after ESC press
  - [x] No crashes or errors

**Implementation Details:**

- **Camera System**: Uses temporary `ACameraActor` spawned at runtime to avoid Blueprint dependencies
- **Input Mapping Strategy**: Three contexts used:
  - `FirstPersonMappingContext`: Full movement + ESC (gameplay)
  - `StaticSceneMappingContext`: ESC only (table view, cutscenes)
  - `MainMenuMappingContext`: UI-only (menu state)
- **Blend Timing**: Consistent 2.0s cubic blend for all camera transitions (smooth, cinematic feel)
- **Cleanup Strategy**: Timer-delayed actor destruction prevents blend interruption

---

#### Step 6.3: Implement ESC from Table View

- [x] **Analysis**

  - [x] What implementations already exist: Checked technical documentation and project file system

- [x] **Implementation - Input Consolidation**

  - [x] Consolidated ESC key handling to single `IA_Escape` input action
  - [x] Removed duplicate `IA_Pause` action (was conflicting with PIE shutdown)
  - [x] Updated `FCPlayerController`:
    - [x] Removed `PauseAction` property
    - [x] Removed `UIEscapeAction` property
    - [x] Added single `EscapeAction` property (loads `/Game/FC/Input/IA_Escape`)
  - [x] Configured `IMC_FC_StaticScene` mapping context:
    - [x] Added `IA_Escape` → Keyboard Escape key
    - [x] Removed `IA_Pause` from this context (unified ESC handling)
    - [x] Verified no movement inputs (WASD, Mouse) are mapped

- [x] **Implementation - Context-Aware ESC Handler (FCPlayerController.cpp)**

  - [x] Rewrote `HandlePausePressed()` with state awareness:

    ```cpp
    void AFCPlayerController::HandlePausePressed()
    {
        // If in table view, ESC exits back to first-person
        if (CameraMode == EFCPlayerCameraMode::TableView)
        {
            SetCameraModeLocal(EFCPlayerCameraMode::FirstPerson, 2.0f);
            return;
        }

        // If in gameplay state, ESC toggles pause menu
        if (CurrentGameState == EFCGameState::Gameplay)
        {
            UFCUIManager* UIManager = ...;
            if (bIsPauseMenuDisplayed)
            {
                UIManager->HidePauseMenu();
                bIsPauseMenuDisplayed = false;
            }
            else
            {
                UIManager->ShowPauseMenu();
                bIsPauseMenuDisplayed = true;
            }
            return;
        }

        // In main menu or other UI states, ESC does nothing
    }
    ```

  - [x] Updated `SetCameraModeLocal()` FirstPerson case:
    - [x] Uses `ClearAllMappings()` + `AddMappingContext(FirstPersonMappingContext)` for clean switching
    - [x] Delays temporary camera cleanup until after blend (BlendTime + 0.1s timer)
    - [x] Restores input immediately for responsive controls
  - [x] Compile successfully

- [x] **Implementation - PIE Shutdown Key Rebinding**

  - [x] Manually rebound PIE "Stop Play Session" from ESC to F10 in Editor Preferences
  - [x] This allows ESC to be used for in-game functionality without quitting PIE

- [x] **Testing After Step 6.3**
  - [x] PIE starts successfully
  - [x] Interact with table → camera blends to table view (2s smooth)
  - [x] Press ESC → camera smoothly blends back to first-person (2s)
  - [x] Movement and mouse look re-enabled immediately
  - [x] Press ESC from first-person → pause menu appears (Step 6.4)
  - [x] ESC toggles pause menu on/off correctly
  - [x] No crashes or errors

**Technical Notes:**

- **Input Action Consolidation**: Single `IA_Escape` replaces `IA_Pause` + `IA_UIEscape` for cleaner input architecture
- **State-Driven Behavior**: `HandlePausePressed()` checks `CameraMode` and `CurrentGameState` to determine ESC action
- **Mapping Context Strategy**:
  - `FirstPersonMappingContext`: Full controls (WASD, mouse, ESC, interact, F5/F9)
  - `StaticSceneMappingContext`: ESC only (blocks movement during table view)
  - `MainMenuMappingContext`: UI-only (menu state)
- **PIE Workflow**: ESC now fully dedicated to game logic; F10 stops PIE session

---

#### Step 6.4: Implement Pause Menu from First-Person

##### Step 6.4.1: Create WBP_PauseMenu Widget

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Implementation**
  - [x] Create UMG widget at `/Game/FC/UI/Menus/WBP_PauseMenu`
  - [x] Add Canvas Panel with centered Vertical Box
  - [x] Add semi-transparent background overlay (50% black)
  - [x] Add title text: "Paused" (large font, white)
  - [x] Add vertical button container with buttons:
    - [x] "Resume" → calls `AFCPlayerController::ResumeGame()`
    - [x] "Settings" → placeholder (disabled for now)
    - [x] "Back to Main Menu" → calls `AFCPlayerController::ReturnToMainMenu()`
    - [x] "Quit to Desktop" → calls `AFCPlayerController::QuitGame()`
  - [x] Apply button hover effects (scale 1.1, highlight color)
  - [x] Compile and save widget
- [x] **Blueprint Configuration (BP_FC_GameInstance)**
  - [x] Add variable `PauseMenuClass` (TSubclassOf<UUserWidget>)
  - [x] Set default value to `WBP_PauseMenu`
  - [x] Compile Blueprint
- [x] **Testing After Step 6.4.1**
  - [x] Widget compiles without errors
  - [x] Buttons visible and styled correctly in widget preview
  - [x] GameInstance Blueprint compiles successfully
  - [x] PIE starts without crashes

##### Step 6.4.2: Implement Pause Logic in UIManager

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Implementation (FCUIManager.h)**

  - [x] Add public methods:

    ```cpp
    public:
        /** Shows pause menu and pauses game */
        UFUNCTION(BlueprintCallable, Category = "UI")
        void ShowPauseMenu();

        /** Hides pause menu and resumes game */
        UFUNCTION(BlueprintCallable, Category = "UI")
        void HidePauseMenu();

    protected:
        /** Cached pause menu widget instance */
        UPROPERTY()
        UUserWidget* PauseMenuWidget = nullptr;
    ```

- [x] **Implementation (FCUIManager.cpp)**

  - [x] Implement `ShowPauseMenu()`
  - [x] Implement `HidePauseMenu()`
  - [x] Compile successfully

- [x] **Testing After Step 6.4.2**
  - [x] Compile succeeds without errors
  - [x] PIE starts successfully
  - [x] Can walk around office

##### Step 6.4.3: Connect Pause Menu to PlayerController

- [x] **Implementation**

  - [x] Added `ResumeGame()` method to `AFCPlayerController`
  - [x] Updated `HandlePausePressed()` to call `UIManager->ShowPauseMenu()` / `HidePauseMenu()`
  - [x] Updated `ReturnToMainMenu()` to hide pause menu before transition
  - [x] Fixed timer issue: pause menu now unpauses before setting level reload timer

- [x] **Testing After Step 6.4.3** ✅ CRITICAL TEST POINT
  - [x] PIE starts successfully
  - [x] Walk around in first-person
  - [x] Press ESC → pause menu appears, game pauses, mouse visible
  - [x] Click "Resume" → menu closes, game unpauses, mouse hidden, movement restored
  - [x] Press ESC again → pause menu reappears
  - [x] Press ESC → pause menu appears
  - [x] Click "Back to Main Menu" → fades to black, returns to menu
  - [x] Click "Quit to Desktop" → game quits
  - [x] No crashes or errors

---

##### Step 6.4.4: Implement Quit to Desktop (COMPLETED - Already working)

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Implementation (FCPlayerController.h)**
  - [x] Add public method:
    ```cpp
    /** Quits the game application */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void QuitGame();
    ```
- [x] **Implementation (FCPlayerController.cpp)**
  - [x] Implement `QuitGame()`:
    ```cpp
    void AFCPlayerController::QuitGame()
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
    }
    ```
- [x] **Blueprint Configuration (WBP_PauseMenu)**
  - [x] Update "Quit to Desktop" button binding:
    - [x] On Clicked → Get Player Controller → Cast to FC Player Controller → Call `QuitGame()`
  - [x] Compile and save widget
- [x] **Testing After Step 6.4.4**
  - [x] PIE starts successfully
  - [x] Open pause menu (ESC from gameplay)
  - [x] Click "Quit to Desktop" → PIE session ends cleanly (or editor prompts to stop)
  - [x] No crashes or errors during shutdown

---

#### Step 6.5: Door Interaction Implementation (Already Complete)

- [x] **Status**: Door interaction already implemented in Task 5.5
  - [x] `BP_OfficeDoor` implements `IFCInteractable`
  - [x] Calls `AFCPlayerController::ReturnToMainMenu()`
  - [x] Fades to black and reloads L_Office in menu state
  - [x] No changes needed

---

#### Step 6.6: State Sanity Checks and Final Verification

- [x] **Full Flow Testing**
  - [x] Start game → Main menu appears
  - [x] Click "New Legacy" → Office gameplay
  - [x] Walk to table → Press E → Table view
  - [x] Press ESC → First-person restored
  - [x] Press ESC → Pause menu
  - [x] Click "Resume" → Gameplay restored
  - [x] Press ESC → Pause menu
  - [x] Click "Back to Main Menu" → Fade → Menu state
  - [x] Click "New Legacy" → Gameplay
  - [x] Press ESC → Pause menu
  - [x] Click "Quit to Desktop" → Clean exit
- [x] **Edge Case Testing**
  - [x] Rapid ESC presses don't break state
  - [x] Pause menu buttons don't respond to multiple clicks (idempotent)
  - [x] Table interaction while paused → no effect (expected)
  - [x] Door interaction from table view → works correctly
- [x] **Code Conventions Compliance Check**
  - [x] All `UObject*` pointers use `UPROPERTY()` for GC tracking ✅
  - [x] PlayerController delegates to UIManager (separation of concerns) ✅
  - [x] Interaction uses `IFCInteractable` C++ interface (BlueprintNativeEvent) and `UFCInteractionComponent` (loose coupling) ✅
  - [x] State changes logged for debugging
  - [x] No excessive Tick usage (event-driven design) ✅

**COMMIT POINT 6.6**: `git add -A && git commit -m "test: Verify all office flow states and transitions"`

---

### 7.0 Verify Week 1 acceptance criteria and clean up repository

> **Purpose**: Ensure all Week 1 requirements met per GDD/DRM, verify code conventions compliance, and prepare repository for Week 2 development.

---

#### Step 7.1: Comprehensive Functional Playthrough

##### Step 7.1.1: Main Menu Flow Verification

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Test Sequence**
  - [x] Launch PIE → L_Office loads in menu state
  - [x] Verify menu camera (static desk view, slow oscillation)
  - [x] Verify atmospheric effects (dust particles, rain audio)
  - [x] Verify UI visibility (logo, buttons, version text)
  - [x] Test all buttons:
    - [x] "New Legacy" → transitions to gameplay ✅
    - [x] "Continue" → disabled if no saves, loads most recent save if exists ✅
    - [x] "Load Game" → opens save slot selector, back button returns to menu ✅
    - [x] "Options" → placeholder (no action expected) ✅
    - [x] "Quit" → exits application cleanly ✅
  - [x] Verify no crashes, errors, or Blueprint warnings

##### Step 7.1.2: Gameplay Flow Verification

- [x] **Analysis**

  - [x] What implementations already exist: Check technical documentation and project file system

- [x] **Test Sequence**
  - [x] From menu, click "New Legacy"
  - [x] Camera blends to first-person (2s smooth transition) ✅
  - [x] Player spawns at gameplay start position ✅
  - [x] Movement controls:
    - [x] WASD → character moves ✅
    - [x] Mouse → camera look (yaw/pitch) ✅
    - [x] Character collides with walls, floor, props ✅
  - [x] Interaction system:
    - [x] Walk to table → Press E → camera blends to table view ✅
    - [x] Movement disabled in table view ✅
    - [x] Press ESC → camera blends back to first-person ✅
    - [x] Movement restored ✅
  - [x] Pause menu:
    - [x] Press ESC from first-person → pause menu appears ✅
    - [x] Game paused, mouse visible ✅
    - [x] "Resume" → gameplay restored ✅
    - [x] Press ESC again → pause menu reappears ✅
  - [x] Door interaction:
    - [x] Walk to door → Press E → fades to black, returns to menu ✅
  - [x] Verify no crashes, errors, or "Accessed None" warnings

##### Step 7.1.3: Save/Load System Verification

- [ ] **Test Sequence**
  - [x] From gameplay, press F6 (QuickSave)
  - [x] Check Output Log for "SaveGame: Successfully saved to QuickSave" ✅
  - [x] Walk to different location in office
  - [x] Return to menu (door interaction)
  - [x] Click "Continue" → loads at saved position ✅
  - [x] Verify player location/rotation matches save ✅
  - [x] Press F6 → QuickSave at new location
  - [x] Return to menu
  - [x] Click "Load Game" → save slot selector appears ✅
  - [x] Select QuickSave slot → loads correctly ✅
  - [x] Verify transitions (fade handling, no flicker)
  - [x] Verify no save/load errors in Output Log

##### Step 7.1.4: Edge Case and Regression Testing

- [x] **Test Sequence**
  - [x] Rapid button clicks (menu) → no state corruption ✅
  - [x] Spam ESC key → state transitions correctly ✅
  - [x] Interact with table from pause menu → no effect (expected) ✅
  - [x] Save in table view → save succeeds, position is first-person location ✅
  - [x] Load save while in pause menu → works correctly ✅
  - [x] Door interaction from table view → works correctly ✅
  - [x] Multiple level cycles (Menu → Gameplay → Menu → Gameplay) → no memory leaks or degradation ✅
  - [x] Verify no "Accessed None" errors throughout all tests

**FINDINGS DOCUMENTATION**: Record any discovered issues in "Known Issues & Backlog" section

---

#### Step 7.2: Technical Architecture Verification

##### Step 7.2.1: Core Framework Classes Verification

- [x] **Check L_Office World Settings**

  - [x] GameMode Override: `BP_FC_GameMode` ✅
  - [x] World Settings → Game Mode → Default Pawn Class: `BP_FC_FirstPersonCharacter` ✅
  - [x] World Settings → Game Mode → Player Controller Class: `BP_FC_PlayerController` ✅

- [x] **Check Project Settings**

  - [x] Maps & Modes → Game Instance Class: `BP_FC_GameInstance` ✅
  - [x] Maps & Modes → Editor Startup Map: `L_Office` ✅
  - [x] Maps & Modes → Game Default Map: `L_Office` ✅

- [x] **Verify Subsystem Initialization**
  - [x] Launch PIE, check Output Log for:
    - [x] `FCLevelManager: Subsystem initialized` ✅
    - [x] `FCUIManager: Subsystem initialized` ✅
    - [x] `FCTransitionManager: Subsystem initialized` ✅
  - [x] No "Failed to initialize" errors

##### Step 7.2.2: Code Conventions Compliance Audit

- [x] **UE_CodeConventions.md Compliance Check**
  - [x] **Encapsulation (§2.1)**:
    - [x] All data members private with public accessors ✅
    - [x] No public member variables except UPROPERTY BlueprintReadWrite where needed ✅
  - [x] **Modular Organization (§2.2)**:
    - [x] Code organized in `/Source/FC/Core`, `/Source/FC/UI` modules ✅
    - [x] Forward declarations used in headers ✅
    - [x] Full includes only in .cpp files ✅
  - [x] **Blueprint Exposure (§2.3)**:
    - [x] Only necessary methods exposed as BlueprintCallable ✅
    - [x] UI callbacks in UIManager, not PlayerController ✅
  - [x] **Memory Management (§2.5)**:
    - [x] All `UObject*` pointers use `UPROPERTY()` for GC tracking ✅
    - [x] Widget outer set to GameInstance (persistent widgets) ✅
    - [x] No dangling pointers or memory leaks ✅
  - [x] **Event-Driven Design (§2.6)**:
    - [x] No excessive Tick usage ✅
    - [x] Timers used for delays (InitializeMainMenu, fade-in) ✅
    - [x] Delegates used for async operations (OnGameLoaded) ✅
  - [x] **Interaction Interface (§3.1)**:
    - [x] `IFCInteractable` (C++ interface, Blueprint-implementable) used for actor interaction ✅
    - [x] No hard casting in interaction system ✅
  - [x] **Separation of Concerns (§4.3)**:
    - [x] PlayerController delegates to UIManager for UI logic ✅
    - [x] LevelManager handles level metadata ✅
    - [x] TransitionManager handles screen transitions ✅

##### Step 7.2.3: Output Log Review

- [x] **Log Analysis**
  - [x] Launch PIE, complete full playthrough (7.1.2)
  - [x] Review Output Log for:
    - [x] No `Error:` messages (except intentional test cases) ✅
    - [x] No `Warning:` messages for missing assets or null pointers ✅
    - [x] Expected logs present:
      - [x] Subsystem initialization messages ✅
      - [x] Camera mode changes (FirstPerson, TableView, MenuCamera) ✅
      - [x] Save/load operations with timestamps ✅
      - [x] Transition manager state changes ✅
  - [x] Document any unexpected warnings for investigation
    - [x] LogTemp: Warning: RestorePlayerPosition: No pending load data (expected if no save exists)
    - [x] LogRenderer: Warning: [VSM] Non-Nanite Marking Job Queue overflow. Performance may be affected. (large shadow map area, not critical for prototype)
    - [x] LogPlayerController: Error: InputMode:UIOnly - Attempting to focus Non-Focusable widget SObjectWidget [Widget.cpp(990)]! (UI focus issue, not blocking)
    - [x] LogFallenCompassPlayerController: Warning: TableView Debug: CameraTargetPoint Rotation/Location/Spawned Camera Rotation (debug output, not critical)

---

#### Step 7.3: Naming Conventions Compliance

##### Step 7.3.1: C++ Class Naming Verification

- [x] **Check Against UE_NamingConventions.md**
  - [x] All classes use `FC` prefix ✅
  - [x] Core classes:
    - [x] `UFCGameInstance` (U prefix for UObject-derived) ✅
    - [x] `AFCGameMode` (A prefix for Actor-derived) ✅
    - [x] `AFCPlayerController` (A prefix) ✅
    - [x] `AFCFirstPersonCharacter` (A prefix) ✅
  - [x] Subsystems:
    - [x] `UFCLevelManager` (UGameInstanceSubsystem) ✅
    - [x] `UFCUIManager` (UGameInstanceSubsystem) ✅
    - [x] `UFCTransitionManager` (UGameInstanceSubsystem) ✅
  - [x] UI widgets:
    - [x] `UFCScreenTransitionWidget` (UUserWidget) ✅
  - [x] SaveGame:
    - [x] `UFCSaveGame` (USaveGame) ✅

##### Step 7.3.2: Blueprint Naming Verification

- [x] **Check Against UE_NamingConventions.md**
  - [x] Blueprints use `BP_` prefix ✅
  - [x] Core Blueprints:
    - [x] `BP_FC_GameInstance` ✅
    - [x] `BP_FC_GameMode` ✅
    - [x] `BP_FC_PlayerController` ✅
    - [x] `BP_FC_FirstPersonCharacter` ✅
  - [x] Props:
    - [x] `BP_OfficeDesk` ✅
    - [x] `BP_OfficeDoor` ✅
  - [x] UI widgets:
    - [x] `WBP_MainMenu` (W prefix for widgets) ✅
    - [x] `WBP_SaveSlotSelector` ✅
    - [x] `WBP_SaveSlotItem` ✅
    - [x] `WBP_PauseMenu` ✅
  - [x] Interfaces:
    - [x] `IFCInteractable` (C++ interface, Blueprint-implementable) ✅

##### Step 7.3.3: Folder Structure Verification

- [x] **Check Against UE_NamingConventions.md**
  - [x] Source code structure:
    - [x] `/Source/FC/` (main module) ✅
    - [x] `/Source/FC/Core/` (core classes) ✅
    - [x] `/Source/FC/UI/` (UI widgets) ✅
  - [x] Content structure:
    - [x] `/Content/FC/` (all project content) ✅
    - [x] `/Content/FC/Blueprints/` ✅
    - [x] `/Content/FC/UI/` ✅
    - [x] `/Content/FC/Maps/` (L_Office level) ✅
  - [x] No loose files in `/Content/` root ✅

---

#### Step 7.4: Git Repository Hygiene

##### Step 7.4.1: Working Tree Cleanup

- [ ] **Verify Clean State**

  - [ ] From repo root, run: `cd W:\\GameDev\\FallenCompass; git status`
  - [ ] Check for untracked files:
    - [ ] No loose `.uasset` or `.umap` files in unexpected locations ✅
    - [ ] No editor temporary files (`.tmp`, `.log` outside `/Saved`) ✅
    - [ ] No build artifacts outside `/Binaries`, `/Intermediate` ✅
  - [ ] Check modified files:
    - [ ] Only intentional changes staged ✅
    - [ ] No accidental modifications to engine content ✅

- [ ] **Stage All Week 1 Changes**

  - [ ] Run: `git add Source/ Content/ Config/ Docs/`
  - [ ] Run: `git status` → verify all intended changes staged
  - [ ] Commit: `git commit -m "feat: Complete Week 1 - Office prototype, menu system, save/load, table view, pause menu"`

- [ ] **Create Milestone Tag**

  - [ ] Run: `git tag -a milestone-week1 -m "Week 1 Complete: Core framework, office level, diegetic menu, interaction system"`
  - [ ] Run: `git push origin master --tags`

- [ ] **Update Development Roadmap**

  - [ ] Open `/Docs/Fallen_Compass_DRM.md`
  - [ ] Mark Week 1 as complete:

    ```markdown
    ## Week 1 – Project Skeleton & First-Person Office Basics ✅ COMPLETE

    - Core framework (GameInstance, GameMode, PlayerController, Character)
    - Diegetic menu system in L_Office
    - Save/load system with slot selector
    - Table view interaction with ESC return
    - Pause menu with resume/quit functionality
    - Interface-based interaction system (C++ interface + BP implementations)
    ```

  - [ ] Commit: `git commit -am "docs: Mark Week 1 milestone complete in DRM"`

---

#### Step 7.5: Week 1 Acceptance Criteria Checklist

- [ ] **GDD Requirements (Fallen_Compass_GDD_v0.2.md)**

  - [ ] Office level exists with first-person navigation ✅
  - [ ] Table interaction for strategy view (foundation complete) ✅
  - [ ] Door returns to main menu ✅
  - [ ] Atmospheric diegetic menu (no separate menu level) ✅

- [ ] **DRM Week 1 Goals (Fallen_Compass_DRM.md)**

  - [ ] UE 5.7 C++ project created with FC prefix ✅
  - [ ] Git repository initialized with proper .gitignore ✅
  - [ ] Core framework classes (GameInstance, GameMode, PlayerController, Character) ✅
  - [ ] First-person character with WASD + mouse look ✅
  - [ ] Input mappings (Move, Look, Interact, Pause) ✅
  - [ ] Greybox office level with table and door ✅
  - [ ] In-world menu system (camera, UI, state management) ✅
  - [ ] Save/load system foundation ✅

- [ ] **Code Quality Standards**

  - [ ] All code follows UE_CodeConventions.md ✅
  - [ ] All names follow UE_NamingConventions.md ✅
  - [ ] No critical errors or warnings in Output Log ✅
  - [ ] Full regression testing passed (7.1.1-7.1.4) ✅
  - [ ] Architecture documented in Technical_Documentation.md ✅

- [ ] **Repository State**
  - [ ] Working tree clean (no untracked garbage) ✅
  - [ ] All changes committed with descriptive messages ✅
  - [ ] Week 1 milestone tag created ✅
  - [ ] Documentation updated ✅

**FINAL COMMIT**: `git commit -am "chore: Week 1 acceptance criteria verified and complete"`

---

### Week 1 Complete ✅

**Next Steps**: Proceed to Week 2 development (Expedition system, Strategy map, Multi-scene management)

---

### 🐛 Known Issues & Backlog

#### Visual Polish

- [ ] **BUG: Camera flicker during save game load transitions**
  - **Symptom**: Brief flicker/flash visible when camera blends to first-person after loading a save game
  - **Occurs**: When clicking "Continue" or loading from save slot selector
  - **Context**:
    - Fade-in starts at 0.5s delay (when `PendingLoadData` exists)
    - Camera blend to first-person starts via `TransitionToGameplay()` (2.0s blend)
    - Timing overlap between fade-in (1.0s) and camera blend (2.0s) causes brief visibility conflict
  - **Current Impact**: Minor visual artifact, does not affect functionality
  - **Priority**: Low (polish issue, not a blocker)
  - **Potential Solutions**:
    - Option A: Synchronize fade-in timing with camera blend completion (delay fade-in until blend starts)
    - Option B: Adjust fade-in duration to complete before camera movement is visible
    - Option C: Use camera cut instead of blend for save loads (instant transition)
  - **Deferred**: Non-blocking issue, will address during visual polish phase

#### Save System

- [ ] **UI: Save slot selector only shows QuickSave slot**
  - **Symptom**: `WBP_SaveSlotSelector` widget displays only one save slot even when multiple saves exist
  - **Logs Confirm**: C++ correctly finds saves (`GetAvailableSaveSlots: Total found: 1` for test case with only QuickSave)
  - **Root Cause**: Blueprint widget `WBP_SaveSlotSelector` may not be iterating through all returned save slots
  - **Current Impact**: Cannot view/select multiple manual saves from UI (though only QuickSave exists in current test)
  - **Priority**: Medium (will become blocker when implementing manual save feature)
  - **Investigation Needed**:
    - Check `WBP_SaveSlotSelector` Blueprint logic for save slot list population
    - Verify ScrollBox binding and item template instantiation
    - Create multiple test saves to confirm C++ returns all slots correctly
  - **Deferred**: Current workflow only uses QuickSave, will fix when implementing manual save UI
