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

  - [x] Create a new Unreal Engine 5.7 **C++** project (e.g. “FallenCompass” with FC prefix).
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
    - [x] Short project description (“Fallen Compass”).
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
  - [x] Add basic logging in `BeginPlay()` to confirm it’s being used.

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
    - [x] Floor.
    - [x] Four walls.
    - [x] Ceiling.
  - [x] Confirm that collision is active to prevent falling out.

- [x] **4.3 Add placeholder table prop**

  - [x] Add a mesh (e.g. a cube or simple table asset) at a logical position in the room.
  - [x] Name it clearly (e.g. `SM_Table_Office` or `BP_OfficeTable`).
  - [x] Ensure it’s reachable by walking.

- [x] **4.4 Add placeholder door prop**

  - [x] Add a door mesh or placeholder rectangle to represent the exit door.
  - [x] Name it clearly (e.g. `SM_OfficeDoor` or `BP_OfficeDoor`).
  - [x] Place it on one wall in a position that is clearly an “exit”.

- [x] **4.5 Set up camera positions for table view**

  - [x] Create a target transform for the **table-focused camera view**:
    - [x] Either place a `Camera` actor pointing at the table, or define a `SceneComponent` on the table blueprint that acts as camera target.
  - [x] Verify that from this transform, the table is clearly visible and framed.

- [x] **4.6 Basic lighting & navigation**
  - [x] Add at least one light so the room is visible (sun light from putdoor for now).
  - [x] Ensure character movement navigates correctly in the room.
  - [x] Quick test: run PIE, walk around, ensure scale and movement feel okay.

---

### 5.0 Implement in-world Main Menu in Office level (Diegetic Menu System)

> **Design:** Menu state within L_Office using static camera on desk + UI overlay. Eliminates separate menu level for immersive experience.  
> **Reference:** See `/Docs/Tasks/0005-MainMenu-Concept.md` for full technical specification.

- [x] **5.1 Set up Menu Camera system**

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
    - [x] Click "Load Save" → opens save slot selector
    - [x] Click save slot → loads into saved position
    - [x] Verify: All UI transitions work correctly

- [x] **5.9 Add atmospheric details and polish**

  - [x] Add ambient effects to menu camera view:
    - [x] Particle system: Dust via Niagara System (`NS_Dust`)
    - [x] Animated material: Candle flicker emissive pulse
    - [x] Dust decals on wall and floor (`BP_DustDecal`)
  - [x] Add audio:
    - [x] Ambient loop: Sound cue with Rain sounds (`Cue_Rain`)
    - ~~Button click sound effects (`S_ButtonClick`)~~ - Deferred to polish phase
    - ~~Menu transition whoosh (`S_MenuTransition`)~~ - Deferred to polish phase
    - ~~Door open sound (`S_DoorOpen`)~~ - Deferred to polish phase
  - ~~Camera enhancements~~ - Deferred to polish phase
    - ~~Depth of field focusing on desk area~~
  - ~~Widget polish~~ - Will be handled by Task 5.11 and 5.12
    - ~~Smooth fade-in on menu appear (1s delay after level load)~~
    - ~~Font selection (period-appropriate serif)~~
    - ~~Drop shadows for text readability~~
  - [x] Test: Menu feels atmospheric but UI remains readable
  - **Status**: Atmospheric effects exceed expectations for current dev phase

- [x] **5.10 Implement modular screen transition system**

  - [x] Create `UFCScreenTransitionWidget` UMG widget:
    - [x] Full-screen black overlay with configurable opacity animation
    - [x] Optional loading spinner/indicator (animated texture or throbber)
    - [x] Fade in/out functions with configurable duration
    - [x] Event dispatcher for transition complete callbacks
  - [x] Implement `UFCTransitionManager` subsystem (UGameInstanceSubsystem):
    - [x] `BeginFadeOut(float Duration, bool bShowLoadingIndicator)` - Start fade to black
    - [x] `BeginFadeIn(float Duration)` - Start fade from black to clear
    - [x] `OnFadeOutComplete` delegate - Triggered when fade out finishes
    - [x] `OnFadeInComplete` delegate - Triggered when fade in finishes
    - [x] Manages single persistent `UFCScreenTransitionWidget` instance
  - [x] Add helper functions to `AFCPlayerController`:
    - [x] `FadeScreenOut(float Duration = 1.0f, bool bShowLoading = false)`
    - [x] `FadeScreenIn(float Duration = 1.0f)`
    - [x] Both delegate to transition manager for actual work
  - [x] Test modular system:
    - [x] Can trigger fade from any context (controller, game mode, etc.)
    - [x] Multiple rapid calls don't break fade state
    - [x] Callbacks fire reliably
    - [x] Loading indicator appears/disappears correctly

