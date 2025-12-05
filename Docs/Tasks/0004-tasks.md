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

### Step 4.2: World Map & Expedition Planning System

_Goal: Drive the new 16×16 planning map by wiring the UMG widget to UFCExpeditionManager, supplying a land mask texture, and feeding live exploration data from the overworld convoy._

#### Step 4.2.1: Create WBP_WorldMap Widget Blueprint (UI Foundation)

- [x] **Create Widget Blueprint**

  - [x] Navigate to `Content/FC/UI/ReusableWidgets/`
  - [x] Right-click → User Interface → Widget Blueprint
  - [x] Name: `WBP_WorldMap`
  - [x] Open the widget in the UMG Designer

- [x] **Designer Layout - Canvas Panel Root**
  - [x] Add **Canvas Panel** as root (full-screen anchor: 0,0 to 1,1)
  - [x] Add **Image** widget: `Image_MapBackground`
    - [x] Set Anchor: Centered (0.5, 0.5)
    - [x] Set Size: 1024×1024 (adjust to your map artwork size)
    - [x] Set Alignment: (0.5, 0.5) to center pivot
    - [x] Assign Brush → Image: Import your world map background texture (coastlines, terrain art)
    - [x] Set Brush Tint: (1,1,1,1) for full visibility
- [x] **Fog of War Overlay Image**

  - [x] Add **Image** widget: `Image_FogOverlay` as child of Canvas Panel
  - [x] Position/size to exactly match `Image_MapBackground` (use same anchor/alignment)
  - [x] Leave Brush empty (will assign Material Instance Dynamic at runtime)
  - [x] Set Z-Order higher than background (drag below background in hierarchy to render on top)
  - [x] **Purpose**: This will display the fog texture from UFCExpeditionManager

- [x] **Route Visualization Overlay Image**

  - [x] Add **Image** widget: `Image_RouteOverlay` as child of Canvas Panel
  - [x] Position/size to exactly match map background
  - [x] Leave Brush empty (will assign Material Instance Dynamic at runtime)
  - [x] Set Z-Order higher than fog overlay
  - [x] **Purpose**: This will display the route preview texture with red path cells

- [x] **Grid Area Buttons (Milestone: Single Button)**

  - [x] Add **Button** widget: `Button_Grid24`
  - [x] Position over Grid 24 on your map artwork (calculate pixel coordinates based on 16×16 grid layout)
  - [x] Set Size: ~64×64 pixels (1024 / 16 = 64px per grid cell)
  - [x] Set Style → Normal/Hovered/Pressed Tint: Semi-transparent (0.2 alpha) for debugging
  - [x] Add **Text Block** as child: `Text_Grid24` with content "Grid 24" (for debugging, can hide later)
  - [x] **Future Expansion**: Add 255 more buttons for all grid cells (or use procedural generation)

- [x] **Start Point Selection Panel (Hidden by Default)**

  - [x] Add **Vertical Box** widget: `Panel_StartPointSelection`
  - [x] Set Visibility: `Collapsed` (will show after grid area selected)
  - [x] Position: Right side of screen or overlay modal
  - [x] Add **Text Block**: `Text_AreaTitle` (will display selected area name)
  - [x] Add **Text Block**: `Text_MoneyCost` (format: "Cost: {Money} Gold")
  - [x] Add **Text Block**: `Text_RiskCost` (format: "Risk: {Risk}%")
  - [x] Add **Button**: `Button_StartExpedition` with text "Depart"
    - [x] Set IsEnabled: false (enable after validation)
  - [x] **Note**: For milestone, you can hardcode single start point; later add radio buttons for 3 start points per area

- [ ] **Route Preview Controls**
  - (moved) See **Clustered Remaining Tasks** section below for Route & UX cluster.

- [ ] **Resource Display (Top Bar)**
  - (moved) See **Clustered Remaining Tasks** section below for Route & UX cluster.

- [ ] **Close Button**
  - (moved) See **Clustered Remaining Tasks** section below for Route & UX cluster.

#### Step 4.2.2: Wire Event Graph - Subsystem Connection & Initialization

- [x] **Create Variables**

  - [x] Add variable: `ExpeditionManager` (Type: `UFCExpeditionManager`, Object Reference)
    - [x] Set Instance Editable: false
    - [x] Set Tooltip: "Cached reference to expedition manager subsystem"
  - [x] Add variable: `FogMaterialInstance` (Type: `UMaterialInstanceDynamic`, Object Reference)
  - [x] Add variable: `RouteMaterialInstance` (Type: `UMaterialInstanceDynamic`, Object Reference)
  - [x] Add variable: `bPreviewRunning` (Type: `bool`, default: false)
  - [x] Add variable: `SelectedGridIndex` (Type: `int32`, default: -1)

- [x] **Event Construct - Subsystem Initialization**

  - [x] Drag `Event Construct` node into graph
  - [x] Add node: `Get Game Instance`
  - [x] Cast to `UFCGameInstance`
  - [x] Add node: `Get Subsystem` (Class: `UFCExpeditionManager`)
  - [x] **Branch**: Check if subsystem is valid
    - [x] **True Path**:
      - [x] Set `ExpeditionManager` variable
      - [x] Add node: `Print String` → "WorldMap: Expedition Manager connected" (for debugging)
    - [x] **False Path**:
      - [x] Add node: `Print String` → "ERROR: WorldMap: Failed to get ExpeditionManager!" (red)
      - [x] Add node: `Remove from Parent` (close widget)
      - [x] **Return** (stop execution)

- [x] **Event Construct - Fog Overlay Setup**

  - [x] Add node: `ExpeditionManager → WorldMap_GetFogTexture()`
  - [x] Store result in local variable: `FogTexture` (Type: `UTexture2D`)
  - [x] **Branch**: Check if `FogTexture` is valid
    - [x] **True Path**:
      - [x] Add node: `Create Dynamic Material Instance`
        - [x] Parent: `M_FogOfWar` (create this material in Step 4.2.3)
      - [x] Set `FogMaterialInstance` variable
      - [x] Add node: `Set Texture Parameter Value`
        - [x] Target: `FogMaterialInstance`
        - [x] Parameter Name: "FogTexture" (must match material parameter)
        - [x] Value: `FogTexture`
      - [x] Add node: `Image_FogOverlay → Set Brush from Material`
        - [x] Material: `FogMaterialInstance`
    - [x] **False Path**:
      - [x] Add node: `Print String` → "Warning: Fog texture not available" (yellow)

- [x] **Event Construct - Route Overlay Setup**

  - [x] Add node: `ExpeditionManager → WorldMap_GetRouteTexture()`
  - [x] Store result in local variable: `RouteTexture` (Type: `UTexture2D`)
  - [x] **Branch**: Check if `RouteTexture` is valid
    - [x] **True Path**:
      - [x] Add node: `Create Dynamic Material Instance`
        - [x] Parent: `M_RouteOverlay` (create this material in Step 4.2.4)
      - [x] Set `RouteMaterialInstance` variable
      - [x] Add node: `Set Texture Parameter Value`
        - [x] Target: `RouteMaterialInstance`
        - [x] Parameter Name: "RouteTexture"
        - [x] Value: `RouteTexture`
      - [x] Add node: `Image_RouteOverlay → Set Brush from Material`
        - [x] Material: `RouteMaterialInstance`
    - [x] **False Path**:
      - [x] Add node: `Print String` → "Warning: Route texture not available" (yellow)

