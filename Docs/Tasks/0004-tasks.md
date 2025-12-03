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

  - [ ] Add **Button**: `Button_PreviewRoute` with text "Preview Route"
  - [ ] Position: Near start point panel or top bar
  - [ ] Add **Button**: `Button_StopPreview` with text "Stop Preview"
  - [ ] Set Visibility: `Collapsed` (show when preview running)

- [ ] **Resource Display (Top Bar)**

  - [ ] Add **Horizontal Box**: `Panel_Resources` at top of screen
  - [ ] Add **Text Block**: `Text_PlayerMoney` (format: "Gold: {Money}")
  - [ ] Add **Text Block**: `Text_PlayerSupplies` (format: "Supplies: {Supplies}")
  - [ ] Set font size: 18-24, bold
  - [ ] **Purpose**: Show if player can afford selected route

- [ ] **Close Button**
  - [ ] Add **Button**: `Button_Close` in top-right corner
  - [ ] Add **Text Block** child: "X" or "Close"
  - [ ] Will call cleanup and close widget

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
  - [ ] Add node: `Get Game Instance → Cast to UFCGameInstance`
  - [ ] Get `GameStateData` (assuming you have `FFCGameStateData` struct with Money/Supplies)
  - [ ] Format string: "Gold: {0}" with `GameStateData.Money`
  - [ ] Set `Text_PlayerMoney → Set Text`
  - [ ] Format string: "Supplies: {0}" with `GameStateData.Supplies`
  - [ ] Set `Text_PlayerSupplies → Set Text`

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

  - [ ] Add node: `ExpeditionManager → GetCurrentExpedition`
  - [ ] **Branch**: Validate `PlannedRouteGlobalIds.Num() > 0` AND resources sufficient
    - [ ] **True Path**:
      - [ ] Add node: `UFCGameInstance → DeductResources` (custom function to subtract Money/Supplies)
        - [ ] Amount: `PlannedMoneyCost`
      - [ ] Add node: `Get Game Instance → UFCGameStateManager → TransitionTo`
        - [ ] New State: `Overworld_Travel` (or appropriate enum)
      - [ ] Add node: `UFCLevelManager → LoadLevel`
        - [ ] Level Name: Get from expedition data (e.g., "L_Overworld_Forest")
      - [ ] Add node: `Remove from Parent` (close widget)
      - [ ] Add node: `Print String` → "Expedition started!" (green)
    - [ ] **False Path**:
      - [ ] Add node: `Print String` → "ERROR: Cannot start expedition (invalid route or insufficient resources)" (red)

- [ ] **Button_Close → OnClicked Event**
  - [ ] Add node: `ExpeditionManager → WorldMap_ClearRoutePreview()` (cleanup)
  - [ ] Add node: `Remove from Parent`
  - [ ] **Note**: Planning state persists in UFCExpeditionManager subsystem

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

- [ ] **Locate Overworld Convoy Pawn**

  - [ ] Navigate to `Content/FC/Blueprints/Overworld/` (or similar)
  - [ ] Open `BP_ConvoyPawn` (or `BP_PlayerOverworld`, or your convoy actor)
  - [ ] Switch to **Event Graph**

- [ ] **Create Variable: CachedExpeditionManager**

  - [ ] Add variable: `CachedExpeditionManager` (Type: `UFCExpeditionManager`, Object Reference)
  - [ ] Set Instance Editable: false
  - [ ] Tooltip: "Cached subsystem reference for performance"

- [ ] **Event BeginPlay - Cache Subsystem**

  - [ ] Find `Event BeginPlay` node
  - [ ] Add node: `Get Game Instance → Cast to UFCGameInstance`
  - [ ] Add node: `Get Subsystem` (Class: `UFCExpeditionManager`)
  - [ ] Set `CachedExpeditionManager` variable
  - [ ] **Branch**: Check if valid
    - [ ] **True**: Print "Convoy: Expedition Manager connected"
    - [ ] **False**: Print "ERROR: Convoy failed to get Expedition Manager" (red)