- [x] **5.11 Implement smart load transitions based on level context**

  - [x] Add level tracking to `UFCTransitionManager`:
    - [x] Add `FName CurrentLevelName` property to track loaded level
    - [x] Add `IsSameLevelLoad(FName TargetLevelName)` to check if load is within same level
    - [x] Add `UpdateCurrentLevel(FName NewLevelName)` to update tracking after loads
  - [x] Modify `UFCGameInstance::LoadGameAsync()` for smart transitions:
    - [x] Strip UEDPIE prefix from level names for comparison
    - [x] Check if `TargetLevelName == CurrentLevelName` via transition manager
    - [x] **Same Level (Office → Office)**:
      - [x] Use existing smooth camera blend (no fade)
      - [x] Existing 2.1s delayed position restoration
      - [x] No loading screen (instant transition)
    - [x] **Cross Level (Office → Future Level)**:
      - [x] Call `TransitionMgr->BeginFadeOut(1.0f, true)` with loading indicator
      - [x] Store `PendingLevelLoad` in game instance
      - [x] Use timer (1.2s delay) to trigger `UGameplayStatics::OpenLevel()`
      - [x] New level's `BeginPlay` detects gameplay level and sets up input
  - [x] Fix cross-level input restoration:
    - [x] Modified `AFCPlayerController::BeginPlay()` to detect non-office/non-menu levels
    - [x] Automatically sets `CurrentGameState = Gameplay`
    - [x] Applies `FirstPerson` input mapping context
    - [x] Sets `FInputModeGameOnly`, hides cursor
  - [x] Test same-level load:
    - [x] Save in office, load from menu
    - [x] Camera smoothly blends from MenuCamera to FirstPerson ✅
    - [x] Position/rotation restored after 2.1s ✅
    - [x] No jarring cuts or loading screens ✅
  - [x] Test cross-level load:
    - [x] Save in office, continue to test level
    - [x] Fade to black with loading indicator ✅
    - [x] Level loads in background ✅
    - [x] Fade in reveals new level ✅
    - [x] Player has full controls (move/look) ✅
  - **Status**: Smart transitions fully functional

- [ ] **5.12 Implement persistent fade-in on all level loads (DEFERRED - See 5.14)**

  - [ ] ~~Create `UFCMenuFadeController` component~~
  - [ ] ~~Delayed fade-in logic~~
  - **Status**: Deferred to Task 5.14 due to widget lifecycle complexity

- [ ] **5.13 Polish menu transitions and camera work**

  - [ ] ~~Add smooth camera pan when opening save slot selector~~
  - [ ] ~~Implement camera zoom/focus on selected menu area~~
  - **Status**: Awaiting Task 5.14 completion for consistent transition polish

- [ ] **5.14 Implement persistent fade-in on all level loads** ⚠️ **BACKLOG ITEM**

  - [ ] **Problem**: Widget lifecycle prevents persistent black screen across level loads
    - [ ] Widget owned by PlayerController (destroyed during `OpenLevel`)
    - [ ] Subsystem persists but can't maintain widget reference
    - [ ] Widget recreation after level load happens too late (level already visible)
  - [ ] **Requirements**:
    - [ ] Screen should be black before level becomes visible
    - [ ] Fade in should start after level fully loaded and player positioned
    - [ ] Should work for PIE start, cross-level loads, and game start
  - [ ] **Potential Solutions**:
    - [ ] Option A: Widget parented to GameInstance or persistent HUD
    - [ ] Option B: Use engine's post-load callback system
    - [ ] Option C: Level streaming with preview to start fade before switch
    - [ ] Option D: Custom loading screen plugin with persistent layer
  - [ ] **Implementation** (TBD after architecture review):
    - [ ] Research UE5.7 persistent UI patterns
    - [ ] Design widget ownership model that survives level transitions
    - [ ] Implement pre-load fade trigger system
    - [ ] Test across all transition scenarios (PIE, cross-level, same-level)
  - **Status**: Deferred pending architectural design - requires widget persistence solution