- [x] **Event Construct - Restore Planning State (If Expedition Active)**

  - [x] Add node: `ExpeditionManager → GetCurrentExpedition()`
  - [x] Store result in local variable: `CurrentExpedition` (Type: `FFCExpeditionState`)
  - [x] **Branch**: Check if `CurrentExpedition.bIsActive` is true
    - [x] **True Path** (expedition already planned/in progress):
      - [x] Get `CurrentExpedition.PlannedGridAreaIndex` → Set `SelectedGridIndex` variable
      - [x] Get `CurrentExpedition.PlannedMoneyCost` → Update `Text_MoneyCost`
      - [x] Get `CurrentExpedition.PlannedRiskCost` → Update `Text_RiskCost`
      - [x] Add node: `Panel_StartPointSelection → Set Visibility` (Visible)
      - [ ] **Branch**: Check if `CurrentExpedition.PlannedRouteGlobalIds.Num() > 0`
        - [ ] **True**: Enable `Button_PreviewRoute` (route exists)
        - [ ] **False**: Keep `Button_PreviewRoute` disabled
      - [ ] Add node: `Print String` → "Restored active expedition state" (green)
    - [x] **False Path**:
      - [x] Keep `Panel_StartPointSelection` collapsed (no selection yet)

- [ ] **Event Construct - Resource Display Update**
  - (moved) See **Clustered Remaining Tasks** section below for Route & UX cluster.

#### Step 4.2.3: Create M_FogOfWar Material (Fog Visualization)

- [x] **Create Material Asset**

  - [x] Navigate to `Content/FC/Assets/Materials/`
  - [x] Right-click → Material → Name: `M_FogOfWar`
  - [x] Open material editor

- [x] **Material Properties**

  - [x] Set Material Domain: `User Interface`
  - [x] Set Blend Mode: `Translucent`
  - [ ] Enable `Fully Rough`: true (no lighting on UI)

- [x] **Parameter Setup**

  - [x] Add `Texture Sample` parameter:
    - [x] Name: "FogTexture"
    - [x] Type: `Texture2D`
    - [x] Default: Black texture
    - [x] Convert to Parameter (right-click → Convert to Parameter)
  - [x] Add `Scalar Parameter`:
    - [x] Name: "FogOpacity"
    - [x] Default Value: 0.8
  - [x] Add `Vector Parameter`:
    - [x] Name: "FogColor"
    - [x] Default: (0, 0, 0, 1) - black

- [x] **Material Graph Logic**

  - [x] Connect `Texture Sample → FogTexture` to texture coordinate input
  - [x] Sample `FogTexture` at `TexCoord[0]` (UV channel 0)
  - [x] Extract R channel from texture sample (grayscale: R=G=B)
  - [x] Add `Lerp` node:
    - [x] A: `FogColor` (opaque black for unexplored)
    - [x] B: Constant `(0,0,0,0)` (transparent for explored)
    - [x] Alpha: `1 - TextureSample.R` (invert so 0=unexplored/foggy, 1=explored/clear)
  - [x] Multiply Lerp output alpha by `FogOpacity` parameter
  - [x] Connect Lerp RGB to **Emissive Color** pin
  - [x] Connect modified alpha to **Opacity** pin

- [x] **Save and Test**
  - [x] Click Apply/Save
  - [x] **Test**: Create material instance, assign black/white test texture, verify fog shows correctly

#### Step 4.2.4: Create M_RouteOverlay Material (Route Visualization)

- [x] **Create Material Asset**

  - [x] Navigate to `Content/FC/Assets/Materials/`
  - [x] Right-click → Material → Name: `M_RouteOverlay`
  - [x] Open material editor

- [x] **Material Properties**

  - [x] Set Material Domain: `User Interface`
  - [x] Set Blend Mode: `Translucent`

- [x] **Parameter Setup**

  - [x] Add `Texture Sample` parameter:
    - [x] Name: "RouteTexture"
    - [x] Type: `Texture2D`
    - [x] Convert to Parameter
  - [x] Add `Vector Parameter`:
    - [x] Name: "RouteColor"
    - [x] Default: (1, 0, 0, 1) - red
  - [x] Add `Scalar Parameter`:
    - [x] Name: "RouteOpacity"
    - [x] Default: 0.7

- [x] **Material Graph Logic**

  - [x] Sample `RouteTexture` at `TexCoord[0]`
  - [x] Extract R channel (route cells have value > 0.5)
  - [x] Add `If` node (or `Lerp` + `Ceil`):
    - [x] Condition: `TextureSample.R > 0.5`
    - [x] True: `RouteColor` with `RouteOpacity` alpha
    - [x] False: Transparent (0,0,0,0)
  - [x] Connect result RGB to **Emissive Color**
  - [x] Connect alpha to **Opacity**

- [x] **Save and Test**

#### Step 4.2.5: Wire Button Events - Grid Selection & Route Planning

- [x] **Button_Grid24 → OnClicked Event**

  - [x] Drag `Button_Grid24` from Variables panel
  - [x] Right-click → Add Event → On Clicked
  - [x] Add node: `ExpeditionManager → WorldMap_SelectGridArea`
    - [x] GridIndex: 24 (hardcoded for milestone)
  - [x] Store return value in local variable: `bSelectionSuccess` (bool)
  - [x] **Branch**: Check `bSelectionSuccess`
    - [x] **True Path**:
      - [x] Set `SelectedGridIndex` variable = 24
      - [x] Add node: `ExpeditionManager → GetCurrentExpedition`
      - [x] Get `PlannedMoneyCost` → Format string "Cost: {0} Gold" → Set `Text_MoneyCost`
      - [x] Get `PlannedRiskCost` → Format string "Risk: {0}%" → Set `Text_RiskCost`
      - [x] Add node: `Panel_StartPointSelection → Set Visibility` (Visible)
      - [x] Add node: `Button_PreviewRoute → Set Is Enabled` (true)
      - [x] Add node: `Print String` → "Grid 24 selected" (green)
      - [x] **Resource Validation**:
        - [x] Get `UFCGameInstance → GameStateData.Money`
        - [x] **Branch**: Check if `PlayerMoney >= PlannedMoneyCost`
          - [x] **True**: Set `Text_MoneyCost` color to white, enable `Button_StartExpedition`
          - [x] **False**: Set `Text_MoneyCost` color to red, disable `Button_StartExpedition`
    - [x] **False Path**:
      - [x] Add node: `Print String` → "ERROR: Failed to select Grid 24 (no valid route or land-locked)" (red)
      - [x] Keep `Panel_StartPointSelection` collapsed

