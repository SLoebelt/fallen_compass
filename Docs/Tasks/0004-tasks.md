# Week 4 – Expedition Loop & Bug Fixing

## Task Metadata

```yaml
Task ID: 0004
Sprint/Phase: Week 4
Feature Name: Expedition Loop, Minimap, and Critical Bug Fixes
Dependencies: Task 0003 (Week 3 - Overworld)
Estimated Complexity: Medium
Primary Files Affected:
  - Source/FC/Core/FCPlayerController.h/.cpp
  - Content/FC/UI/Menus/WBP_PauseMenu.uasset
  - Content/FC/UI/HUD/WBP_OverworldMap.uasset (New)
  - Content/FC/UI/Menus/WBP_ExpeditionSummary.uasset (New)
  - Source/FC/Core/UFCExpeditionManager.h/.cpp
```

---

## Overview & Context

### Purpose

This week focuses on closing the gameplay loop by implementing the return transition from Overworld to Office, adding a basic Minimap for orientation, and fixing critical state management bugs identified in Week 3 (Issues #1, #2, #4).

### Architecture Notes

- **State Management**: Ensure `UFCGameStateManager` correctly handles the transition back to `Office_Exploration` without resetting to `MainMenu`.
- **UI Architecture**: Use `UFCUIManager` for the new Minimap and Summary screens.
- **Input Handling**: Fix the ESC key toggle issue by properly handling input consumption in the Pause Menu or Player Controller.

### Reference Documents

- `/Docs/Fallen_Compass_DRM.md` - Week 4 Section
- `/Docs/Tasks/backlog.md` - Issues #1, #2, #4
- `/Docs/Technical_Documentation.md` - Game State & UI Systems

### Scope Limitations & Deferred Requirements

To ensure the critical "Expedition Loop" and bug fixes are delivered within Week 4, the following aspects of the Map System requirements are **simplified or deferred**:

1.  **Dynamic Route Construction**: The requirement to "connect points... to set up a road" implies a dynamic graph/pathfinding system. For Week 4, we will implement **Pre-defined Routes** (DataAssets) that the player selects. The ability to manually build custom routes segment-by-segment is deferred.
2.  **Real-time Fog of War Unveiling**: The map will use a static "Explored/Unexplored" state based on Area data. Real-time clearing of fog as the convoy moves in the Overworld (dynamic texture masking) is deferred.
3.  **Complex Economy Integration**: Resource checks (Money/Proviant) will be implemented, but may rely on **Mock Data** if the persistent Inventory System is not fully integrated yet.

---

## Pre-Implementation Phase

### Step 4.0: Analysis & Discovery

- [x] **Analysis of Existing Implementations**

  - [x] **State Management**: Review `Source/FC/Core/FCGameStateManager.h` and `FCPlayerController.cpp` to understand the current state transition logic causing Issue #1.
  - [x] **UI Input**: Review `WBP_PauseMenu` and `FCPlayerController` input handling to address Issue #4 (ESC toggle).
  - [x] **Map System**: Check if any map assets exist in `Content/FC/UI/` or `Content/FC/Data/`. If not, plan the `UFCWorldMapSubsystem` or DataAssets needed for the complex map requirements.
  - [x] **Interaction System**: Verify if an Interaction Interface exists for the "Return Home" POI action. If not, plan `BPI_Interactable`.

- [x] **Code Conventions Compliance Check**

  - [x] **Encapsulation**: Ensure new Map data structures (Routes, Areas) are properly encapsulated in C++ structs/classes or DataAssets.
  - [x] **Separation of Concerns**: Keep Map logic (Costs, Risks) separate from the UI Widget (`WBP_OverworldMap`). Use a Manager or Subsystem.
  - [x] **Naming**: Ensure new widgets use `WBP_` and C++ classes use `FC` prefix.

- [x] **Implementation Plan**
  - [x] **Task 4.1 (Bugs)**: Implement fixes for Issues #1, #2, #4 using the strategies defined in `backlog.md`.
  - [x] **Task 4.2 (Map)**: Create `UFCMapDataAsset` (for areas/routes), `WBP_WorldMap` (visuals), and logic for cost/risk calculation.
  - [x] **Task 4.3 (Return)**: Create `BP_ExtractionPoint` actor and `WBP_ExpeditionSummary`.

---

## Implementation Phase

### Step 4.1: Critical Bug Fixes (Stability)

#### Step 4.1.1: Fix Issue #1 - Office State Reset

_Goal: Ensure returning to Office preserves `Office_Exploration` state instead of resetting to `MainMenu`._

- [ ] **Modify `AFCPlayerController`**
  - [ ] Open `Source/FC/Core/FCPlayerController.h`.
  - [ ] Add helper method: `void ApplyInputAndCameraModeForState(EFCGameStateID State);`.
  - [ ] Open `Source/FC/Core/FCPlayerController.cpp`.
  - [ ] Implement `ApplyInputAndCameraModeForState` to switch camera/input context based on the passed state (Office vs Overworld).
  - [ ] Modify `InitializeMainMenu()`:
    - [ ] Get `UFCGameStateManager`.
    - [ ] Check `GetCurrentState()`.
    - [ ] **IF** State is `None` or `MainMenu`: Proceed with normal Main Menu setup.
    - [ ] **ELSE**: Call `ApplyInputAndCameraModeForState(CurrentState)` and log that state is being preserved.

#### Step 4.1.2: Fix Issue #4 - ESC Key Toggle

_Goal: Allow ESC key to close the Pause Menu even when the engine is paused._

- [ ] **Modify `WBP_PauseMenu`**
  - [ ] Open `Content/FC/UI/Menus/WBP_PauseMenu.uasset`.
  - [ ] Go to Graph -> Functions -> Override -> `OnKeyDown`.
  - [ ] Implement logic:
    - [ ] Get Key from `InKeyEvent`.
    - [ ] Check if Key == `Escape`.
    - [ ] **True**:
      - [ ] Get Owning Player -> Cast to `AFCPlayerController`.
      - [ ] Call `ResumeGame()`.
      - [ ] Return `Handled`.
    - [ ] **False**: Return `Unhandled`.

#### Step 4.1.3: Fix Issue #2 - Abort Button Visibility

_Goal: Only show "Abort Expedition" when actually in the Overworld._

- [ ] **Modify `WBP_PauseMenu`**
  - [ ] Open `Content/FC/UI/Menus/WBP_PauseMenu.uasset`.
  - [ ] In `Event Construct`:
    - [ ] Get Owning Player World -> Get Map Name.
    - [ ] Check if Map Name contains "L_Overworld".
    - [ ] **True**: Set Abort Button Visibility to `Visible`.
    - [ ] **False**: Set Abort Button Visibility to `Collapsed`.

---

### Step 4.2: World Map & Expedition Planning System

_Goal: Implement a data-driven map system for expedition planning (Office) and orientation (Overworld), following existing subsystem architecture._

#### Step 4.2.1: Map Data Architecture (C++ Structs & DataTable)

- [x] **Create `FFCMapAreaData` struct in `UFCWorldMapManager.h`**

  - [x] Derive from `FTableRowBase` for DataTable support
  - [x] Add properties:
    - [x] `AreaID` (FName) – unique identifier for this area
    - [x] `AreaName` (FText) – display name (localized)
    - [x] `Description` (FText) – lore/flavor text
    - [x] `LevelName` (FName) – corresponding Overworld level (e.g., "L_Overworld_Forest")
    - [x] `GridPosition` (FIntPoint) – position on world map grid (2km × 2km grid)
    - [x] `StartPoints` (TArray<FName>) – list of 3 fixed start point IDs for this area
    - [x] `ExploredSubCells` (TArray<bool>) – **12×12 sub-grid** (144 booleans) for gradual fog of war unveiling (~166m per cell)

- [x] **Create `FFCStartPointData` struct in `UFCWorldMapManager.h`**

  - [x] Derive from `FTableRowBase` for DataTable support
  - [x] Add properties:
    - [x] `StartPointID` (FName) – unique identifier
    - [x] `StartPointName` (FText) – display name (e.g., "Northern Coast", "Mountain Pass")
    - [x] `AreaID` (FName) – parent area reference
    - [x] `RiskLevel` (int32) – event probability multiplier (0-100)
    - [x] `MoneyCost` (int32) – gold required to reach this point
    - [x] `SupplyCost` (int32) – provisions/days required
    - [x] `SplinePoints` (TArray<FVector2D>) – route waypoints from office (bottom center) to start point
    - [x] `Description` (FText) – tactical info for player

- [x] **Create `FFCExpeditionPlanningState` struct in `UFCGameInstance.h`**
  - [x] Add to campaign save data (non-DataTable, runtime state):
    - [x] `SelectedAreaID` (FName)
    - [x] `SelectedStartPointID` (FName)
    - [x] `bPlanningInProgress` (bool) – true if player ESC'd from planning widget
    - [x] Methods: `ClearSelection()`, `IsValid()` (checks if area+start point exist)

#### Step 4.2.2: World Map Subsystem (C++ Manager)

- [ ] **Create `UFCWorldMapManager` subsystem**
  - [ ] Inherit from `UGameInstanceSubsystem` (like `UFCLevelManager`, `UFCUIManager`)
  - [ ] File: `Source/FC/Core/UFCWorldMapManager.h/.cpp`
  - [ ] Add properties:
    - [ ] `UDataTable* AreaDataTable` – DT_MapAreas (editable, configured in GameInstance BP)
    - [ ] `UDataTable* StartPointDataTable` – DT_StartPoints (editable, configured in GameInstance BP)
  - [ ] Add methods:
    - [ ] `Initialize()` – validate DataTables, cache office position
    - [ ] `GetAreaData(FName AreaID)` – lookup area row
    - [ ] `GetStartPointData(FName StartPointID)` – lookup start point row
    - [ ] `GetStartPointsForArea(FName AreaID)` – return 3 start points for given area
    - [ ] `CalculateRouteCost(FName StartPointID)` – return Money + Supply cost from DataTable
    - [ ] `GetRouteSplinePoints(FName StartPointID)` – return waypoints for visual route drawing
    - [ ] `IsSubCellExplored(FName AreaID, int32 SubCellIndex)` – check if sub-cell explored (0-143)
    - [ ] `SetSubCellExplored(FName AreaID, int32 SubCellIndex, bool bExplored)` – update campaign save + mark dirty
    - [ ] `UpdateExplorationAtWorldPosition(FVector WorldPosition)` – called by convoy Tick, converts world coords to area+sub-cell index
    - [ ] `GetExploredPercentage(FName AreaID)` – return 0.0-1.0 for area completion tracking
    - [ ] `ValidatePlanningState()` – ensure selected area+start point exist and are affordable

#### Step 4.2.3: Planning State Persistence (GameInstance Integration)

- [ ] **Extend `UFCGameInstance`**
  - [ ] Add `FFCExpeditionPlanningState CurrentPlanningState` member
  - [ ] Add methods:
    - [ ] `SavePlanningState(AreaID, StartPointID)` – immediately save to CurrentPlanningState
    - [ ] `LoadPlanningState()` – return CurrentPlanningState (called by widget on open)
    - [ ] `ClearPlanningState()` – reset selections (called after expedition starts)
    - [ ] `GetWorldMapManager()` – subsystem accessor helper

#### Step 4.2.4: WBP_WorldMap Widget (Blueprint UI)

- [ ] **Visual Setup**

  - [ ] Create `WBP_WorldMap` (UMG Widget)
  - [ ] Add **Background Image** (world map texture with coastlines)
  - [ ] Add **Fog of War Overlay** (Grid Panel 12×12 per area, or Image with material mask)
    - [ ] **Option A (Simple)**: Use Uniform Grid Panel with 144 widgets per area (visibility toggled via `IsSubCellExplored`)
    - [ ] **Option B (Optimized)**: Use single Image per area with Material Instance Dynamic (write sub-grid bool array to texture parameter)
    - [ ] Bind visibility/opacity to `UFCWorldMapManager->IsSubCellExplored(AreaID, SubCellIndex)`
  - [ ] Add **Area Buttons** (Invisible buttons positioned over each grid cell)
    - [ ] On Click → call `SelectArea(AreaID)`
  - [ ] Add **Start Point Selection Panel** (Hidden by default)
    - [ ] 3 Radio Buttons (one per start point)
    - [ ] For each: display Name, Risk, Money Cost, Supply Cost
    - [ ] On Click → call `SelectStartPoint(StartPointID)`
  - [ ] Add **Route Visualization** (Spline or Line Renderer)
    - [ ] Draw from office position (bottom center) to selected start point
    - [ ] Use `GetRouteSplinePoints()` waypoints
  - [ ] Add **Resource Display** (Top bar)
    - [ ] Show current Money, Supplies (from GameInstance)
    - [ ] Highlight in red if insufficient for selected start point
  - [ ] Add **Start Expedition Button**
    - [ ] Enable only if: selection valid AND resources sufficient
    - [ ] On Click → call `StartExpedition()`

- [ ] **Logic & Event Graph**

  - [ ] `Event Construct`:
    - [ ] Get `UFCGameInstance` → `LoadPlanningState()`
    - [ ] If state exists: restore area + start point selection, redraw route
    - [ ] Query `UFCWorldMapManager` for all areas, populate buttons
    - [ ] Refresh fog of war overlay (iterate through all sub-cells per area)
  - [ ] `SelectArea(AreaID)`:
    - [ ] Get `UFCWorldMapManager->GetStartPointsForArea(AreaID)`
    - [ ] Populate start point radio buttons with data
    - [ ] Show start point selection panel
    - [ ] Call `UFCGameInstance->SavePlanningState(AreaID, NAME_None)` (area set, start point pending)
  - [ ] `SelectStartPoint(StartPointID)`:
    - [ ] Get start point data from `UFCWorldMapManager`
    - [ ] Update route visualization (call `GetRouteSplinePoints()`)
    - [ ] Update resource display (highlight costs)
    - [ ] Enable/disable Start Expedition button based on affordability
    - [ ] Call `UFCGameInstance->SavePlanningState(SelectedAreaID, StartPointID)`
  - [ ] `StartExpedition()`:
    - [ ] Validate via `UFCWorldMapManager->ValidatePlanningState()`
    - [ ] Deduct costs from `UFCGameInstance` (Money, Supplies)
    - [ ] Call `UFCGameStateManager->TransitionTo(Overworld_Travel)`
    - [ ] Call `UFCLevelManager->LoadLevel(SelectedArea->LevelName)`
    - [ ] Close widget

- [ ] **Fog of War Update (Runtime)**
  - [ ] In Overworld convoy pawn's `Tick()` or movement component:
    - [ ] Call `UFCWorldMapManager->UpdateExplorationAtWorldPosition(GetActorLocation())` every frame
    - [ ] Manager converts world position to area + sub-cell index
    - [ ] Sets sub-cell explored if not already marked
    - [ ] Marks save data dirty for persistence
  - [ ] In `WBP_WorldMap`, on open or periodic refresh:
    - [ ] Query `IsSubCellExplored()` for all sub-cells in visible areas
    - [ ] Update fog overlay widgets/material accordingly

#### Step 4.2.5: Office Integration (Table Interaction)

- [ ] **Connect to existing table system**
  - [ ] Ensure `BP_TableObject_Map` (from Week 2) triggers `WBP_WorldMap`
  - [ ] On interact → `UFCUIManager->ShowTableWidget(WBP_WorldMap)`
  - [ ] Widget handles own visibility and ESC behavior (close widget, persist state)

#### Step 4.2.6: Overworld Integration (Minimap Mode)

- [ ] **Add Map Toggle Input Action**

  - [ ] Create `IA_ToggleMap` (Key: M) in `Content/FC/Input/Actions/`
  - [ ] Bind in `AFCPlayerController` (TopDown input context)
  - [ ] On press: call `UFCUIManager->ToggleWorldMap()`

- [ ] **UFCUIManager Extension**
  - [ ] Add method: `ToggleWorldMap()`
    - [ ] If map not open: show `WBP_WorldMap` in "View Only" mode
    - [ ] If map open: hide widget
  - [ ] View Only Mode changes:
    - [ ] Hide Start Expedition button
    - [ ] Hide start point selection (show only if area explored)
    - [ ] Add "You are here" marker (query current level from `UFCLevelManager`)

#### Step 4.2.7: DataTable Assets (Content Creation)

- [ ] **Create `DT_MapAreas` DataTable**

  - [ ] Location: `Content/FC/Data/`
  - [ ] Row Structure: `FFCMapAreaData`
  - [ ] Populate with example areas:
    - [ ] Forest (AreaID: "Forest", GridPosition: (2,3), LevelName: "L_Overworld_Forest", ExploredSubCells: 144 false entries)
    - [ ] Mountains (AreaID: "Mountains", GridPosition: (3,2), LevelName: "L_Overworld_Mountains", ExploredSubCells: 144 false entries)
    - [ ] Swamp (AreaID: "Swamp", GridPosition: (1,4), LevelName: "L_Overworld_Swamp", ExploredSubCells: 144 false entries)

- [ ] **Create `DT_StartPoints` DataTable**

  - [ ] Location: `Content/FC/Data/`
  - [ ] Row Structure: `FFCStartPointData`
  - [ ] Populate with 3 start points per area:
    - [ ] Forest_North: Risk=20, Money=50, Supply=3, SplinePoints=[...], AreaID="Forest"
    - [ ] Forest_East: Risk=30, Money=70, Supply=5, SplinePoints=[...], AreaID="Forest"
    - [ ] Forest_South: Risk=10, Money=30, Supply=2, SplinePoints=[...], AreaID="Forest"
    - [ ] (Repeat for Mountains, Swamp)

- [ ] **Configure in `BP_FC_GameInstance`**
  - [ ] Open `BP_FC_GameInstance`
  - [ ] Find `UFCWorldMapManager` subsystem settings
  - [ ] Assign `DT_MapAreas` to `AreaDataTable`
  - [ ] Assign `DT_StartPoints` to `StartPointDataTable`

---

### Step 4.3: Expedition Return Flow

_Goal: Allow the player to successfully return from the Overworld to the Office._

#### Step 4.3.1: Extraction Point Actor

- [ ] **Create `BP_ExtractionPoint`**
  - [ ] Create Actor Blueprint `BP_ExtractionPoint`.
  - [ ] Add Static Mesh (Visual representation, e.g., a flag or wagon).
  - [ ] Add Box Collision (Trigger zone).
  - [ ] Implement "Return Home" Action:
    - [ ] On Interaction (or Overlap + Key Press):
      - [ ] Open `WBP_ExpeditionSummary`.

#### Step 4.3.2: Expedition Summary Screen

- [ ] **Create `WBP_ExpeditionSummary`**
  - [ ] Create Widget Blueprint.
  - [ ] Add Text: "Expedition Complete".
  - [ ] Add Placeholder Stats: "Resources Collected: [X]", "Crew Status: [Y]".
  - [ ] Add Button: "Return to Office".

#### Step 4.3.3: Transition Logic

- [ ] **Implement Return Logic**
  - [ ] On "Return to Office" Click:
    - [ ] Get `UFCGameStateManager`.
    - [ ] Call `TransitionTo(EFCGameStateID::Office_Exploration)`.
    - [ ] Get `UFCLevelManager`.
    - [ ] Call `LoadLevel("L_Office")`.
  - [ ] **Verify**: Ensure this flow triggers the fix from Task 4.1.1 (State preservation).

---

### Step 4.4: Integration & Testing

#### Step 4.4.1: Full Loop Verification

- [ ] **Test Sequence**
  1.  Start in Office.
  2.  Open Map (Planning Mode) -> Select Route -> Check Costs -> Start Expedition.
  3.  Load into Overworld.
  4.  Move around.
  5.  Press 'M' -> Check Map (View Mode).
  6.  Press 'ESC' -> Check Pause Menu (Abort Button Visible, ESC closes menu).
  7.  Go to `BP_ExtractionPoint` -> Trigger Return.
  8.  View Summary -> Click Return.
  9.  Load into Office -> **Verify Input/State is correct (First Person, no cursor issues)**.

#### Step 4.4.2: Documentation

- [ ] Update `Docs/Technical_Documentation.md` with new Map System architecture.
- [ ] Mark Issues #1, #2, #4 as Resolved in `backlog.md`.
