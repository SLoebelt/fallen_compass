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

- [ ] **Analysis of Existing Implementations**

  - [ ] **State Management**: Review `Source/FC/Core/FCGameStateManager.h` and `FCPlayerController.cpp` to understand the current state transition logic causing Issue #1.
  - [ ] **UI Input**: Review `WBP_PauseMenu` and `FCPlayerController` input handling to address Issue #4 (ESC toggle).
  - [ ] **Map System**: Check if any map assets exist in `Content/FC/UI/` or `Content/FC/Data/`. If not, plan the `UFCWorldMapSubsystem` or DataAssets needed for the complex map requirements.
  - [ ] **Interaction System**: Verify if an Interaction Interface exists for the "Return Home" POI action. If not, plan `BPI_Interactable`.

- [ ] **Code Conventions Compliance Check**

  - [ ] **Encapsulation**: Ensure new Map data structures (Routes, Areas) are properly encapsulated in C++ structs/classes or DataAssets.
  - [ ] **Separation of Concerns**: Keep Map logic (Costs, Risks) separate from the UI Widget (`WBP_OverworldMap`). Use a Manager or Subsystem.
  - [ ] **Naming**: Ensure new widgets use `WBP_` and C++ classes use `FC` prefix.

- [ ] **Implementation Plan**
  - [ ] **Task 4.1 (Bugs)**: Implement fixes for Issues #1, #2, #4 using the strategies defined in `backlog.md`.
  - [ ] **Task 4.2 (Map)**: Create `UFCMapDataAsset` (for areas/routes), `WBP_WorldMap` (visuals), and logic for cost/risk calculation.
  - [ ] **Task 4.3 (Return)**: Create `BP_ExtractionPoint` actor and `WBP_ExpeditionSummary`.

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

_Goal: Implement a comprehensive map system for both Expedition Planning (Office) and Orientation (Overworld), meeting specific user requirements._

#### Step 4.2.1: Map Data Architecture

- [ ] **Create Data Structures (C++ or Blueprint)**
  - [ ] Create `FMapRouteSegment` struct:
    - [ ] StartPoint (ID/Name), EndPoint (ID/Name).
    - [ ] RiskValue (Float/Int).
    - [ ] TravelCost (Money/Proviant).
    - [ ] bIsKnown (Boolean).
  - [ ] Create `FMapArea` struct:
    - [ ] Name, Description, Image.
    - [ ] bIsExplored (Fog of War status).
    - [ ] List of EntryPoints/POIs.
  - [ ] Create `UFCWorldMapData` (DataAsset) to hold the static world definition (Continents, Areas, Routes).

#### Step 4.2.2: WBP_WorldMap Implementation

- [ ] **Visual Setup**
  - [ ] Create `WBP_WorldMap`.
  - [ ] Add **Background Image** (Fictional World Map).
  - [ ] Add **Fog of War Layer** (Masked image or separate overlay widgets for unexplored areas).
  - [ ] Add **Route Visualization** (Splines or simple lines connecting POIs).
  - [ ] Add **Area Click Detection** (Buttons or invisible interactable widgets over map regions).

#### Step 4.2.3: Logic & Interaction

- [ ] **Area Interaction**
  - [ ] On Area Click: Show `WBP_AreaInfo` popup (Name, Description).
  - [ ] Display "Possible Entry Points" for the selected area.
- [ ] **Route Calculation Logic**
  - [ ] Implement function `CalculateRouteCost(Start, End)`:
    - [ ] Sum up `TravelCost` (Money, Proviant) for all segments.
    - [ ] Sum up `RiskValue`.
  - [ ] Update UI to show "Required: X Gold, Y Proviant".
- [ ] **Start Expedition Validation**
  - [ ] Check Player Inventory (Mock or actual GameState).
  - [ ] **IF** (PlayerGold >= Cost AND PlayerProviant >= Cost):
    - [ ] Enable "Start Expedition" button.
    - [ ] Allow start regardless of Crew count (as per requirement "own risk").
  - [ ] **ELSE**: Disable button and show missing resources.

#### Step 4.2.4: Overworld Integration (Minimap Mode)

- [ ] **Toggle Logic**
  - [ ] In `AFCPlayerController` or `UFCUIManager`, add `IA_ToggleMap` (Key: M).
  - [ ] On Toggle: Open `WBP_WorldMap` in "View Only" mode (Hide "Start Expedition" buttons, show "You are here" marker).

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