- [ ] **Button_StartExpedition → OnClicked Event**
  - (moved) See **Clustered Remaining Tasks** section below for Start-Expedition cluster.

- [ ] **Button_Close → OnClicked Event**
  - (moved) See **Clustered Remaining Tasks** section below for Route & UX cluster.

#### Step 4.2.6: Implement Fog of War Refresh (Delegate Binding)

- [x] **Create Custom Event: OnExplorationChanged**

  - [x] Add `Custom Event` → Name: "OnExplorationChanged"
  - [x] Add node: `ExpeditionManager → WorldMap_GetFogTexture()`
  - [x] Add node: `FogMaterialInstance → Set Texture Parameter Value`
    - [x] Parameter Name: "FogTexture"
    - [x] Value: refreshed fog texture
  - [x] Add node: `Print String` → "Fog of war updated (new cells explored)" (cyan)
  - [x] **Optional**: Update explored percentage text
    - [x] Add node: `ExpeditionManager → GetExploredPercentage` (if exists)
    - [x] Format string: "Explored: {0}%" → Set text widget

- [x] **Event Construct - Bind to OnWorldMapChanged Delegate**

  - [x] After subsystem initialization, add node: `ExpeditionManager → OnWorldMapChanged → Add Event`
  - [x] Connect to `OnExplorationChanged` custom event
  - [x] Add node: `Print String` → "Bound to OnWorldMapChanged delegate" (cyan)

- [x] **Event Destruct - Unbind Delegate**
  - [x] Drag `Event Destruct` into graph
  - [x] **Branch**: Check if `ExpeditionManager` is valid
    - [x] **True Path**:
      - [x] Add node: `ExpeditionManager → OnWorldMapChanged → Remove Event`
      - [x] Connect to `OnExplorationChanged` custom event
      - [x] Add node: `Print String` → "Unbound from OnWorldMapChanged delegate" (yellow)

#### Step 4.2.7: Create T_LandMask_256 Texture Asset (Content Creation)

- [x] **Author Land Mask Image (External Tool)**

  - [x] Open image editor (Photoshop, GIMP, Krita, etc.)
  - [x] Create new image: **256×256 pixels**, grayscale, 8-bit
  - [x] Use your planning map reference artwork
  - [x] Paint **white (RGB 255,255,255)** for water areas
  - [x] Paint **black (RGB 0,0,0)** for land areas
  - [x] Ensure office grid cell (e.g., bottom-center) is white
  - [x] **Critical**: Image must be exactly 256×256 pixels (16×16 grid × 16 pixels per cell)
  - [x] Save as PNG: `LandMask_256.png`

- [x] **Import into Unreal**

  - [x] Navigate to `Content/FC/Data/` (or `Content/FC/Assets/Textures/`)
  - [x] Drag `LandMask_256.png` into Content Browser
  - [x] Import settings:
    - [x] Texture Group: `UI` (or `2D Pixels (unfiltered)`)
    - [x] Compression: `Grayscale` or `VectorDisplacementmap` (no compression)
    - [x] sRGB: **Unchecked** (critical - we need linear values)
    - [x] Mip Gen Settings: `NoMipmaps`
  - [x] Rename imported texture to `T_LandMask_256`

- [x] **Configure Texture Properties**

  - [x] Open `T_LandMask_256` in asset editor
  - [x] Verify **Texture Properties** panel:
    - [x] Size: 256×256 (must match exactly)
    - [x] Format: `G8` or `RGBA8` (8-bit per channel)
    - [x] sRGB: false
    - [x] Mip Maps: 0 (no mipmaps)
  - [x] Scroll to **Advanced** section:
    - [x] Enable: `Allow CPU Access` (critical - subsystem reads pixel data)
  - [x] Click **Save**

- [x] **Verify Pixel Data**
  - [x] In texture preview, zoom to 100%
  - [x] Manually check a few known grid cells:
    - [x] Office cell: should be white
    - [x] Ocean cells: should be black
    - [x] Coastal boundaries: sharp black/white edges (no gray blending)
  - [x] If colors look wrong, re-export with no gamma correction

#### Step 4.2.8: Configure BP_FC_GameInstance with World Map Settings

- [x] **Open Game Instance Blueprint**

  - [x] Navigate to `Content/FC/Blueprints/Core/`
  - [x] Open `BP_FC_GameInstance`
  - [x] Switch to **Class Defaults** view

- [x] **Configure Expedition|WorldMap Category**

  - [x] In Details panel, find category: `Expedition|WorldMap`
  - [x] This section contains all UFCExpeditionManager configuration that will be passed to the subsystem on initialization

- [x] **Assign Land Mask Texture**

  - [x] Find property: `WorldMapLandMaskTexture` (Type: `TSoftObjectPtr<UTexture2D>`)
  - [x] Click dropdown → Search: "T_LandMask_256"
  - [x] Select `T_LandMask_256` texture asset
  - [x] **Purpose**: This texture defines traversable (white) vs blocked (black) cells on the 16×16 planning map

- [x] **Configure Overworld Bounds**

  - [x] Find property: `OverworldWorldMin` (Type: `FVector2D`)
    - [x] Set to minimum world coordinates of your overworld level (default: -50000, -50000)
  - [x] Find property: `OverworldWorldMax` (Type: `FVector2D`)
    - [x] Set to maximum world coordinates of your overworld level (default: 50000, 50000)
  - [x] **Purpose**: These bounds map 3D world positions to 2D planning map coordinates for fog-of-war tracking

- [x] **Configure Office Grid Position**

  - [x] Find property: `OfficeGridId` (Type: `int32`)
    - [x] Set to grid cell index where office is located (default: 8)
    - [x] **Calculation**: GridId = GridY × 16 + GridX (e.g., row 0, column 8 → 8)
  - [x] Find property: `OfficeSubId` (Type: `int32`)
    - [x] Set to sub-cell within the grid cell 0-15 (default: 0 for top-left)
  - [x] **Purpose**: Defines starting position for all expedition routes

- [x] **Configure Milestone Start Point (Week 4 Demo)**

  - [x] Find property: `AvailableStartGridId` (Type: `int32`)
    - [x] Set to demo start point grid cell (default: 24)
  - [x] Find property: `AvailableStartSubId` (Type: `int32`)
    - [x] Set to demo start point sub-cell (default: 26)
  - [x] **Purpose**: Single available expedition start point for Week 4 milestone

- [x] **Configure Preview Target (Route Demo)**

  - [x] Find property: `PreviewTargetGridId` (Type: `int32`)
    - [x] Set to preview target grid cell (default: 25)
  - [x] Find property: `PreviewTargetSubId` (Type: `int32`)
    - [x] Set to preview target sub-cell (default: 0)
  - [x] **Purpose**: End point for route preview visualization

- [x] **Save and Compile**

  - [x] Click **Compile** button
  - [x] Click **Save** button
  - [x] Close Blueprint editor

