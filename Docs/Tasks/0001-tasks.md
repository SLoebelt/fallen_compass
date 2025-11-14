## Relevant Files

- `/Docs/UE_NamingConventions.md` - Unreal Engine naming convention document and desired folder structure.
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

- [ ] **3.1 Create first-person character class**

  - [ ] Create `AFCFirstPersonCharacter` derived from `ACharacter`.
  - [ ] Add a camera component as a child of the character (for first-person view).
  - [ ] Optionally add a basic mesh/arms or placeholder component (not required in Week 1).

- [ ] **3.2 Configure input mappings**

  - [ ] In Project Settings → Input, configure:
    - [ ] Axis mappings:
      - [ ] `MoveForward` (W = +1, S = -1).
      - [ ] `MoveRight` (D = +1, A = -1).
      - [ ] `Turn` (Mouse X).
      - [ ] `LookUp` (Mouse Y).
    - [ ] Action mappings:
      - [ ] `Interact` (e.g. E key).
      - [ ] `Pause` (ESC key).
  - [ ] Save and verify no Input mapping conflicts.

- [ ] **3.3 Implement movement & look in character**

  - [ ] In `AFCFirstPersonCharacter`, bind Axis and Action mappings in `SetupPlayerInputComponent`.
  - [ ] Implement:
    - [ ] Forward/backward movement for `MoveForward`.
    - [ ] Right/left movement for `MoveRight`.
    - [ ] Yaw rotation for `Turn`.
    - [ ] Pitch rotation for `LookUp`.
  - [ ] Ensure the camera rotates with the character and feels like a standard FPS.

- [ ] **3.4 Connect character to GameMode**

  - [ ] Set `AFCFirstPersonCharacter` as the **Default Pawn Class** in `AFCGameMode`.
  - [ ] Launch PIE and confirm that the player spawns as `AFCFirstPersonCharacter`.

- [ ] **3.5 Expose hooks for interaction and ESC**
  - [ ] In `AFCFirstPersonCharacter` or `AFCPlayerController`:
    - [ ] Bind `Interact` action to a function that will later call interactables (e.g. trace forward).
    - [ ] Bind `Pause` action (ESC) to a function that will delegate pause/menu handling (eventually to controller/UI).

---

### 4.0 Create greybox Office level with spawn, table & door interactables, and camera positions

- [ ] **4.1 Create Office level**

  - [ ] Create a new level asset (e.g. `/Content/Levels/L_Office`).
  - [ ] Assign `AFCGameMode` as the GameMode for this level (World Settings).
  - [ ] Place a `PlayerStart` at a reasonable position in the room.

- [ ] **4.2 Build basic greybox geometry**

  - [ ] Add static meshes or BSP/geometry for:
    - [ ] Floor.
    - [ ] Four walls.
    - [ ] Ceiling.
  - [ ] Confirm that collision is active to prevent falling out.

- [ ] **4.3 Add placeholder table prop**

  - [ ] Add a mesh (e.g. a cube or simple table asset) at a logical position in the room.
  - [ ] Name it clearly (e.g. `SM_Table_Office` or `BP_OfficeTable`).
  - [ ] Ensure it’s reachable by walking.

- [ ] **4.4 Add placeholder door prop**

  - [ ] Add a door mesh or placeholder rectangle to represent the exit door.
  - [ ] Name it clearly (e.g. `SM_OfficeDoor` or `BP_OfficeDoor`).
  - [ ] Place it on one wall in a position that is clearly an “exit”.

- [ ] **4.5 Set up camera positions for table view**

  - [ ] Create a target transform for the **table-focused camera view**:
    - [ ] Either place a `Camera` actor pointing at the table, or define a `SceneComponent` on the table blueprint that acts as camera target.
  - [ ] Verify that from this transform, the table is clearly visible and framed.

- [ ] **4.6 Basic lighting & navigation**
  - [ ] Add at least one light so the room is visible.
  - [ ] Ensure NavMesh (if used) or character movement navigates correctly in the room.
  - [ ] Quick test: run PIE, walk around, ensure scale and movement feel okay.

---

### 5.0 Implement dedicated Main Menu level with Start & Quit

- [ ] **5.1 Create Main Menu level**

  - [ ] Create a new level (e.g. `/Content/Levels/L_MainMenu`).
  - [ ] Use a simple background (e.g. static camera with basic environment or flat color).
  - [ ] Set this level as the **Editor Startup Map** and **Game Default Map** in Project Settings → Maps & Modes.

- [ ] **5.2 Create Main Menu widget**

  - [ ] Create a UMG widget (e.g. `WBP_MainMenu`).
  - [ ] Add buttons:
    - [ ] “Start Game”
    - [ ] “Quit Game”
  - [ ] Optionally add a simple title text.

- [ ] **5.3 Hook Main Menu widget into level**

  - [ ] In the Main Menu level blueprint (or a custom menu GameMode/Controller if desired):
    - [ ] On level begin play, create and add `WBP_MainMenu` to the viewport.
  - [ ] Implement button behavior:
    - [ ] “Start Game” → open Office level (`L_Office`).
    - [ ] “Quit Game” → call `QuitGame`.

- [ ] **5.4 Verify menu → Office flow**
  - [ ] Start game from UE.
  - [ ] Confirm Main Menu level loads.
  - [ ] Click “Start Game”:
    - [ ] Office level loads.
    - [ ] Player spawns as `AFCFirstPersonCharacter`.

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

- [ ] **6.5 Implement door interaction → Main Menu**

  - [ ] Create a blueprint for the door (e.g. `BP_OfficeDoor`) implementing `Interact`.
  - [ ] On `Interact` from first-person:
    - [ ] Load the Main Menu level (`L_MainMenu`).
    - [ ] Optionally ensure the game is unpaused and in a clean state.

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