- [ ] **5.15 Implement delayed menu fade-in on level start**

  - [ ] Create `UFCMenuFadeController` component:
    - [ ] Attached to `WBP_MainMenu` or controlled by player controller
    - [ ] Manages menu visibility state during initial load
    - [ ] Configurable delay duration (default 1.5s)
  - [ ] Refactor `InitializeMainMenu()`:
    - [ ] Spawn `WBP_MainMenu` widget but set initial opacity to 0
    - [ ] Add widget to viewport (invisible)
    - [ ] Start timer for fade-in delay (1.5-2.0 seconds)
    - [ ] After delay: Animate menu opacity 0 → 1 over 1 second
    - [ ] Use UMG animation or manual lerp in Tick
  - [ ] Implement `WBP_MainMenu` fade animation:
    - [ ] Add UMG animation track "FadeIn"
    - [ ] Animates root panel opacity 0 → 1 over 1s
    - [ ] Triggered by controller after delay timer
  - [ ] Alternative: Use `UFCTransitionManager`:
    - [ ] Create specialized `FadeInWidget(UUserWidget* Widget, float Delay, float Duration)`
    - [ ] More consistent with existing transition system
    - [ ] Reusable for any widget fade-in needs
  - [ ] Test menu fade-in:
    - [ ] Launch game → Office level loads
    - [ ] MenuCamera is active, scene visible
    - [ ] 1.5-2s delay, then menu smoothly fades in
    - [ ] No abrupt UI pop-in
    - [ ] Player can interact after fade completes
  - [ ] Ensure no interference with load transitions:
    - [ ] Loading a save shouldn't trigger menu fade-in
    - [ ] Only trigger on fresh level load (game start or door return)
    - [ ] State flag to distinguish fresh load vs. save load

- [ ] **5.13 Implement Quit button and verify all flows**
  - [ ] Implement `OnQuitClicked()`:
    - [ ] Call `UKismetSystemLibrary::QuitGame()`
    - [ ] Ensure clean shutdown (save auto-save if needed)
  - [ ] Full playthrough test:
    - [ ] Game starts → L_Office menu state
    - [ ] "New Legacy" → gameplay transition works
    - [ ] Door interaction → returns to menu
    - [ ] "Continue" → loads most recent save (if exists)
    - [ ] "Load Save Game" → shows slot selector → loads selected slot
    - [ ] "Options" → placeholder message (implement in Week 2)
    - [ ] "Quit" → exits cleanly
  - [ ] Edge case testing:
    - [ ] Rapid button clicks don't break state
    - [ ] Loading non-existent save shows error
    - [ ] Corrupted save handled gracefully
  - [ ] Set L_Office as Editor Startup Map and Game Default Map

---

### 6.0 Implement in-game Office flow (table camera, ESC behaviour, pause menu, door → Main Menu)

- [ ] **6.1 Implement interaction detection**

  - [ ] Choose an approach (line trace from camera, overlap, etc.).
  - [ ] From `AFCPlayerController` or `AFCFirstPersonCharacter`:
    - [ ] On `Interact` input, perform a short line trace from camera forward.
    - [ ] If hit actor implements an “interactable” interface or specific blueprint class (table/door), call its `Interact` function.
  - [ ] Create a generic `BPI_Interactable` Blueprint Interface or C++ interface if helpful.