- [x] **Verify in Play-In-Editor (PIE)**
  - [x] Click **Play** button
  - [x] Open Output Log (Window → Developer Tools → Output Log)
  - [x] Filter by `LogFCExpedition`
  - [x] Look for log line: "Expedition Manager configured from GameInstance"
  - [x] **If you see "Failed to get UFCGameInstance - using default subsystem config"**:
    - [x] Verify BP_FC_GameInstance is set as Game Instance Class in Project Settings → Maps & Modes
  - [x] **Note**: Subsystem configuration now comes from GameInstance, not directly from subsystem properties
    - [x] Re-check texture import (must be exactly 256×256)
    - [x] Re-check "Allow CPU Access" is enabled
    - [x] Re-import texture if necessary
  - [x] Stop PIE

#### Step 4.2.9: Feed Overworld Exploration Updates (Convoy Integration)
  - (moved) See **Clustered Remaining Tasks** section below for Convoy integration cluster.

---

### Step 4.2.P1: Close Priority 1 Gaps (Core Loop & Data Consistency)

_Goal: Make the world map configuration, exploration updates, and expedition planning state fully functional and consistent between C++ and Blueprint/UMG, as described in `Docs/analysis_0004.md` Priority 1._

#### Step 4.2.P1.1: GameInstance → ExpeditionManager World-Map Configuration

- [x] **Implement config copy in `UFCExpeditionManager::Initialize`**
  - [x] In `Source/FC/Expedition/FCExpeditionManager.cpp`, inside `UFCExpeditionManager::Initialize`:
    - [x] Cast `GetGameInstance()` to `UFCGameInstance`.
    - [x] Copy `WorldMapLandMaskTexture`, `OverworldWorldMin`, `OverworldWorldMax`.
    - [x] Copy `OfficeGridId`, `OfficeSubId`, `AvailableStartGridId`, `AvailableStartSubId`, `PreviewTargetGridId`, `PreviewTargetSubId`.
    - [x] Copy `DefaultRevealedWorldMapGridIds`.
    - [x] Log "Expedition Manager configured from GameInstance" on success; log a warning and keep defaults if cast fails.
    - [x] Call `WorldMap_InitOrLoad()` after configuration is applied.

- [x] **Configure `BP_FC_GameInstance` World Map Settings**
  - [x] Open `Content/FC/Blueprints/Core/BP_FC_GameInstance`.
  - [x] In Class Defaults → `Expedition|WorldMap`:
    - [x] Assign `T_LandMask_256` to `WorldMapLandMaskTexture`.
    - [x] Set `OverworldWorldMin` / `OverworldWorldMax` to match the Overworld level bounds.
    - [x] Set `OfficeGridId` / `OfficeSubId` to the office location cell.
    - [x] Set `AvailableStartGridId` / `AvailableStartSubId` for the Week 4 demo start point.
    - [x] Set `PreviewTargetGridId` / `PreviewTargetSubId` for the Week 4 demo target.
    - [x] Populate `DefaultRevealedWorldMapGridIds` with the grid IDs that should be visible at campaign start.

#### Step 4.2.P1.2: Live Exploration → Fog-of-War Updates

- [x] **Implement exploration mapping in `WorldMap_RecordVisitedWorldLocation`**
  - [x] In `FCExpeditionManager.cpp`, ensure `WorldMap_RecordVisitedWorldLocation`:
    - [x] Maps world space X/Y into [0,1) UV using `OverworldWorldMin/Max`.
    - [x] Converts UV to global cell indices using `FFCWorldMapExploration::GlobalSize` and `XYToGlobalId`.
    - [x] Calls `WorldMap.SetRevealed_Global(GlobalId, true)` and captures whether a change occurred.
    - [x] If changed, calls `WorldMap_UpdateFogPixel(GlobalId)`, `WorldMap_StartAutosaveDebounced()`, and `OnWorldMapChanged.Broadcast()`.

- [x] **Hook convoy pawn into exploration recording (Blueprint)**
  - [x] Open Overworld convoy pawn Blueprint (e.g. `BP_ConvoyPawn` or equivalent).
  - [x] Add variable `CachedExpeditionManager` of type `UFCExpeditionManager` (Object Reference).
  - [x] On `Event BeginPlay`:
    - [x] Get Game Instance → Cast to `UFCGameInstance`.
    - [x] Call `GetSubsystem` (`UFCExpeditionManager`) and assign to `CachedExpeditionManager`.
  - [x] On `Event Tick` (or a timer):
    - [x] Branch: `CachedExpeditionManager` is valid AND `CachedExpeditionManager → IsExpeditionActive()` AND current map name contains "Overworld".
    - [x] If true: call `CachedExpeditionManager → WorldMap_RecordVisitedWorldLocation(GetActorLocation())`.
    - [x] Optionally cache `LastRecordedGlobalCell` to avoid redundant calls while standing still.

#### Step 4.2.P1.3: Expedition Planning State (Route, Costs & Risk)

- [x] **Extend `UFCExpeditionData` with planning state**
  - [x] In `Source/FC/Expedition/FCExpeditionData.h`:
    - [x] Add planning properties under `Category = "Expedition|Planning"`:
      - [x] `SelectedGridId`, `SelectedStartGridId`, `SelectedStartSubId`.
      - [x] `PreviewTargetGridId`, `PreviewTargetSubId`.
      - [x] `PlannedRouteGlobalIds : TArray<int32>`.
      - [x] `PlannedMoneyCost`, `PlannedRiskCost`.
    - [x] Initialize sensible defaults in the constructor (e.g. `INDEX_NONE`, zero costs, empty array).

- [x] **Populate planning state in `WorldMap_SelectGridArea` and `WorldMap_BuildPreviewRoute`**
  - [x] In `FCExpeditionManager.cpp`:
    - [x] In `WorldMap_SelectGridArea`:
      - [x] Reject grid IDs other than `AvailableStartGridId` for the Week 4 milestone and log a verbose reason.
      - [x] Create `CurrentExpedition` if null and set status to `Planning`.
      - [x] Fill `SelectedGridId`, `SelectedStartGridId`, `SelectedStartSubId`, `PreviewTargetGridId`, `PreviewTargetSubId` on `CurrentExpedition`.
    - [x] In `WorldMap_BuildPreviewRoute`:
      - [x] Use `FFCWorldMapExploration::AreaSubToGlobalId(OfficeGridId, OfficeSubId)` and preview target IDs to compute start/goal.
      - [x] Call `WorldMap.FindShortestPath_BFS(StartGlobal, GoalGlobal, Path)`; on failure, clear `PlannedRouteGlobalIds` and reset costs.
      - [x] On success, assign `PlannedRouteGlobalIds` and call `ComputeCostsForPath` to fill `PlannedMoneyCost` and `PlannedRiskCost`.
      - [x] Log path length and costs for debugging.

---

### Step 4.2.P2: Close Priority 2 Gaps (Resources & Start-Expedition Flow)

_Goal: Integrate the campaign resource system (Money/Supplies) with expedition planning and implement a robust start-expedition flow that transitions into the Overworld using `UFCGameStateManager` and `UFCLevelManager`._

#### Step 4.2.P2.1: GameInstance Resource API (Money)