- [ ] **Event Tick - Record Exploration**

  - [ ] Find `Event Tick` node (or create if doesn't exist)
  - [ ] **Optimization Gate**: Add `DoOnce` or timer (optional, for reducing frequency)
    - [ ] If you want to throttle updates, add `Delay` node (0.1-0.5 seconds) or check distance moved
  - [ ] **Validation Branch**: Check all conditions:
    - [ ] `CachedExpeditionManager` Is Valid
    - [ ] AND `CachedExpeditionManager → IsExpeditionActive()` (returns bool)
    - [ ] AND Current Level Name contains "Overworld" (use `Get Current Level Name` → `Contains`)
  - [ ] **True Path**:
    - [ ] Add node: `Get Actor Location` (returns FVector world position)
    - [ ] Add node: `CachedExpeditionManager → WorldMap_RecordVisitedWorldLocation`
      - [ ] WorldLocation: Connected to `Get Actor Location`
    - [ ] **Optional Debug**: Add `Print String` → "Recording exploration at {location}" (very verbose, comment out for release)

- [ ] **Throttling Optimization (Optional but Recommended)**

  - [ ] Create variable: `LastRecordedGlobalCell` (Type: `int32`, default: -1)
  - [ ] Before calling `WorldMap_RecordVisitedWorldLocation`:
    - [ ] Add node: `CachedExpeditionManager → WorldLocationToGlobalCell` (helper function, if exists)
      - [ ] If not exists: Calculate manually: `Floor(WorldLocation.X / CellSize) + Floor(WorldLocation.Y / CellSize) * 16`
    - [ ] **Branch**: Check if `CurrentGlobalCell != LastRecordedGlobalCell`
      - [ ] **True**: Call `WorldMap_RecordVisitedWorldLocation`, set `LastRecordedGlobalCell = CurrentGlobalCell`
      - [ ] **False**: Skip (already recorded this cell)
  - [ ] **Purpose**: Avoid calling subsystem hundreds of times per second in same cell

- [ ] **Event EndPlay - Cleanup**
  - [ ] Add `Event EndPlay` node
  - [ ] Set `CachedExpeditionManager` = None (clear reference)
  - [ ] **Optional**: Call `ClearTimer` if you used timer-based throttling

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

- [ ] **Introduce `EFCGameStateID::ExpeditionSummary` and wire transitions**
  - [ ] In `EFCGameStateID` (GameStateManager), add a new value `ExpeditionSummary`.
  - [ ] Ensure `UFCGameStateManager::TransitionTo` only allows `ExpeditionSummary` when the current level is `L_Office` and the previous state was `Paused` or `Overworld_Travel`.
  - [ ] In `AFCPlayerController::ApplyInputAndCameraModeForState`, add handling for `ExpeditionSummary`:
    - [ ] Set camera to TableView (same camera mode used for map/desk widgets).
    - [ ] Set input mode to Game and UI; apply the static scene input mapping context (IMC for table/static scene).
  - [ ] Update the return flow from Overworld so that, after loading `L_Office`, the game transitions into `ExpeditionSummary` instead of directly into `Office_Exploration` and shows `WBP_ExpeditionSummary`.
  - [ ] Implement ESC handling (and optional "Close Summary" button) so that both:
    - [ ] Transition from `ExpeditionSummary` back to `Office_Exploration`.
    - [ ] Restore first-person camera/input in office (no cursor, default office IMC).
  - [ ] **Verify**: Summary is only ever shown in Office, fully blocks world interaction while open, and both ESC and the close button reliably return to normal office gameplay.

---

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

### Step 4.4: Integration & Testing

#### Step 4.4.1: Full Loop Verification (Priorities 1–4)

- [ ] **Unified Test Sequence (P1–P4)**
  1.  Start in Office.
  2.  Open Map (Planning Mode) -> Select Route -> Check Costs -> Start Expedition.
  3.  Load into Overworld.
  4.  Move around and confirm fog-of-war updates (Priority 1).
  5.  Press 'M' -> Check Map (View-Only Overworld Map, Priority 3).
  6.  Press 'ESC' -> Check Pause Menu (Abort Button Visible when in Overworld, ESC closes menu; Priority 1 bugfixes).
  7.  Go to `BP_ExtractionPoint` -> Trigger Return (Priority 4).
  8.  View Expedition Summary -> Click Return (Priority 4).
  9.  Load into Office -> **Verify Input/State is correct (First Person, no cursor issues, Priority 1+4)**.

#### Step 4.4.2: Documentation

- [ ] Update `Docs/Technical_Documentation.md` with new Map System architecture.
- [ ] Mark Issues #1, #2, #4 as Resolved in `backlog.md`.


---


#### Step 4.5: Testing & Validation (Full Map System Loop)

- [ ] **Test 1: Office Map Table Interaction**

  - [ ] PIE in Office level
  - [ ] Interact with `BP_TableObject_Map` (should open `WBP_WorldMap`)
  - [ ] Verify fog overlay renders (should be mostly black/unexplored)
  - [ ] Verify route overlay is empty (no red cells initially)
  - [ ] Check Output Log for "WorldMap: Expedition Manager connected"

- [ ] **Test 2: Grid Selection & Route Preview**

  - [ ] Click `Button_Grid24`
  - [ ] Verify `Panel_StartPointSelection` becomes visible
  - [ ] Verify `Text_MoneyCost` and `Text_RiskCost` show numbers > 0
  - [ ] Check if `Button_StartExpedition` is enabled (depends on resources)
  - [ ] Click `Button_PreviewRoute`
  - [ ] Watch route overlay for 3 seconds (red cells should appear progressively)
  - [ ] Verify `Button_PreviewRoute` is disabled during animation
  - [ ] After 3 seconds, verify button re-enables

- [ ] **Test 3: Start Expedition & Level Transition**

  - [ ] Click `Button_StartExpedition` (if enabled)
  - [ ] Verify level transitions to Overworld (e.g., `L_Overworld_Forest`)
  - [ ] Verify game state changes (check HUD, player controller mode)
  - [ ] Check Output Log for "Expedition started!" message

- [ ] **Test 4: Overworld Exploration Recording**

  - [ ] In Overworld, move convoy pawn around map
  - [ ] Check Output Log (VeryVerbose filter):
    - [ ] Look for "Recording exploration at..." messages (if debug enabled)
    - [ ] Look for "Expedition data autosaved" messages (every ~1 second)
  - [ ] Move to different areas of map (change grid cells)
  - [ ] Verify logs show different global cell indices

- [ ] **Test 5: Fog of War Update Verification**

  - [ ] After moving in Overworld, pause game
  - [ ] Open Console (`key), type:`open L_Office` (or use extraction point)
  - [ ] Return to Office, interact with map table again
  - [ ] **Verify**: Fog overlay now shows cleared cells (white/transparent) where convoy traveled
  - [ ] **If fog doesn't update**: Check delegate binding, check autosave logs, check texture CPU access

- [ ] **Test 6: Land Mask Validation**

  - [ ] In Office, open map widget
  - [ ] Try to select a grid cell that should be water/blocked (if you added multiple buttons)
  - [ ] Verify `WorldMap_SelectGridArea` returns false (check Output Log for failure message)
  - [ ] Verify `Panel_StartPointSelection` stays hidden

- [ ] **Test 7: Resource Validation**

  - [ ] In Office, cheat money to low value (use Console: `GiveGold 10` or modify GameInstance in debugger)
  - [ ] Select Grid 24 (cost should exceed 10 gold)
  - [ ] Verify `Text_MoneyCost` displays in red color
  - [ ] Verify `Button_StartExpedition` is disabled
  - [ ] Restore money, verify button enables

- [ ] **Test 8: Widget Cleanup (Memory Leak Check)**

  - [ ] Open map widget, close via `Button_Close`
  - [ ] Re-open map widget
  - [ ] Verify no errors in Output Log
  - [ ] Close editor, check log for "Unbound from OnWorldMapChanged delegate" (confirms cleanup)

- [ ] **Bug Fixes & Polish**

  - [ ] If fog texture doesn't update: Check `OnWorldMapChanged` delegate broadcast in C++ subsystem
  - [ ] If route preview stutters: Ensure timer duration matches `WorldMap_BeginRoutePreviewPaint` paint duration (3.0s)
  - [ ] If material parameters don't update: Verify parameter names exactly match between material and Blueprint (`FogTexture`, `RouteTexture`)
  - [ ] If land mask fails to load: Re-import texture with "Allow CPU Access" enabled

- [ ] **Performance Profiling (Optional)**
  - [ ] In PIE, open Console → type `stat fps`
  - [ ] Move convoy in Overworld with exploration recording active
  - [ ] Verify FPS stays above 30 (if lower, implement throttling from Step 4.2.9)
  - [ ] Open map widget, verify UI renders smoothly (fog/route overlays should be single draw call each)