- [ ] **6.2 Implement table interaction (camera pan/zoom)**

  - [ ] Create a blueprint for the table (e.g. `BP_OfficeTable`) implementing `Interact`.
  - [ ] On `Interact`:
    - [ ] Notify player/controller to switch to **table view**:
      - [ ] Either blend to a dedicated table camera.
      - [ ] Or move/lerp the current camera to the table target transform.
    - [ ] Disable character movement input while in table view.
    - [ ] Set an internal state (e.g. `bIsInTableView = true`) in controller or character.

- [ ] **6.3 Implement ESC behaviour from table view**

  - [ ] In the ESC/Pause handler:
    - [ ] If `bIsInTableView` is **true**:
      - [ ] Do **not** open the pause menu.
      - [ ] Instead, smoothly return camera to first-person view (original character camera).
      - [ ] Re-enable movement input.
      - [ ] Set `bIsInTableView = false`.
  - [ ] Verify:
    - [ ] Interact with table → camera moves to table view.
    - [ ] ESC → returns to first-person view, no pause menu shown.

- [ ] **6.4 Implement pause menu from first-person**

  - [ ] Create a UMG widget (e.g. `WBP_PauseMenu`) with buttons:
    - [ ] “Resume”
    - [ ] “Back to Main Menu”
    - [ ] “Quit Game”
  - [ ] In controller:
    - [ ] On ESC when **not** in table view and not already paused:
      - [ ] Show `WBP_PauseMenu`.
      - [ ] Set game to paused (`SetPause(true)`).
    - [ ] On “Resume”:
      - [ ] Hide pause menu.
            [ ] Unpause game.
  - [ ] On “Back to Main Menu”:
    - [ ] Unpause game.
    - [ ] Load Main Menu level.
  - [ ] On “Quit Game”:
    - [ ] Call `QuitGame`.

- [x] **6.5 Implement door interaction → Main Menu**

  - [x] Create a blueprint for the door (e.g. `BP_OfficeDoor`) implementing `Interact`.
  - [x] On `Interact` from first-person:
    - [x] Load the Main Menu level (`L_MainMenu`).
    - [x] Optionally ensure the game is unpaused and in a clean state.

- [ ] **6.6 State sanity checks**
  - [ ] Verify transitions:
    - [ ] Table view → ESC → first-person.
    - [ ] First-person → ESC → pause menu.
    - [ ] First-person → Interact with door → Main Menu.
    - [ ] From Main Menu, “Start Game” still works as expected.

---

### 7.0 Verify Week 1 acceptance criteria and clean up repository

- [ ] **7.1 Functional playthrough**

  - [ ] Start game → Main Menu.
  - [ ] Click “Start Game” → Office level with first-person character.
  - [ ] Walk around, check collisions and scale.
  - [ ] Interact with table:
    - [ ] Camera pans/zooms into table view.
    - [ ] ESC → camera returns to first-person; movement restored.
  - [ ] From first-person:
    - [ ] ESC → pause menu appears.
    - [ ] “Resume” → returns to game.
    - [ ] “Back to Main Menu” → returns to Main Menu level.
  - [ ] Interact with door:
    - [ ] Directly returns to Main Menu level.

- [ ] **7.2 Technical checks**

  - [ ] Confirm `UFCGameInstance`, `AFCGameMode`, `AFCPlayerController`, and `AFCFirstPersonCharacter` are used in Office level.
  - [ ] Confirm no critical warnings or errors appear in the log on a normal flow.
  - [ ] Confirm class/folder names follow `/Docs/UE_NamingConventions.md`.

- [ ] **7.3 Git & repo hygiene**

  - [ ] Check `git status` → working tree clean (no untracked garbage).
  - [ ] Review `.gitignore` and adjust if any UE-generated folders slipped in.
  - [ ] Commit Week 1 changes with a clear message (e.g. `feat: week 1 office prototype and menu flow`).

- [ ] **7.4 Tag/mark Week 1 milestone**
  - [ ] Optionally create a Git tag (e.g. `milestone-week1`).
  - [ ] Note in `/Docs/Fallen_Compass_DRM.md` that Week 1 goals are achieved (if you’re keeping a manual log).