- [x] **Extend `FFCGameStateData` with money helpers**
  - [x] In `Source/FC/Core/UFCGameInstance.h`:
    - [x] Verify `FFCGameStateData` already contains `Money` and `Supplies`.
    - [x] Add small inline helpers or comments clarifying their intended usage for expedition planning.

- [x] **Add explicit money functions to `UFCGameInstance`**
  - [x] In `UFCGameInstance.h` / `.cpp`:
    - [x] Add `int32 GetMoney() const;` returning `GameStateData.Money`.
    - [x] Add `void AddMoney(int32 Delta);` to increase money (clamping at zero if needed).
    - [x] Add `bool ConsumeMoney(int32 Amount);` that:
      - [x] Checks if `GameStateData.Money >= Amount`.
      - [x] Subtracts `Amount` and returns `true` on success, `false` otherwise.
    - [x] Ensure these functions are `BlueprintCallable` where useful for UI/debug cheats.

#### Step 4.2.P2.2: Planning Widget – Resource Display & Validation

- [x] **Wire resource display in `WBP_WorldMap` / planning shell**
  - [x] In `WBP_WorldMap` (UMG):
    - [x] Complete **Resource Display (Top Bar)** from Step 4.2.1:
      - [x] Add `Panel_Resources`, `Text_PlayerMoney`, and `Text_PlayerSupplies` widgets.
  - [x] In the Event Graph:
    - [x] On Construct, get `UFCGameInstance` and read `GameStateData.Money` / `GameStateData.Supplies`.
    - [x] Update `Text_PlayerMoney` and `Text_PlayerSupplies` accordingly.
    - [x] Optionally expose a `RefreshResources()` custom event so the shell widget can trigger updates after changes.

- [x] **Validate route affordability before enabling Start button**
  - [x] In `WBP_WorldMap` `Button_Grid24` click logic:
    - [x] After retrieving `PlannedMoneyCost`, call `UFCGameInstance → GetMoney()`.
    - [x] If `Money >= PlannedMoneyCost`:
      - [x] Set `Text_MoneyCost` color to normal and enable `Button_StartExpedition`.
    - [x] Else:
      - [x] Set `Text_MoneyCost` color to red and disable `Button_StartExpedition`.

#### Step 4.2.P2.3: Start-Expedition Flow (C++ Shell)

- [x] **Implement `OnStartExpeditionClicked` in `UFCExpeditionPlanningWidget`**
  - [x] In `Source/FC/UI/FCExpeditionPlanningWidget.cpp`:
    - [x] Bind `StartExpeditionButton` to `OnStartExpeditionClicked` in `NativeConstruct`.
    - [x] In `OnStartExpeditionClicked`:
      - [x] Get `UFCGameInstance` from `GetWorld()->GetGameInstance()` and cast.
      - [x] Get `UFCExpeditionManager` via `GetSubsystem<UFCExpeditionManager>()`.
      - [x] Early out (with log warnings) if any of these references are invalid.
      - [x] Retrieve `CurrentExpedition` from the expedition manager.
      - [x] Validate `PlannedRouteGlobalIds.Num() > 0` and `PlannedMoneyCost > 0`.
      - [x] Check affordability via `UFCGameInstance::ConsumeMoney(PlannedMoneyCost)`.
      - [x] If not affordable, log and return without starting.
      - [x] If affordable:
        - [x] Mark the expedition as `InProgress` (if still `Planning`).
        - [x] Optionally store the chosen expedition id/name on `UFCGameInstance`.
        - [x] Request a state change and level transition (see next subsection).

- [x] **Coordinate with GameState/Level managers**
  - [x] In `OnStartExpeditionClicked` (continuation):
    - [x] Get `UFCGameStateManager` (e.g. via `UFCGameStateManager::Get(this)` or subsystem accessor, depending on existing pattern).
    - [x] Call `TransitionTo(EFCGameStateID::Overworld_Travel)` (or appropriate enum for active expedition).
    - [x] Get `UFCLevelManager` and call `LoadLevel` with the Overworld level name.
      - [x] For Week 4, it is acceptable to hardcode an Overworld level name (e.g. `L_Overworld_Forest`) or read from a simple config field on the GameInstance.
    - [x] Close the planning widget via `RemoveFromParent()` and log "Expedition started".

#### Step 4.2.P2.4: Shared Affordability Helper (C++ + UMG)

- [x] **Introduce a shared affordability helper in C++**
  - [x] Add a small helper (e.g. on `UFCExpeditionManager` or `UFCExpeditionPlanningWidget`) that evaluates if the current expedition is affordable based on `UFCGameInstance::GetMoney()` and `CurrentExpedition->PlannedMoneyCost`.
  - [x] Use this helper from Blueprint (`WBP_ExpeditionPlanning` / map planning shell) to drive `Button_StartExpedition` enabled/disabled state instead of duplicating the money check in Blueprint.
  - [x] Use the same helper (or identical logic) in `OnStartExpeditionClicked()` to keep UI gating and C++ execution rules in sync.

#### Step 4.2.P2.5: Priority 1+2 Combined Testing Block

_Use Step 4.4.1 (Full Loop Verification) as the unified test suite for Priority 1–3 once all implementation tasks are complete._

---

### Step 4.2.P3: Close Priority 3 Gaps (Overworld View Map & UX Polish)

_Goal: Provide a view-only Overworld map mode and basic UX polish so that players can inspect explored areas during travel without opening the planning shell, and verify that logs/feedback are surfaced clearly when actions fail._

#### Step 4.2.P3.1: Overworld View-Only Map Wrapper

- [x] **Create `WBP_OverworldMap` HUD widget**
  - [x] Navigate to `Content/FC/UI/HUD/`.
  - [x] Create Widget Blueprint: `WBP_OverworldMap`.
  - [x] Add a root `Canvas Panel` and place a child `SizeBox` or `Border` centered on screen for the map.
  - [x] Inside, add a `NamedSlot` (e.g. `WorldMapSlot`) intended to host `WBP_WorldMap`.
  - [x] Add a small top bar (Text: "World Map" and hint "Press M to close").

- [x] **Embed `WBP_WorldMap` in view-only mode**
  - [x] In `WBP_OverworldMap` Event Construct, create `WBP_WorldMap` and add it as a child of `WorldMapSlot`.
  - [x] Expose a `bIsPlanningMode` (bool) on `WBP_WorldMap` (default true).
  - [x] When used from `WBP_OverworldMap`, set `bIsPlanningMode` to false so the widget hides planning-only controls (e.g. Grid selection button, Start button, route preview controls).

#### Step 4.2.P3.2: Input Binding for Overworld Map ("M" Key)

- [x] **Add input action in Project Settings**
  - [x] Open Project Settings → Input.
  - [x] Add Action Mapping `ToggleOverworldMap` bound to the `M` key.

- [x] **Handle `ToggleOverworldMap` in Overworld player controller/pawn**
  - [x] In the `FCPlayerController` (the same one that drives convoy movement), implement an input event for `ToggleOverworldMap` in `IMC_FC_TopDown`.
  - [x] On pressed:
    - [x] If `WBP_OverworldMap` is not currently open, create it via `Create Widget`, add to viewport, and set input mode to Game and UI (or UI only, depending on desired behavior).
    - [x] If already open, remove it from parent and restore normal input mode.
  - [x] Ensure the HUD map can be toggled without pausing the game.

#### Step 4.2.P3.3: Planning vs View Mode Separation in `WBP_WorldMap`

- [x] **Expose a mode flag**
  - [x] In `WBP_WorldMap`, add a `bool` variable `bIsPlanningMode` (Instance Editable, Expose on Spawn, default true).

- [x] **Hide/show planning UI based on mode**
  - [x] On Event Construct, branch on `bIsPlanningMode`.
  - [x] If `false` (view mode):
    - [x] Hide or disable `Button_Grid24`, `Panel_StartPointSelection`, and any route preview or Start-Expedition controls.
    - [x] Keep only the fog/route overlays and resource readout visible.
  - [x] If `true` (planning mode):
    - [x] Show full interaction surface as already implemented.

#### Step 4.2.P3.4: On-Screen Feedback & Logging Review

- [x] **Verify Start-Expedition failure feedback**
  - [x] In PIE, intentionally trigger each failure path for `OnStartExpeditionClicked` (no plan, insufficient money, missing subsystems, failed state transition).
  - [x] Confirm that each case logs a clear error in Output Log and shows a red/yellow on-screen debug message with the reason.

- [x] **Optional: Centralize map-related warnings**
  - [x] Review `WBP_WorldMap` Blueprint `Print String` nodes.
  - [x] Normalize message prefixes (e.g. `WorldMap:`) and colors (yellow for warnings, red for errors).
  - [x] Optionally gate very noisy debug prints behind a `bDebugMap` bool to keep shipping logs clean.

---

### Step 4.3: Expedition Return Flow

_Goal: Allow the player to successfully return from the Overworld to the Office._

#### Step 4.3.1: Extraction Point Actor

- [x] **Create `BP_ExtractionPoint`**
  - [x] Create Actor Blueprint `BP_ExtractionPoint`.
  - [x] Add Static Mesh (Visual representation, e.g., a flag or wagon).
  - [x] Add Sphere Collision (Trigger zone).
  - [x] Implement "Return Home" Action:
    - [x] On Interaction (or Overlap + Key Press):
      - [x] Open `WBP_ExpeditionSummary`.

#### Step 4.3.2: Expedition Summary Screen

- [x] **Create `WBP_ExpeditionSummary` based on C++ blocking widget**
  - [x] Create Widget Blueprint `WBP_ExpeditionSummary`.
  - [x] Set parent class to `UFCExpeditionSummaryWidget` (inherits from `UFCBlockingWidgetBase`).
  - [x] Add summary text (e.g. "Expedition Complete" and basic stats like resources collected / crew status) bound to the optional `SummaryText` slot.
  - [x] Add a primary button `ReturnButton` (visual label: "Close Summary" or similar) bound to the C++ handler `HandleReturnToOfficeClicked` or left to the internal binding from `NativeConstruct`.

#### Step 4.3.3: Transition Logic

- [x] **Introduce `EFCGameStateID::ExpeditionSummary` and wire basic transitions**
  - [x] In `EFCGameStateID` (GameStateManager), add a new value `ExpeditionSummary`.
  - [x] Extend `ValidTransitions` so that:
    - [x] `Overworld_Travel` can transition to `ExpeditionSummary` and `Paused`.
    - [x] `ExpeditionSummary` can transition back to `Office_Exploration` and `Paused`.
    - [x] `Paused` can resume back to `ExpeditionSummary`.
- [x] **Support Loading hops for summary-related transitions**
  - [x] Add helper `UFCGameStateManager::TransitionViaLoading(EFCGameStateID TargetState)` to support flows that go through `Loading` without losing the intended target state.
- [x] **Controller handling for `ExpeditionSummary`**
  - [x] In `AFCPlayerController::OnGameStateChanged`, handle `ExpeditionSummary` by:
    - [x] Setting input mode to the static-scene mapping (`EFCInputMappingMode::StaticScene`).
    - [ ] (Future) Applying a dedicated table-view camera blend helper once 4.1.1 is implemented.
- [x] **ESC / Close behaviour for `ExpeditionSummary`**
  - [x] Replace `HandleReturnToOfficeClicked` with a `CloseSummary()` function on `UFCExpeditionSummaryWidget` that delegates to `UFCLevelTransitionManager::CloseExpeditionSummaryAndReturnToOffice()`.
  - [x] Implement `UFCLevelTransitionManager::CloseExpeditionSummaryAndReturnToOffice()` so it:
    - [x] Validates current state (logs if not `ExpeditionSummary`).
    - [x] Transitions back to `Office_Exploration` and lets `AFCPlayerController::OnGameStateChanged` restore office camera/input.
  - [x] Extend ESC handling in `AFCPlayerController::HandlePausePressed` so that pressing ESC while in `ExpeditionSummary` calls `CloseExpeditionSummaryAndReturnToOffice()` instead of toggling pause.

- [ ] **Overworld → Office → ExpeditionSummary flow (final wiring)**
  - [x] Implement `UFCLevelTransitionManager::ReturnFromOverworldToOfficeWithSummary()` to:
    - [x] Validate current state is `Overworld_Travel`.
    - [x] Call `TransitionViaLoading(EFCGameStateID::ExpeditionSummary)`.
    - [x] Trigger fade-out and load `L_Office` via `UFCLevelManager`.
  - [ ] Implement `UFCLevelTransitionManager::InitializeLevelTransitionOnLevelStart()` and call it from Office level startup so that when entering `L_Office` with a pending `ExpeditionSummary` target it:
    - [ ] Transitions the game state into `ExpeditionSummary`.
    - [ ] Asks `UFCUIManager` to show the expedition summary widget.
  - [ ] Add `UFCUIManager::ShowExpeditionSummary()` as a helper that spawns `WBP_ExpeditionSummary` and passes current expedition result data into it.
  - [ ] **Verify**: Summary is only ever shown in Office, fully blocks world interaction while open, and both ESC and the close button reliably return to normal office gameplay.

---

### Step 4.6: Centralize Level & State Transitions (FCLevelTransitionManager)

_Goal: Move all complex "state + level + UI" transitions into `UFCLevelTransitionManager`, removing ad-hoc logic from widgets and the player controller, and ensuring old code paths are cleaned up._

- [x] **4.6.1: Define Responsibilities & API of `UFCLevelTransitionManager`**
  - [x] Open `Source/FC/Core/FCLevelTransitionManager.h/.cpp`.
  - [x] Define `UFCLevelTransitionManager` as a `UGameInstanceSubsystem` that coordinates:
    - [x] Game state changes (`UFCGameStateManager`).
    - [x] Level loads (`UFCLevelManager` / `UGameplayStatics::OpenLevel`).
    - [x] Screen fades / transitions (`UFCTransitionManager`).
    - [x] High-level UI screen entry points (`UFCUIManager`, e.g. pause, main menu, expedition summary).
  - [x] Expose a clear, minimal C++ API for flows used in Week 4:
    - [x] `void StartExpeditionFromOfficeTableView();` (Office_TableView  Overworld_Travel).
    - [x] `void ReturnToMainMenuFromGameplay();` (any gameplay/overworld  Main Menu in Office).
    - [x] `void ReturnFromOverworldToOfficeWithSummary();` (Overworld_Travel  ExpeditionSummary in Office).
  - [x] Document these responsibilities in a short comment block in the header and add a reminder that **old transition code in other classes must be removed when moved here**.

- [x] **4.6.2: Refactor Start Expedition Flow to use `UFCLevelTransitionManager`**
  - [x] Open `Source/FC/UI/FCExpeditionPlanningWidget.cpp` and locate the existing `OnStartExpeditionClicked` implementation.
  - [x] Replace its direct calls to `UFCGameStateManager::TransitionTo(Overworld_Travel)` and `UFCLevelManager::LoadLevel` / custom fade logic with a single call to `UFCLevelTransitionManager::StartExpeditionFromOfficeTableView()`.
  - [x] In `StartExpeditionFromOfficeTableView()`:
    - [x] Validate the current game state is `Office_TableView` and that the planned expedition data is valid (via `UFCExpeditionManager`).
    - [x] Call `UFCGameStateManager::TransitionViaLoading(EFCGameStateID::Overworld_Travel)`.
    - [x] Trigger fade-out using `UFCTransitionManager`.
    - [x] Use `UFCLevelManager` (or `OpenLevel`) to load the overworld map level defined by the expedition data.
  - [x] **Remove** any now-duplicated or obsolete level/state transition code from `FCExpeditionPlanningWidget.cpp` to avoid two competing implementations.

- [x] **4.6.3: Refactor "Return to Main Menu" to use `UFCLevelTransitionManager`**
  - [x] Inspect `AFCPlayerController::ReturnToMainMenu` in `Source/FC/Core/FCPlayerController.cpp`.
  - [x] Introduce a new method `UFCLevelTransitionManager::ReturnToMainMenuFromGameplay()` that:
    - [x] Uses `UFCGameStateManager::TransitionViaLoading(EFCGameStateID::MainMenu)` or the appropriate menu state.
    - [x] Hides the pause menu via `UFCUIManager` if necessary.
    - [x] Starts a fade-out via `UFCTransitionManager`.
    - [x] Loads `L_Office` (main menu lives in the office map).
  - [x] Change call sites (e.g. pause menu button logic, controller helper) to call `ReturnToMainMenuFromGameplay()` instead of implementing their own fade + `OpenLevel("L_Office")` sequence.
  - [x] **Remove** the direct `UGameplayStatics::OpenLevel(this, FName(TEXT("L_Office")))` call from `AFCPlayerController::ReturnToMainMenu` once the new flow is verified.

- [x] **4.6.4: Implement Overworld  Office + Expedition Summary via `UFCLevelTransitionManager` (first stage)**
  - [x] Add `ReturnFromOverworldToOfficeWithSummary()` to `UFCLevelTransitionManager`.
  - [x] In this method:
    - [x] Require that the current game state is `Overworld_Travel`.
    - [x] Use `UFCGameStateManager::TransitionViaLoading(EFCGameStateID::ExpeditionSummary)` to enter a `Loading` hop destined for the summary state.
    - [x] Trigger fade-out via `UFCTransitionManager`.
    - [x] Use `UFCLevelManager` to load `L_Office` (or `OfficeMap` from `UFCGameInstance` data).
  - [ ] Wire Office-level startup (see new 4.6.5) so that after `L_Office` is loaded, the state moves into `ExpeditionSummary` and shows the summary widget.
  - [ ] Update `BP_ExtractionPoint` (or its interaction handler) so it calls only this function (`ReturnFromOverworldToOfficeWithSummary()`) instead of making any direct state/level/UI calls itself.

- [x] **4.6.5: Initialize Level Transitions on Level Start**
  - [x] Add `InitializeLevelTransitionOnLevelStart()` to `UFCLevelTransitionManager`.
  - [x] Call it from level startup logic (e.g. Office Level Blueprint, Office GameMode BeginPlay, and any other core levels).
  - [ ] In this function:
    [x] Read current `EFCGameStateID` from `UFCGameStateManager` and the current map name.
    [x] If current map is `L_Office` and the last hop used `TransitionViaLoading(ExpeditionSummary)`, transition into `ExpeditionSummary` and call `UFCUIManager::ShowExpeditionSummary()`.
    [ ] If current map is `L_Office` and state is `MainMenu` at PIE start, call into `FCPlayerController` / `UFCUIManager` to initialize the main menu flow (camera to menu, show main menu widget).
    [ ] Add additional cases for other level setups in the existing game flow as they are migrated (e.g. direct Office_Exploration start, Overworld-only test maps).

- [x] **4.6.6: Hook Summary Widget & ESC Behaviour into the Centralized Flow (remaining pieces)**
  - [x] Extend `UFCUIManager` with a helper like `ShowExpeditionSummary(APlayerController* OwningPlayer)` that:
    - [x] Spawns `WBP_ExpeditionSummary` (parent `UFCExpeditionSummaryWidget`).
    - [ ] Optionally pulls the latest expedition result data from `UFCExpeditionManager` and forwards it into the widget via a Blueprint-exposed function or `BlueprintImplementableEvent`.
  - [x] Ensure `AFCPlayerController::OnGameStateChanged` for `EFCGameStateID::ExpeditionSummary` remains responsible only for camera/input (table-view camera + static-scene IMC), while `UFCLevelTransitionManager` / `UFCUIManager` handle widget creation.
  - [ ] Note: post-summary target state (`Office_TableView` vs `Office_Exploration`) may still be refined in a later table-view refactor.

- [ ] **4.6.7: Cleanup & Remove Old Transition Code**
  - [ ] Search the codebase for any remaining ad-hoc sequences that combine:
    - [ ] `OpenLevel("L_Office")` / direct level name strings.
    - [ ] Manual `GameStateManager->TransitionTo(...)` around level loads.
    - [ ] Direct fade calls in widgets or controller that duplicate `UFCTransitionManager` responsibilities.
  - [ ] For each such site, either:
    - [ ] Replace it with a call into `UFCLevelTransitionManager`.
    - [ ] Or explicitly document why it remains a special case.
  - [ ] Remove dead/obsolete code paths once the new centralized flows are validated (to prevent future regressions and ensure only one implementation of each transition exists).

---

### Step 4.7: Align Core Managers & Introduce Reusable Transition Pattern

_Goal: Restore explicit game state transitions for Office table view and unify how state, level, UI, and camera/input transitions are coordinated across core managers (GameInstance, GameStateManager, LevelManager, TransitionManager, UIManager, LevelTransitionManager, PlayerController)._

- [x] **4.7.1: Document Desired Transition Responsibilities**
  - [x] **Design**:
    - [x] In `Docs/Technical_Documentation.md`, add a short “Transition Responsibilities” section that states:
      - [x] `UFCGameStateManager`: pure game state machine and validated transitions (no level or UI knowledge).
      - [x] `UFCLevelManager`: level identity, metadata, and low-level level loading (`LoadLevel`, `OpenLevel` call site).
      - [x] `UFCTransitionManager`: fades and loading overlays, no game-logic decisions.
      - [x] `UFCUIManager`: widget lifecycle and high-level screens (main menu, pause, summary, table, overworld HUD).
      - [x] `UFCLevelTransitionManager`: orchestration of multi-step flows (state + level + fade + UI entry).
      - [x] `AFCPlayerController` / `UFCCameraManager`: react to state changes to set camera + input modes, no direct level loads.
  - [x] **Implementation Tasks**:
    - [x] Add the new section to `Docs/Technical_Documentation.md` under the existing GameInstance / State / UI chapters.
    - [ ] Cross-link from the `UFCLevelTransitionManager` and `UFCGameStateManager` sections to this responsibility overview.
    - [ ] Add a short comment block to `FCLevelTransitionManager.h` summarizing its orchestrator role (matching the doc text).

- [ ] **4.7.2: Design a Reusable Transition Helper API**
  - [x] **Design**:
    - [x] Specify a small API on `UFCLevelTransitionManager` such as:
      - [x] `void RequestTransition(EFCGameStateID TargetState, FName TargetLevelName = NAME_None, bool bUseFade = true);`
      - [x] `void EnterOfficeTableView(AActor* TableActor);`
      - [x] `void ExitOfficeTableView();`
      - [x] `void InitializeOnLevelStart();` (general front-end for the existing `InitializeLevelTransitionOnLevelStart` logic).
    - [x] Define rules:
      - [x] `RequestTransition` is the only place that combines state + level + fade; it delegates to `UFCGameStateManager`, `UFCLevelManager`, and `UFCTransitionManager`.
      - [x] Flows that don’t change level (e.g., only camera/input) still go through LevelTransitionManager for consistency (no direct `TransitionTo` from UI/PC).
  - [ ] **Implementation Tasks**:
    - [x] Add these function declarations (or a subset) to `FCLevelTransitionManager.h` with TODO comments referencing Step 4.7.
    - [x] Add stub implementations in `FCLevelTransitionManager.cpp` that log and early-return.
    - [x] Update `Docs/Technical_Documentation.md` with a small “Transition API” diagram showing how requests flow into LevelTransitionManager.

- [x] **4.7.3: Plan Table View State Flow**
  - [x] **Design**:
    - [x] Define the desired state transitions:
      - [x] `Office_Exploration → Office_TableView` when interacting with the office table.
      - [x] `Office_TableView → Office_Exploration` when closing the table (ESC with no widget open, or an explicit back action).
    - [x] Define trigger points:
      - [x] Entry: `AFCPlayerController::OnTableObjectClicked` calls `UFCLevelTransitionManager::EnterOfficeTableView(TableActor)`.
      - [x] Exit: `AFCPlayerController::HandlePausePressed` calls `UFCLevelTransitionManager::ExitOfficeTableView()` instead of directly calling `TransitionTo(Office_Exploration)` + `SetCameraModeLocal`.
    - [x] Document expected camera/input modes:
      - [x] `Office_Exploration`: FirstPerson camera, exploration IMC.
      - [x] `Office_TableView`: TableView camera, static-scene/input-for-UI IMC.
      - [x] `ExpeditionSummary`: table-view camera + summary widget blocking input (just like now).
  - [x] **Implementation Tasks**:
    - [x] In `Docs/Technical_Documentation.md`, add a small “Office Table View Flow” subsection with a simple state diagram and the entry/exit triggers.
    - [x] Add TODO comments in `AFCPlayerController::OnTableObjectClicked` and the table-view branch of `HandlePausePressed` pointing to `EnterOfficeTableView` / `ExitOfficeTableView` (without changing behavior yet).
    - [x] Add stub functions `EnterOfficeTableView` / `ExitOfficeTableView` in `FCLevelTransitionManager.cpp` that currently just log and return.

- [x] **4.7.4: Plan Startup & Main Menu Initialization Flow**
  - [x] **Design**:
    - [x] Define a unified startup path:
      - [x] Office `BeginPlay` (via GameMode or Level BP) calls `UFCLevelTransitionManager::InitializeOnLevelStart()`.
      - [x] `InitializeOnLevelStart()` internally:
        - [x] If current state is `None`: transition to `MainMenu`, call a “menu setup” helper (camera to menu + show main menu widget).
        - [x] If current state is `Loading` with target `ExpeditionSummary` and level is `L_Office`: run the existing summary path (already implemented in `InitializeLevelTransitionOnLevelStart`).
        - [x] For other states (e.g., `Office_Exploration` loaded from save): call a helper to restore camera/input without re-showing the main menu.
    - [x] Decide whether `AFCPlayerController::InitializeMainMenu` remains as an internal helper (called only from LevelTransitionManager) or is deprecated.
  - [x] **Implementation Tasks**:
    - [x] In `Docs/Technical_Documentation.md`, update the “L_Office dual purpose” section to describe the new `InitializeOnLevelStart` orchestration instead of the Level BP calling `InitializeMainMenu` directly.
    - [x] Add a TODO comment near the existing `UFCLevelTransitionManager::InitializeLevelTransitionOnLevelStart` noting it will be folded into `InitializeOnLevelStart` in a later task.
    - [x] Add a brief comment above `AFCPlayerController::InitializeMainMenu` marking it as “called only from LevelTransitionManager in future (see 4.7.4)” without changing call sites yet.

- [x] **4.7.5: Create Implementation Checklists for Future Refactor**
  - [x] **Design**:
    - [x] Enumerate where code should change once the new API is ready, without touching behavior now.
  - [x] **Implementation Tasks**:
    - [x] In `0004-tasks.md` or a new small section in `Docs/Technical_Documentation.md`, create a checklist of call sites to migrate:
      - [x] `AFCPlayerController::OnTableObjectClicked` (table view entry).
      - [x] `AFCPlayerController::HandlePausePressed` (table view exit and ExpeditionSummary ESC path).
      - [x] `AFCPlayerController::InitializeMainMenu` and `TransitionToGameplay`.
      - [x] `UFCGameInstance::LoadGameAsync` cross-level transitions (decide whether to route via `RequestTransition` or document as a save/load-special case).
      - [x] Any remaining direct `UGameplayStatics::OpenLevel` usages outside `UFCLevelManager` that are gameplay-related.
    - [x] For each item, add a one-liner: “Replace with `LevelTransitionManager::RequestTransition(...)` once 4.7 API is implemented” or “Document as a special-case path (save/load only).”.

_Goal: Restore explicit game state transitions for Office table view and unify how state, level, UI, and camera/input transitions are coordinated across core managers (GameInstance, GameStateManager, LevelManager, TransitionManager, UIManager, LevelTransitionManager, PlayerController)._

---
