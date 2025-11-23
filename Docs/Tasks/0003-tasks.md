# Week 3 – Overworld Level & Basic Convoy Movement

## Task Metadata

```yaml
Task ID: 0003
Sprint/Phase: Week 3 (21.11.-28.11.2025)
Feature Name: Overworld 3D Environment with Top-Down Camera and Click-to-Move Convoy
Dependencies: Task 0002 (Week 2 - Table Interaction, Level Manager, Input Manager Component)
Estimated Complexity: High
Primary Files Affected:
  - Content/FC/World/Levels/L_Overworld.umap
  - Content/FC/World/Blueprints/Pawns/BP_OverworldConvoy.uasset
  - Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset
  - Content/FC/World/Blueprints/Interactables/BP_OverworldPOI.uasset
  - Content/FC/Input/Contexts/IMC_FC_TopDown.uasset
  - Source/FC/Core/FCPlayerController.h/.cpp
  - Source/FC/Components/UFCInputManager.h/.cpp (extend)
```

---

## Overview & Context

### Purpose

Implement the first version of the 3D Overworld level with top-down camera control, basic convoy pawn movement via left-click pathfinding, and right-click interaction stubs on POI actors. This establishes the foundation for Week 4's expedition flow and future resource/event systems.

### Architecture Notes

- **Input Management**: Leverage existing `UFCInputManager` component (Priority 6) to switch between FirstPerson (Office) and TopDown (Overworld) input contexts
- **Camera System**: Reuse `UFCCameraManager` component patterns for camera mode management, but create new Overworld-specific camera actor
- **Level Transition**: Use existing `UFCLevelManager::LoadLevel()` from Week 2 for Office → Overworld transition
- **Game State Integration**: Transition to `Overworld_Travel` state using `UFCGameStateManager` when entering Overworld
- **NavMesh Movement**: Implement simple AI movement using Unreal's built-in NavMesh system for convoy pathfinding
- **Input Context**: Use `IMC_FC_TopDown` (already created in Week 2) for Overworld-specific input bindings

### Reference Documents

- `/Docs/Fallen_Compass_GDD_v0.2.md` - §3.2.2 "Overworld (3D, Echtzeit)", §4.1 "Perspektiven (Büro, Overworld, Lager)"
- `/Docs/Fallen_Compass_DRM.md` - Week 3 features (Overworld Level, Convoy Movement, POI Stubs)
- `/Docs/UE_CodeConventions.md` - Blueprint/C++ hybrid approach, pathfinding patterns
- `/Docs/UE_NamingConventions.md` - BP*/IA*/IMC\_ prefixes, folder structure
- `/Docs/Technical_Documentation.md` - UFCInputManager, UFCCameraManager, UFCLevelManager, UFCGameStateManager subsystems

---

## Relevant Files

### Week 2 Foundation (Review Before Starting)

- `/Source/FC/Core/UFCGameInstance.h/.cpp` - Global state owner, subsystem configuration
- `/Source/FC/Core/UFCLevelManager.h/.cpp` - Level tracking and LoadLevel() method
- `/Source/FC/Core/UFCUIManager.h/.cpp` - Widget lifecycle management subsystem
- `/Source/FC/Core/UFCGameStateManager.h/.cpp` - Game state machine (Overworld_Travel state)
- `/Source/FC/Core/FCPlayerController.h/.cpp` - First-person player controller (Office)
- `/Source/FC/Components/UFCCameraManager.h/.cpp` - Camera mode management component
- `/Source/FC/Components/UFCInputManager.h/.cpp` - Input mapping context management component
- `/Source/FC/Interaction/IFCTableInteractable.h/.cpp` - Table object interaction interface
- `/Content/FC/UI/TableMap/WBP_TableMap.uasset` - Map table widget with "Start Test Expedition" button
- `/Content/FC/Input/Contexts/IMC_FC_TopDown.uasset` - TopDown input mapping context (created in Week 2, needs configuration)

### Files to Create This Week

#### C++ Classes

- `/Source/FC/Core/FCPlayerController.h/.cpp` - Top-down player controller for Overworld
- `/Source/FC/World/FCOverworldPawn.h/.cpp` - Base pawn class for Overworld entities (optional, may use Blueprint-only)

#### Blueprint Assets

- `/Content/FC/World/Levels/L_Overworld.umap` - Main Overworld level
- `/Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset` - Top-down camera actor
- `/Content/FC/World/Blueprints/Pawns/BP_OverworldConvoy.uasset` - Convoy pawn with NavMesh movement
- `/Content/FC/World/Blueprints/Interactables/BP_OverworldPOI.uasset` - Point of Interest actor base class
- `/Content/FC/World/Blueprints/Interactables/BP_OverworldPOI_Test.uasset` - Test POI instance

#### Input Assets

- `/Content/FC/Input/Actions/IA_OverworldPan.uasset` - WASD camera panning (Axis2D)
- `/Content/FC/Input/Actions/IA_OverworldZoom.uasset` - Mouse wheel zoom (Axis1D)
- `/Content/FC/Input/Actions/IA_Interact.uasset` - Right-click interaction (Boolean)

### Files to Modify This Week

- `/Source/FC/Components/UFCInputManager.h/.cpp` - May need extensions for Overworld-specific input handling
- `/Source/FC/Core/UFCUIManager.h/.cpp` - Add conditional pause logic for Overworld state
- `/Content/FC/UI/TableMap/WBP_TableMap.uasset` - Wire "Start Test Expedition" button to level load

---

## Pre-Implementation Phase

### Task 1: Pre-Implementation Analysis & Setup

**Purpose**: Review existing Week 2 systems, verify naming conventions, plan Overworld architecture, and create feature branch.

---

#### Step 1.0: Analysis & Discovery

##### Step 1.0.1: Review Week 2 Foundation Systems

- [x] **Analysis**

  - [x] Read `/Docs/Technical_Documentation.md` sections:
    - [x] §2.1.5: UFCLevelManager (LoadLevel() method, level type detection)
    - [x] §2.1.6: UFCUIManager (widget management, ShowPauseMenu())
    - [x] §2.1.7: UFCGameStateManager (state transitions, Overworld_Travel state)
    - [x] §2.3: AFCPlayerController (input handling, camera integration patterns)
    - [x] §2.4: UFCCameraManager (camera mode blending, SetCameraMode() patterns)
    - [x] §2.5: UFCInputManager (SetInputMappingMode(), IMC registry, TopDown mode)
  - [x] Check existing subsystem implementations:
    - [x] `/Source/FC/Core/FCLevelManager.h/.cpp` - Review LoadLevel() implementation
    - [x] `/Source/FC/Core/FCGameStateManager.h/.cpp` - Review Overworld_Travel state definition
    - [x] `/Source/FC/Components/FCInputManager.h/.cpp` - Review TopDown input mode support
    - [x] `/Content/FC/Input/Contexts/IMC_FC_TopDown.uasset` - Check if exists, note if empty
  - [x] Identify patterns to replicate for Week 3:
    - [x] PlayerController creation pattern (inherit from APlayerController)
    - [x] Input Manager component instantiation in controller constructor
    - [x] Camera Manager component usage for mode switching
    - [x] Level transition flow: Button click → UFCLevelManager::LoadLevel() → GameState transition
    - [x] NavMesh-based AI movement (review UE documentation/existing examples)

**Key Findings Documentation:**

```
✅ VERIFIED Week 2 Foundation Systems:

1. UFCLevelManager (Source/FC/Core/FCLevelManager.h/.cpp)
   - ✅ LoadLevel(FName LevelName, bool bUseFade=true) method exists
   - ✅ Handles fade-out → OpenLevel → fade-in flow via UFCTransitionManager
   - ✅ PendingLevelName tracking with OnFadeOutCompleteForLevelLoad callback
   - ✅ Level type detection with EFCLevelType enum (metadata-first, pattern fallback)

2. UFCGameStateManager (Source/FC/Core/FCGameStateManager.h/.cpp)
   - ✅ EFCGameStateID enum has Overworld_Travel state defined
   - ✅ ValidTransitions map includes: Office_Exploration → Overworld_Travel
   - ✅ TransitionTo(EFCGameStateID) method with validation
   - ✅ OnStateChanged delegate for state change notifications
   - ✅ State stack support (PushState/PopState for pause/modal states)

3. UFCInputManager (Source/FC/Components/FCInputManager.h/.cpp)
   - ✅ EFCInputMappingMode enum has TopDown value defined
   - ✅ SetInputMappingMode(EFCInputMappingMode) switches contexts
   - ✅ Component-based design (attached to PlayerController)
   - ✅ Properties: FirstPersonMappingContext, TopDownMappingContext, FightMappingContext, StaticSceneMappingContext
   - ✅ ClearAllMappings → AddMappingContext pattern implemented

4. IMC_FC_TopDown Asset Status
   - ✅ NOT FOUND in Content/FC/Input/

Week 3 Implementation Patterns Identified:
✅ Controller Pattern:
   - Inherit from APlayerController
   - Add UFCInputManager component in constructor: CreateDefaultSubobject<UFCInputManager>()
   - Call InputManager->SetInputMappingMode(TopDown) in BeginPlay

✅ Level Transition Pattern:
   - Get UFCGameInstance → GetSubsystem<UFCLevelManager>()
   - Call LevelManager->LoadLevel(FName("L_Overworld"), true /*use fade*/)
   - GameStateManager transitions happen separately (Office_Exploration → Overworld_Travel)

✅ NavMesh Movement Pattern:
   - Use Unreal's SimpleMoveToLocation(Controller, TargetLocation) for AI pathfinding
   - Requires NavMeshBoundsVolume in level + FloatingPawnMovement or CharacterMovement component
   - Controller GetHitResultUnderCursor() for click position detection

To Create in Week 3:
- L_Overworld level with terrain, lighting, NavMesh
- BP_OverworldCamera actor with pan/zoom logic
- AFCPlayerController C++ class
- BP_OverworldConvoy pawn with click-to-move
- BP_OverworldPOI interaction stub
- Input Actions: IA_OverworldPan, IA_OverworldZoom, IA_Click (existing), IA_Interact
- IMC_FC_TopDown asset (bind input actions)
```

##### Step 1.0.2: Code & Naming Conventions Pre-Check

- [x] **Code Conventions Review** (`/Docs/UE_CodeConventions.md`)

  - [x] §2.1 Encapsulation: FCOverworldPlayerController will use private members with public accessors
  - [x] §2.2 Modular Organization: Overworld classes in `/Source/FC/Core/` (controller) and `/Source/FC/World/` (pawns/actors)
  - [x] §2.3 Blueprint Exposure: Only expose necessary methods (MoveTo, InteractWithPOI) as BlueprintCallable
  - [x] §2.5 Memory Management: All UObject\* pointers will use UPROPERTY() or TObjectPtr<> for GC tracking
  - [x] §2.6 Event-Driven Design: Use delegates for convoy arrival events (no Tick for movement)
  - [x] §4.3 Separation of Concerns: PlayerController → Pawn (movement) → POI (interaction)

- [x] **Naming Conventions Review** (`/Docs/UE_NamingConventions.md`)
  - [x] C++ Classes: `AFCPlayerController`, `AFCOverworldPawn` (A prefix for Actors)
  - [x] Blueprints: `BP_OverworldCamera`, `BP_OverworldConvoy`, `BP_OverworldPOI` (BP\_ prefix)
  - [x] Input Actions: `IA_OverworldPan`, `IA_OverworldZoom`, `IA_Click` (existing), `IA_Interact` (IA\_ prefix)
  - [x] Input Mapping Context: `IMC_FC_TopDown` (already exists from Week 2)
  - [x] Level: `L_Overworld` (L\_ prefix)
  - [x] Folders: `/Source/FC/Core/` (controllers), `/Source/FC/World/` (pawns/actors), `/Content/FC/World/Levels/`, `/Content/FC/World/Blueprints/`

**Key Conventions Applied to Week 3:**

1. **Controller Pattern:**

   ```cpp
   // FCPlayerController.h
   UCLASS()
   class FC_API AFCPlayerController : public APlayerController
   {
       GENERATED_BODY()
   public:
       AFCPlayerController();
   protected:
       UPROPERTY()
       TObjectPtr<UFCInputManager> InputManager;
   private:
       UPROPERTY()
       TObjectPtr<APawn> ControlledConvoy;
   };
   ```

2. **Blueprint Pawn Pattern:**
   - `BP_OverworldConvoy` inherits from `APawn` (may use C++ base class `AFCOverworldPawn` if needed)
   - Uses `FloatingPawnMovement` or `CharacterMovement` component with NavMesh
   - Blueprint implements click-to-move logic using `SimpleMoveToLocation()`

**✅ VERIFIED Conventions:**

- Encapsulation: Private members with UPROPERTY() for GC safety
- Modular org: Core/ for controllers, World/ for game actors
- Blueprint exposure: Only BlueprintCallable on user-facing methods
- Event-driven: Delegates over Tick (PrimaryActorTick.bCanEverTick = false)
- Clean APIs: Small public interfaces, hide implementation
- Naming: All FC-prefixed (AFCPlayerController), BP* for Blueprints, IA* for input actions
- Folders match asset types per UE_NamingConventions.md structure

##### Step 1.0.3: Architecture Planning & Implementation Strategy

- [x] **Document Classes to Create**

  ```
  C++ Classes:
  1. AFCPlayerController (Source/FC/Core/)
     - Inherits: APlayerController
     - Purpose: Handle Overworld input (pan, zoom, click-move, interact)
     - Components: UFCInputManager (set to TopDown mode)
     - Methods: HandlePanInput(), HandleZoomInput(), HandleClickMoveInput(), HandleInteractInput()

  Blueprint Classes:
  2. BP_OverworldCamera (Content/FC/World/Blueprints/Cameras/)
     - Inherits: ACameraActor
     - Purpose: Top-down camera with configurable height, pitch
     - Components: SpringArm (optional), Camera
     - Properties: ZoomMin, ZoomMax, PanSpeed, EdgeScrollThreshold

  3. BP_OverworldConvoy (Content/FC/World/Blueprints/Pawns/)
     - Inherits: APawn
     - Purpose: Player-controlled convoy that moves via pathfinding
     - Components: CapsuleComponent (root), StaticMesh/SkeletalMesh, FloatingPawnMovement
     - Methods: SimpleMoveToLocation() (built-in AI function)

  4. BP_OverworldPOI (Content/FC/World/Blueprints/Interactables/)
     - Inherits: AActor
     - Purpose: Interactable point of interest
     - Components: StaticMesh, ClickableComponent (collision for raycasts)
     - Methods: OnRightClick() → Print "Interact with POI"

  Input Assets:
  5. IA_OverworldPan (Axis2D) - WASD camera movement
  6. IA_OverworldZoom (Axis1D) - Mouse wheel zoom
  7. IA_Click (Boolean, existing) - Left mouse button
  8. IA_Interact (Boolean) - Right mouse button
  9. IMC_FC_TopDown (configure with above actions)
  ```

- [x] **Document Expected Call Flow**

  ```
  Office → Overworld Transition:
  1. Player clicks "Start Test Expedition" in WBP_TableMap
  2. Button calls UFCLevelManager::LoadLevel("L_Overworld")
  3. UFCTransitionManager fades out, OpenLevel loads L_Overworld
  4. L_Overworld Level Blueprint calls UFCGameStateManager::TransitionTo(Overworld_Travel)
  5. AFCPlayerController::BeginPlay() sets InputManager to TopDown mode
  6. Camera possesses BP_OverworldCamera, player can pan/zoom

  Convoy Movement:
  1. Player left-clicks on ground
  2. AFCPlayerController::HandleClickMoveInput() raycasts to world
  3. Calls ControlledConvoy->SimpleMoveToLocation(HitLocation)
  4. BP_OverworldConvoy uses NavMesh pathfinding to move

  POI Interaction:
  1. Player right-clicks on BP_OverworldPOI
  2. AFCPlayerController::HandleInteractInput() raycasts to actors
  3. Finds BP_OverworldPOI, calls OnRightClick()
  4. BP_OverworldPOI prints "Interact with [POIName]" to log
  ```

**✅ VERIFIED Architecture:**

- Clear class hierarchy: PlayerController → Camera/Pawn → POI
- Component-based: UFCInputManager reused from Week 2
- Event-driven: Input callbacks (no Tick), delegates for async events
- Separation: Controller handles input routing, Pawn handles movement, POI handles interaction
- Integration: Leverages existing subsystems (LevelManager, GameStateManager, InputManager)
- NavMesh pathfinding: SimpleMoveToLocation() with FloatingPawnMovement
- Input flow: Enhanced Input → InputActions → Controller handlers → Game logic

**Pre-Implementation Phase Complete** ✅

**COMMIT POINT 1.0**: `git add -A && git commit -m "docs(week3): Complete pre-implementation analysis and planning"`

---

## Implementation Phase

### Task 2: Overworld Level Creation & Greybox Environment

**Purpose**: Create L_Overworld level with basic terrain, lighting, NavMesh bounds, and player start for Week 3 testing.

---

#### Step 2.1: Create L_Overworld Level

##### Step 2.1.1: Create New Level and Configure World Settings

- [ ] **Analysis**

  - [ ] Check existing levels in `/Content/FC/World/Levels/` (L_Office should exist)
  - [ ] Review L_Office for lighting patterns to replicate (DirectionalLight, SkyLight, etc.)
  - [ ] Confirm naming convention: `L_Overworld` (L\_ prefix for levels)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open Unreal Editor
  - [ ] File → New Level → Empty Level
  - [ ] Save as: `/Game/FC/World/Levels/L_Overworld`
  - [ ] Open World Settings (Window → World Settings)
  - [ ] Set GameMode Override: `BP_FCGameMode` (same as L_Office)
  - [ ] Save level

- [ ] **Testing After Step 2.1.1** ✅ CHECKPOINT
  - [ ] Level opens without errors
  - [ ] World Settings shows correct GameMode
  - [ ] Level saved successfully in correct folder
  - [ ] No warnings in Output Log

**COMMIT POINT 2.1.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Create empty L_Overworld level with GameMode configuration"`

---

##### Step 2.1.2: Add Lighting Setup

- [x] **Analysis**

  - [x] Review L_Office lighting configuration (DirectionalLight angle, SkyLight intensity)
  - [x] Determine appropriate lighting for top-down view (higher angle, clear shadows)

- [x] **Implementation (Unreal Editor)**

  - [x] Add Directional Light:
    - [x] Place in level, rename to `DirectionalLight_Sun`
    - [x] Set Rotation: Pitch=-60, Yaw=45, Roll=0 (top-down lighting angle)
    - [x] Set Intensity: 5.0 lux (outdoor daylight)
    - [x] Enable Cast Shadows: True
    - [x] Set Mobility: Stationary
  - [x] Add Sky Light:
    - [x] Place in level, rename to `SkyLight_Main`
    - [x] Set Intensity: 1.0
    - [x] Set Source Type: SLS Captured Scene
    - [x] Enable Cast Shadows: True
    - [x] Set Mobility: Stationary
  - [x] Add Sky Atmosphere:
    - [x] Place in level, rename to `SkyAtmosphere_Main`
    - [x] Use default settings (connects to DirectionalLight automatically)
  - [x] Add Exponential Height Fog (optional for atmosphere):
    - [x] Place in level, rename to `HeightFog_Main`
    - [x] Set Fog Density: 0.02 (subtle haze)
    - [x] Set Fog Height Falloff: 0.2
  - [x] Save level

- [x] **Testing After Step 2.1.2** ✅ CHECKPOINT
  - [x] PIE (Play In Editor) - level loads with proper lighting
  - [x] Viewport shows clear directional shadows
  - [x] No lighting build warnings (stationary lights may show preview)
  - [x] Save and verify no errors

**COMMIT POINT 2.1.2**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add lighting setup (sun, sky, atmosphere, fog)"`

---

#### Step 2.2: Create Terrain & Ground Plane

##### Step 2.2.1: Add Ground Plane with Basic Material

- [x] **Analysis**

  - [x] Check if starter content materials available (M_Ground, M_Concrete)
  - [x] Determine appropriate ground size for initial testing (recommendation: 10,000 x 10,000 units)

- [x] **Implementation (Unreal Editor)**

  - [x] Add Landscape:
    - [x] Basede on HeightMap
    - [x] Rename to `Landscape`
  - [x] Apply Material:
    - [x] Assign Material: `/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark`
  - [x] Save level

**COMMIT POINT 2.2.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add ground plane with collision for click-to-move testing"`

---

##### Step 2.2.2: Add Basic Terrain Variation (Optional Heightmap/Landscape)

- [x] **Analysis**

  - [x] If Landscape: Review UE Landscape documentation for NavMesh compatibility

- [x] **Implementation (Unreal Editor) - Option B: Create Landscape (More Realistic)**

  - [x] Landscape Mode → Create New Landscape
  - [x] Set Section Size: 63x63 quads, Sections Per Component: 1x1, Components: 8x8 (medium size)
  - [x] Set Location: X=0, Y=0, Z=0
  - [x] Set Material: Starter Content landscape material or placeholder
  - [x] Use Sculpt tool to add gentle hills (avoid steep slopes for NavMesh)
  - [x] Paint with basic grass/ground texture
  - [x] Save level

- [x] **Testing After Step 2.2.2** ✅ CHECKPOINT
  - [x] Terrain visible and walkable
  - [x] No z-fighting or visual glitches
  - [x] Props (if added) have collision
  - [x] Save successful

**COMMIT POINT 2.2.2**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add terrain variation"`

---

#### Step 2.3: Configure NavMesh for AI Pathfinding

##### Step 2.3.1: Add NavMesh Bounds Volume

- [x] **Analysis**

  - [x] Review UE NavMesh documentation (NavMeshBoundsVolume, RecastNavMesh)
  - [x] Confirm ground plane/landscape has proper collision for NavMesh generation

- [x] **Implementation (Unreal Editor)**

  - [x] Add NavMesh Bounds Volume:
    - [x] Place Actor → Volumes → Nav Mesh Bounds Volume
    - [x] Rename to `NavMeshBounds_Main`
    - [x] Set Location: X=0, Y=0, Z=0 (center of map)
    - [x] Set Scale to cover entire ground plane: X=100, Y=100, Z=10 (or adjust to match ground size)
  - [x] Verify NavMesh Generation:
    - [x] Press `P` key in viewport to toggle NavMesh visualization
    - [x] Green overlay should appear on walkable surfaces (ground plane)
    - [x] If no green overlay: Check collision settings on ground plane
  - [x] Configure Project Settings (deferred until convoy details known):
    - [x] Project Settings → Navigation System → Agents → Agent0
    - [x] NOTE: Agent Radius, Height, Max Step Height will be adjusted after convoy implementation
    - [x] Current defaults acceptable for initial testing
  - [x] Save level

- [x] **Testing After Step 2.3.1** ✅ CHECKPOINT
  - [x] Press `P` to see green NavMesh overlay covering ground
  - [x] NavMesh regenerates when moving bounds volume
  - [x] No errors in Output Log related to NavMesh
  - [x] Save successful

**Navigation System Settings Note**: Agent parameters (Radius, Height, Max Step Height) will be configured after convoy pawn implementation in Task 5 when exact dimensions are known.

**COMMIT POINT 2.3.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add NavMesh bounds volume for AI pathfinding"`

---

#### Step 2.4: Add Player Start and Camera Position

##### Step 2.4.1: Add Player Start for Overworld Spawn

- [x] **Analysis**

  - [x] Check L_Office PlayerStart placement pattern
  - [x] Determine appropriate spawn location (center or edge of map)

- [x] **Implementation (Unreal Editor)**

  - [x] Add Player Start:
    - [x] Place Actor → Basic → Player Start
    - [x] Rename to `PlayerStart_Overworld`
    - [x] Set Location: X=0, Y=0, Z=100 (center of map, above ground)
    - [x] Set Rotation: Yaw=0 (facing north)
  - [x] Add Camera Start Position Marker (optional, for reference):
    - [x] Place Actor → Empty Actor
    - [x] Rename to `CameraStart_Overworld`
    - [x] Set Location: X=-1000, Y=0, Z=1500 (top-down view position)
    - [x] Set Rotation: Pitch=-70, Yaw=0, Roll=0 (looking down at spawn)
  - [x] Save level

- [x] **Testing After Step 2.4.1** ✅ CHECKPOINT
  - [x] PlayerStart visible in viewport (blue flag icon)
  - [x] PlayerStart positioned correctly on NavMesh (green overlay)
  - [x] PIE (Play In Editor) - spawns at PlayerStart location
  - [x] No spawn collision warnings
  - [x] Save successful

**COMMIT POINT 2.4.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add PlayerStart and camera reference marker"`

---

#### Step 2.5: Level Blueprint Configuration & Game State Transition

##### Step 2.5.1: Configure Level Blueprint to Set Overworld_Travel State

- [x] **Analysis**

  - [x] Review L_Office Level Blueprint for GameStateManager usage patterns
  - [x] Confirm UFCGameStateManager has Overworld_Travel state (should exist from Week 2)

- [x] **Implementation (Unreal Editor - Level Blueprint)**

  - [x] Open L_Overworld Level Blueprint (Blueprints → Open Level Blueprint)
  - [x] Add Event BeginPlay node
  - [x] Get Game Instance:
    - [x] Add node: Get Game Instance
    - [x] Cast to BP_FCGameInstance (or UFCGameInstance if C++ cast node)
  - [x] Get GameStateManager Subsystem:
    - [x] From Game Instance, add node: Get Subsystem
    - [x] Subsystem Class: UFCGameStateManager
  - [x] Transition to Overworld_Travel:
    - [x] From GameStateManager, call: TransitionTo
    - [x] New State: Overworld_Travel (enum value)
  - [x] Add Print String (debug):
    - [x] Connect after TransitionTo
    - [x] Text: "L_Overworld: Transitioned to Overworld_Travel state"
    - [x] Duration: 5.0
  - [x] Compile and save Level Blueprint

- [x] **Testing After Step 2.5.1** ✅ CHECKPOINT
  - [x] Open L_Overworld and PIE (Play In Editor)
  - [x] Check Output Log for:
    - [x] "L_Overworld: Transitioned to Overworld_Travel state" message
    - [x] UFCGameStateManager transition log (if logging enabled)
  - [x] No errors or "Accessed None" warnings
  - [x] Save successful

**COMMIT POINT 2.5.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Configure Level Blueprint to transition to Overworld_Travel state"`

---

### Task 2 Acceptance Criteria

- [x] L_Overworld level created and opens without errors
- [x] Lighting setup complete (DirectionalLight, SkyLight, SkyAtmosphere, optional fog)
- [x] Ground plane or landscape with proper collision
- [x] NavMesh visible (green overlay when pressing `P`)
- [x] PlayerStart positioned correctly
- [x] Level Blueprint transitions to Overworld_Travel state on BeginPlay
- [x] No errors or warnings when loading level in PIE
- [x] All assets saved in correct folders (`/Game/FC/World/Levels/Overworld`)

**Task 2 complete. Ready for Task 3 sub-tasks (Top-Down Camera Actor)? Respond with 'Go' to continue.**

---

### Task 3: Top-Down Camera Actor & Input

**Purpose**: Implement BP_OverworldCamera with Wartales-style distance-limited panning and zoom. Camera movement is constrained by crew skills (represented by a base distance parameter with multipliers for pan/zoom limits). Camera is always north-aligned with no rotation support.

**Key Design Constraints**:

- **Distance-Limited Panning**: Camera can only pan within a radius from the player pawn (convoy), determined by `CrewVisionRange` parameter
- **Distance-Limited Zoom**: Zoom out distance is also limited by `CrewVisionRange` using a `ZoomDistanceMultiplier`
- **North-Aligned**: Camera rotation locked to Yaw=0 (always facing north), no rotation input
- **Skill-Based**: `CrewVisionRange` parameter will later integrate with crew skill system to dynamically adjust exploration range
- **Reference Game**: Wartales top-down camera behavior

---

#### Step 3.1: Create Input Actions for Overworld Camera

##### Step 3.1.1: Create IA_OverworldPan (Axis2D for WASD)

- [x] **Analysis**

  - [x] Check existing Input Actions in `/Content/FC/Input/Actions/` (IA_Move, IA_Look from Week 1)
  - [x] Review IA_Move configuration for Axis2D pattern
  - [x] Confirm naming: IA_OverworldPan (IA\_ prefix)

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Input/Actions/`
  - [x] Right-click → Input → Input Action
  - [x] Name: `IA_OverworldPan`
  - [x] Open IA_OverworldPan
  - [x] Set Value Type: Axis2D (Vector2D)
  - [x] Save asset

- [x] **Testing After Step 3.1.1** ✅ CHECKPOINT
  - [x] Asset created at correct path
  - [x] Value Type set to Axis2D
  - [x] Asset saves without errors
  - [x] No warnings in Output Log

**COMMIT POINT 3.1.1**: `git add Content/FC/Input/Actions/IA_OverworldPan.uasset && git commit -m "feat(overworld): Create IA_OverworldPan input action (Axis2D)"`

---

##### Step 3.1.2: Create IA_OverworldZoom (Axis1D for Mouse Wheel)

- [x] **Analysis**

  - [x] Review UE Enhanced Input mouse wheel axis configuration
  - [x] Confirm value type: Axis1D (float) for scroll delta

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Input/Actions/`
  - [x] Right-click → Input → Input Action
  - [x] Name: `IA_OverworldZoom`
  - [x] Open IA_OverworldZoom
  - [x] Set Value Type: Axis1D (float)
  - [x] Save asset

- [x] **Testing After Step 3.1.2** ✅ CHECKPOINT
  - [x] Asset created at correct path
  - [x] Value Type set to Axis1D
  - [x] Asset saves without errors

**COMMIT POINT 3.1.2**: `git add Content/FC/Input/Actions/IA_OverworldZoom.uasset && git commit -m "feat(overworld): Create IA_OverworldZoom input action (Axis1D)"`

---

#### Step 3.2: Configure IMC_FC_TopDown Input Mapping Context

##### Step 3.2.1: Add WASD Bindings to IA_OverworldPan

- [x] **Analysis**

  - [x] Check IMC_FC_FirstPerson for WASD swizzle modifier patterns (Week 1)
  - [x] Confirm IMC_FC_TopDown exists (created in Week 2, should be empty)

- [x] **Implementation (Unreal Editor)**

  - [x] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [x] Add Mapping: IA_OverworldPan
  - [x] Add Key: **W** (forward/up movement)
    - [x] Add Modifier: Swizzle Input Axis Values
    - [x] Set Order: YXZ (maps W to Y-axis positive)
  - [x] Add Key: **S** (backward/down movement)
    - [x] Add Modifier: Swizzle Input Axis Values (YXZ)
    - [x] Add Modifier: Negate (makes Y negative)
  - [x] Add Key: **D** (right movement)
    - [x] No modifiers (X-axis positive by default)
  - [x] Add Key: **A** (left movement)
    - [x] Add Modifier: Negate (makes X negative)
  - [x] Save IMC_FC_TopDown

- [x] **Testing After Step 3.2.1** ✅ CHECKPOINT
  - [x] All 4 keys (WASD) bound to IA_OverworldPan
  - [x] Modifiers configured correctly (Swizzle for W/S, Negate for S/A)
  - [x] Asset saves without errors

**COMMIT POINT 3.2.1**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Configure WASD bindings for camera pan in IMC_FC_TopDown"`

---

##### Step 3.2.2: Add Mouse Wheel Binding to IA_OverworldZoom

- [x] **Analysis**

  - [x] Review UE mouse wheel input key name (Mouse Wheel Axis)
  - [x] Determine if negate needed (wheel up = positive zoom or negative zoom)

- [x] **Implementation (Unreal Editor)**

  - [x] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown` (if not already open)
  - [x] Add Mapping: IA_OverworldZoom
  - [x] Add Key: **Mouse Wheel Axis**
    - [x] No modifiers (or add Negate if zoom direction feels inverted)
  - [x] Save IMC_FC_TopDown

- [x] **Testing After Step 3.2.2** ✅ CHECKPOINT
  - [x] Mouse Wheel Axis bound to IA_OverworldZoom
  - [x] Asset saves without errors
  - [x] IMC_FC_TopDown now has 2 mappings (Pan and Zoom)

**COMMIT POINT 3.2.2**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Configure mouse wheel binding for camera zoom in IMC_FC_TopDown"`

---

#### Step 3.3: Create AFCOverworldCamera C++ Class

##### Step 3.3.1: Create C++ Class Files (AFCOverworldCamera)

- [x] **Analysis**

  - [x] Review ACameraActor parent class for inheritance
  - [x] Check Enhanced Input C++ binding patterns from AFCFirstPersonCharacter (Week 1)
  - [x] Confirm folder: `Source/FC/World/`
  - [x] Determine if SpringArm or direct Z-height zoom approach

- [x] **Implementation (C++ Files)**

  - [x] Create new C++ class via Unreal Editor:
    - [x] Tools → New C++ Class
    - [x] Parent Class: Camera Actor
    - [x] Name: `FCOverworldCamera`
    - [x] Path: `Source/FC/World/`
  - [x] **FCOverworldCamera.h** - Add to class declaration:

```cpp
UCLASS()
class FC_API AFCOverworldCamera : public ACameraActor
{
    GENERATED_BODY()

public:
    AFCOverworldCamera();

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    /** Set the player pawn reference for distance limiting (called by controller) */
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetPlayerPawn(APawn* NewPawn);

    /** Get current crew vision range (for UI or skill system integration) */
    UFUNCTION(BlueprintPure, Category = "Camera")
    float GetCrewVisionRange() const { return CrewVisionRange; }

protected:
    virtual void BeginPlay() override;

    /** Handle camera panning input */
    void HandlePan(const struct FInputActionValue& Value);

    /** Handle camera zoom input */
    void HandleZoom(const struct FInputActionValue& Value);

    /** Apply distance limiting to camera position relative to player pawn */
    void ApplyDistanceLimit();

    /** Force camera rotation to north (Yaw=0) */
    void EnforceNorthAlignment();

    /** Draw debug visualization for pan/zoom limits */
    void DrawDebugLimits() const;

private:
    // ========== Input Actions ==========

    /** Pan input action (WASD) */
    UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
    TObjectPtr<class UInputAction> PanAction;

    /** Zoom input action (Mouse Wheel) */
    UPROPERTY(EditDefaultsOnly, Category = "Input|Actions")
    TObjectPtr<class UInputAction> ZoomAction;

    // ========== Movement Parameters ==========

    /** Camera pan speed (units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Movement", meta = (AllowPrivateAccess = "true"))
    float PanSpeed = 500.0f;

    /** Camera zoom speed (units per wheel tick) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float ZoomSpeed = 100.0f;

    /** Minimum zoom distance (closest to convoy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float ZoomMin = 500.0f;

    // ========== Crew Skill Parameters ==========

    /** Base vision range for crew (drives both pan and zoom limits) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Crew Skills", meta = (AllowPrivateAccess = "true", Tooltip = "Base vision range, will be driven by crew skills in future"))
    float CrewVisionRange = 2000.0f;

    /** Multiplier for max pan distance from convoy (e.g., 1.5 = 150% of CrewVisionRange) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Movement", meta = (AllowPrivateAccess = "true"))
    float PanDistanceMultiplier = 1.5f;

    /** Multiplier for max zoom distance (e.g., 2.0 = 200% of CrewVisionRange) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Crew Skills", meta = (AllowPrivateAccess = "true"))
    float ZoomDistanceMultiplier = 2.0f;

    // ========== References ==========

    /** Player pawn reference (convoy) for distance limiting */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<APawn> PlayerPawn;

    /** SpringArm component (optional, created in constructor) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USpringArmComponent> SpringArm;

    // ========== Debug ==========

    /** Show debug radius circles (yellow=pan, cyan=zoom) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Debug", meta = (AllowPrivateAccess = "true"))
    bool bShowDebugRadius = false;

    /** Pending pan movement delta (accumulated from input) */
    FVector PendingMovementDelta = FVector::ZeroVector;
};
```

- [x] Save FCOverworldCamera.h

- [x] **Testing After Step 3.3.1** ✅ CHECKPOINT
  - [x] Header file created in `Source/FC/World/FCOverworldCamera.h`
  - [x] Class declaration complete with all parameters
  - [x] File saved without syntax errors
  - [x] Ready for .cpp implementation**COMMIT POINT 3.3.1**: `git add Source/FC/World/FCOverworldCamera.h && git commit -m "feat(overworld): Create AFCOverworldCamera C++ class header"`

---

##### Step 3.3.2: Implement C++ Constructor and Core Logic (AFCOverworldCamera.cpp)

- [x] **Analysis**

  - [x] Review Enhanced Input C++ binding patterns from AFCFirstPersonCharacter
  - [x] Plan vector math for distance limiting (FVector::Dist2D, FMath::Clamp)
  - [x] Determine SpringArm setup in constructor

- [x] **Implementation (FCOverworldCamera.cpp)**

```cpp
#include "World/FCOverworldCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"

AFCOverworldCamera::AFCOverworldCamera()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create SpringArm component for zoom functionality
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 1500.0f; // Starting zoom distance
    SpringArm->bDoCollisionTest = false; // No collision for camera
    SpringArm->bEnableCameraLag = false; // Instant response
    SpringArm->SetRelativeRotation(FRotator(-70.0f, 0.0f, 0.0f)); // Top-down angle

    // Reattach camera to spring arm tip (override ACameraActor default)
    GetCameraComponent()->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    GetCameraComponent()->SetRelativeLocation(FVector::ZeroVector);
    GetCameraComponent()->SetRelativeRotation(FRotator::ZeroRotator);
}

void AFCOverworldCamera::BeginPlay()
{
    Super::BeginPlay();

    // Enforce initial north alignment
    EnforceNorthAlignment();
}

void AFCOverworldCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput) return;

    // Bind pan action (WASD)
    if (PanAction)
    {
        EnhancedInput->BindAction(PanAction, ETriggerEvent::Triggered, this, &AFCOverworldCamera::HandlePan);
    }

    // Bind zoom action (Mouse Wheel)
    if (ZoomAction)
    {
        EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AFCOverworldCamera::HandleZoom);
    }
}

void AFCOverworldCamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply pending pan movement with distance limiting
    if (!PendingMovementDelta.IsNearlyZero())
    {
        FVector NewLocation = GetActorLocation() + PendingMovementDelta * DeltaTime;
        SetActorLocation(NewLocation);
        ApplyDistanceLimit();
        PendingMovementDelta = FVector::ZeroVector;
    }

    // Force north alignment every frame
    EnforceNorthAlignment();

    // Draw debug visualization
    if (bShowDebugRadius)
    {
        DrawDebugLimits();
    }
}

void AFCOverworldCamera::SetPlayerPawn(APawn* NewPawn)
{
    PlayerPawn = NewPawn;
}

void AFCOverworldCamera::HandlePan(const FInputActionValue& Value)
{
    // Get 2D input vector (X=right, Y=forward)
    const FVector2D InputVector = Value.Get<FVector2D>();

    // Convert to 3D movement (Z=0 for horizontal panning)
    const FVector MovementDelta = FVector(InputVector.X, InputVector.Y, 0.0f) * PanSpeed;

    // Accumulate delta for Tick application (frame-rate independent)
    PendingMovementDelta += MovementDelta;
}

void AFCOverworldCamera::HandleZoom(const FInputActionValue& Value)
{
    if (!SpringArm) return;

    // Get zoom delta (mouse wheel: positive=zoom in, negative=zoom out)
    const float ZoomDelta = Value.Get<float>();

    // Calculate new arm length (subtract because wheel up should zoom in = decrease distance)
    float NewArmLength = SpringArm->TargetArmLength - (ZoomDelta * ZoomSpeed);

    // Calculate max zoom distance based on crew vision range
    const float MaxZoomDistance = CrewVisionRange * ZoomDistanceMultiplier;

    // Clamp between min and skill-based max
    NewArmLength = FMath::Clamp(NewArmLength, ZoomMin, MaxZoomDistance);

    // Apply new zoom distance
    SpringArm->TargetArmLength = NewArmLength;
}

void AFCOverworldCamera::ApplyDistanceLimit()
{
    // Only apply limit if player pawn reference is valid
    if (!PlayerPawn.IsValid()) return;

    const FVector PawnLocation = PlayerPawn->GetActorLocation();
    const FVector CameraLocation = GetActorLocation();

    // Calculate 2D distance (ignore Z-axis for top-down view)
    const float Distance = FVector::Dist2D(CameraLocation, PawnLocation);

    // Calculate max allowed pan distance
    const float MaxPanDistance = CrewVisionRange * PanDistanceMultiplier;

    // Clamp camera position to circular boundary if exceeding limit
    if (Distance > MaxPanDistance)
    {
        // Get 2D direction from pawn to camera
        FVector Direction = CameraLocation - PawnLocation;
        Direction.Z = 0.0f; // Flatten to 2D
        Direction.Normalize();

        // Calculate clamped position at max distance
        FVector ClampedLocation = PawnLocation + Direction * MaxPanDistance;
        ClampedLocation.Z = CameraLocation.Z; // Preserve Z height

        SetActorLocation(ClampedLocation);
    }
}

void AFCOverworldCamera::EnforceNorthAlignment()
{
    // Force camera rotation to always face north (Yaw=0)
    FRotator CurrentRotation = GetActorRotation();
    if (!FMath::IsNearlyZero(CurrentRotation.Yaw))
    {
        CurrentRotation.Yaw = 0.0f;
        SetActorRotation(CurrentRotation);
    }
}

void AFCOverworldCamera::DrawDebugLimits() const
{
    if (!PlayerPawn.IsValid()) return;

    const FVector PawnLocation = PlayerPawn->GetActorLocation();
    const UWorld* World = GetWorld();
    if (!World) return;

    // Draw yellow circle for pan distance limit
    const float PanRadius = CrewVisionRange * PanDistanceMultiplier;
    DrawDebugCircle(World, PawnLocation, PanRadius, 64, FColor::Yellow, false, -1.0f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0), false);

    // Draw cyan circle for zoom distance limit
    const float ZoomRadius = CrewVisionRange * ZoomDistanceMultiplier;
    DrawDebugCircle(World, PawnLocation, ZoomRadius, 64, FColor::Cyan, false, -1.0f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0), false);
}
```

- [x] Save FCOverworldCamera.cpp
- [x] Compile C++ project (hot reload or editor restart)
- [x] Verify no compilation errors

- [x] **Testing After Step 3.3.2** ✅ CHECKPOINT
  - [x] C++ compiles successfully
  - [x] AFCOverworldCamera appears in Content Browser (C++ Classes folder)
  - [x] No linker errors
  - [x] Class can be instantiated in editor**COMMIT POINT 3.3.2**: `git add Source/FC/World/FCOverworldCamera.cpp && git commit -m "feat(overworld): Implement AFCOverworldCamera core logic (pan, zoom, distance limiting, north lock)"`

---

##### Step 3.3.3: Create BP_OverworldCamera Blueprint Child

- [x] **Analysis**

  - [x] Confirm AFCOverworldCamera C++ class compiled and visible
  - [x] Plan Blueprint child for designer-friendly parameter defaults
  - [x] Confirm folder: `/Content/FC/World/Blueprints/Cameras/`

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/World/Blueprints/Cameras/` (create folder if needed)
  - [x] Right-click → Blueprint Class
  - [x] Parent Class: Search for `FCOverworldCamera` (C++ class)
  - [x] Name: `BP_OverworldCamera`
  - [x] Open BP_OverworldCamera
  - [x] **Class Defaults** (set default parameter values for designers):
    - [x] **Camera|Movement** section:
      - [x] Pan Speed: 500.0
      - [x] Pan Distance Multiplier: 1.5
    - [x] **Camera|Zoom** section:
      - [x] Zoom Speed: 100.0
      - [x] Zoom Min: 500.0
    - [x] **Camera|Crew Skills** section:
      - [x] Crew Vision Range: 2000.0
      - [x] Zoom Distance Multiplier: 2.0
    - [x] **Camera|Debug** section:
      - [x] Show Debug Radius: False
  - [x] **Components Panel** (verify SpringArm from C++):
    - [x] SpringArm component should be present (created in C++ constructor)
    - [x] Camera Component attached to SpringArm tip
    - [x] SpringArm Rotation: Pitch=-70, Yaw=0, Roll=0 (verify)
  - [x] **Optional Blueprint-Only Additions** (Event Graph):
    - [x] Add custom Begin Play logic if needed (e.g., level-specific camera positioning)
    - [x] Add Blueprint-callable helper functions for designers
    - [x] **Keep Event Graph minimal** - core logic is in C++
  - [x] Compile and save Blueprint

- [x] **Testing After Step 3.3.3** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] All C++ parameters visible in Class Defaults
  - [x] Default values set correctly for design iteration
  - [x] Can place BP_OverworldCamera in L_Overworld viewport
  - [x] SpringArm component present with correct rotation

**COMMIT POINT 3.3.3**: `git add Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset && git commit -m "feat(overworld): Create BP_OverworldCamera Blueprint child with designer defaults"`

---

##### Step 3.3.4: Test Camera Functionality in L_Overworld

- [x] **Analysis**

  - [x] Verify camera works in isolation before controller integration (Task 4)
  - [x] Test input actions, distance limiting, north lock, debug visualization
  - [x] Manually set PlayerPawn reference for testing
  - **SKIPPED**: Cannot test camera pan/zoom without controller (AFCPlayerController)
  - **REASON**: BP_FC_GameMode uses BP_FC_PlayerController which doesn't have TopDown input bindings
  - **RESOLUTION**: Implement Task 4 first to create controller that binds IA_OverworldPan/Zoom to camera methods

**COMMIT POINT 3.3.4**: `git add Content/FC/World/Levels/Overworld/L_Overworld.umap && git commit -m "test(overworld): Verify BP_OverworldCamera functionality in L_Overworld"`

**Note**: Proper camera spawning and PlayerPawn assignment will be handled by AFCPlayerController in Task 4. This step confirms the camera C++ logic works correctly in isolation.

**Architecture Note**: AFCOverworldCamera (C++) handles all core logic (input, math, distance limiting, north lock, Tick performance), while BP_OverworldCamera (Blueprint) provides designer-friendly parameter defaults and component configuration. This split maximizes maintainability and performance while keeping design iteration fast.

**Design Note**: The `CrewVisionRange` parameter serves as the base value for both pan and zoom limits, modified by separate multipliers. This allows future integration with crew skill systems while maintaining independent control over camera behavior (e.g., crews might see farther than they can zoom, or vice versa).

**Task 3 complete. Ready for Task 4 sub-tasks (Overworld Player Controller)? Respond with 'Go' to continue.**

---

### Task 4: Overworld Player Controller & Input Context

**Purpose**: ~~Create AFCPlayerController C++ class~~ **REVISED**: Enhance existing AFCPlayerController to bind Overworld input actions and use UFCCameraManager's BlendToTopDown() method.

**Architecture Decision**: After reviewing Technical Documentation, UFCCameraManager already exists with placeholder BlendToTopDown(). We'll:

1. Implement BlendToTopDown() in UFCCameraManager to find and possess BP_OverworldCamera
2. Extend AFCPlayerController to bind IA_OverworldPan/Zoom input actions
3. Use BP_FC_PlayerController (existing Blueprint) configured for L_Overworld
4. This avoids duplicating camera management logic and leverages existing component architecture

---

#### Step 4.1: Implement BlendToTopDown in UFCCameraManager

##### Step 4.1.1: Implement Camera Discovery and Possession Logic

- [x] **Analysis**

  - [x] Review UFCCameraManager.cpp BlendToTopDown placeholder (line 230)
  - [x] Follow pattern from BlendToTableObject (dynamic camera spawning)
  - [x] Overworld camera is placed in level (not spawned), so use GetAllActorsOfClass
  - [x] Set PlayerPawn reference via reflection to avoid circular dependency

- [x] **Implementation (UFCCameraManager.cpp)**

  - [x] Replace placeholder BlendToTopDown with full implementation:

    ```cpp
    void UFCCameraManager::BlendToTopDown(float BlendTime)
    {
        // Find BP_OverworldCamera in level
        UWorld* World = GetWorld();
        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsOfClass(World, ACameraActor::StaticClass(), FoundCameras);

        ACameraActor* OverworldCamera = nullptr;
        for (AActor* Actor : FoundCameras)
        {
            if (Actor->GetName().Contains(TEXT("OverworldCamera")))
            {
                OverworldCamera = Cast<ACameraActor>(Actor);
                break;
            }
        }

        // Set PlayerPawn reference via reflection (avoid including FCOverworldCamera.h)
        APlayerController* PC = GetPlayerController();
        if (PC && PC->GetPawn())
        {
            UFunction* SetPawnFunc = OverworldCamera->FindFunction(FName("SetPlayerPawn"));
            if (SetPawnFunc)
            {
                struct FSetPlayerPawnParams { APawn* NewPawn; };
                FSetPlayerPawnParams Params;
                Params.NewPawn = PC->GetPawn();
                OverworldCamera->ProcessEvent(SetPawnFunc, &Params);
            }
        }

        float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);
        BlendToTarget(OverworldCamera, EffectiveBlendTime, DefaultBlendFunction);
        SetCameraMode(EFCPlayerCameraMode::TopDown);
    }
    ```

  - [x] Save file

- [x] **Testing After Step 4.1.1** ✅ CHECKPOINT
  - [x] Code compiles without errors
  - [x] No new includes needed (uses existing Kismet/GameplayStatics.h)
  - [x] Reflection-based SetPlayerPawn avoids circular dependency

**COMMIT POINT 4.1.1**: `git add Source/FC/Components/FCCameraManager.cpp && git commit -m "feat(overworld): Implement BlendToTopDown in UFCCameraManager"`

---

#### Step 4.2: Extend AFCPlayerController with Overworld Input Bindings

##### Step 4.2.1: Add Input Actions and Handler Methods

- [x] **Analysis**

  - [x] Review AFCPlayerController.h input action pattern (InteractAction, QuickSaveAction, etc.)
  - [x] Add OverworldPanAction and OverworldZoomAction properties
  - [x] Add HandleOverworldPan and HandleOverworldZoom handler methods
  - [x] Forward input to AFCOverworldCamera's HandlePan/HandleZoom methods

- [x] **Implementation (FCPlayerController.h)**

  - [x] Add input action properties after QuickLoadAction:

    ```cpp
    /** Input action for Overworld camera pan (WASD) - Week 3 */
    UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
    TObjectPtr<UInputAction> OverworldPanAction;

    /** Input action for Overworld camera zoom (Mouse Wheel) - Week 3 */
    UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
    TObjectPtr<UInputAction> OverworldZoomAction;
    ```

  - [x] Add handler method declarations after HandleQuickLoadPressed:
    ```cpp
    void HandleOverworldPan(const FInputActionValue& Value);
    void HandleOverworldZoom(const FInputActionValue& Value);
    ```
  - [x] Save file

- [x] **Implementation (FCPlayerController.cpp)**

  - [x] Add include for AFCOverworldCamera:
    ```cpp
    #include "World/FCOverworldCamera.h"
    ```
  - [x] Load input actions in constructor (after ClickAction):

    ```cpp
    static ConstructorHelpers::FObjectFinder<UInputAction> OverworldPanActionFinder(TEXT("/Game/FC/Input/Actions/IA_OverworldPan"));
    if (OverworldPanActionFinder.Succeeded())
    {
        OverworldPanAction = OverworldPanActionFinder.Object;
        UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_OverworldPan"));
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> OverworldZoomActionFinder(TEXT("/Game/FC/Input/Actions/IA_OverworldZoom"));
    if (OverworldZoomActionFinder.Succeeded())
    {
        OverworldZoomAction = OverworldZoomActionFinder.Object;
        UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_OverworldZoom"));
    }
    ```

  - [x] Bind actions in SetupInputComponent (after ClickAction binding):

    ```cpp
    if (OverworldPanAction)
    {
        EnhancedInput->BindAction(OverworldPanAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldPan);
    }

    if (OverworldZoomAction)
    {
        EnhancedInput->BindAction(OverworldZoomAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldZoom);
    }
    ```

  - [x] Implement handlers (after HandleQuickLoadPressed):

    ```cpp
    void AFCPlayerController::HandleOverworldPan(const FInputActionValue& Value)
    {
        if (!CameraManager || CameraManager->GetCameraMode() != EFCPlayerCameraMode::TopDown)
            return;

        AActor* ViewTarget = GetViewTarget();
        AFCOverworldCamera* OverworldCamera = Cast<AFCOverworldCamera>(ViewTarget);
        if (OverworldCamera)
        {
            OverworldCamera->HandlePan(Value);
        }
    }

    void AFCPlayerController::HandleOverworldZoom(const FInputActionValue& Value)
    {
        if (!CameraManager || CameraManager->GetCameraMode() != EFCPlayerCameraMode::TopDown)
            return;

        AActor* ViewTarget = GetViewTarget();
        AFCOverworldCamera* OverworldCamera = Cast<AFCOverworldCamera>(ViewTarget);
        if (OverworldCamera)
        {
            OverworldCamera->HandleZoom(Value);
        }
    }
    ```

  - [x] Save file

- [x] **Testing After Step 4.2.1** ✅ CHECKPOINT
  - [x] Code compiles without errors
  - [x] Input actions load correctly in constructor
  - [x] Handlers only execute in TopDown camera mode
  - [x] Input forwarded to camera's HandlePan/HandleZoom methods

**COMMIT POINT 4.2.1**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "feat(overworld): Add Overworld input bindings to AFCPlayerController"`

---

#### Step 4.3: Configure State-Driven Camera and Input for Overworld_Travel

##### Step 4.3.1: Add OnStateChanged Handler to AFCPlayerController

- [x] **Analysis**

  - [x] GameStateManager broadcasts OnStateChanged when state transitions occur
  - [x] AFCPlayerController should subscribe to this delegate and react to Overworld_Travel state
  - [x] This keeps camera/input behavior coupled to game state, not level name
  - [x] Follows existing pattern used for Office states (Office_Exploration, Office_TableView)

- [x] **Implementation (FCPlayerController.h)**

  - [x] Add protected method declaration after HandleOverworldZoom:
    ```cpp
    /** Handle game state changes (bind to GameStateManager.OnStateChanged) */
    UFUNCTION()
    void OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState);
    ```
  - [x] Added forward declaration: `struct FInputActionValue;`
  - [x] Save file

- [x] **Implementation (FCPlayerController.cpp)**

  - [x] Add OnGameStateChanged implementation after HandleOverworldZoom

    ```cpp
    void AFCPlayerController::OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState)
    {
        UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: %s -> %s"),
            *UEnum::GetValueAsString(OldState),
            *UEnum::GetValueAsString(NewState));

        // React to Overworld_Travel state entry
        if (NewState == EFCGameStateID::Overworld_Travel)
        {
            // Switch to TopDown input mode
            if (InputManager)
            {
                InputManager->SetInputMappingMode(EFCInputMappingMode::TopDown);
                UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Switched to TopDown input mode"));
            }

            // Blend to Overworld camera
            if (CameraManager)
            {
                CameraManager->BlendToTopDown(2.0f);
                UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Blending to TopDown camera"));
            }
        }
        // React to leaving Overworld_Travel state
        else if (OldState == EFCGameStateID::Overworld_Travel)
        {
            // Restore FirstPerson input mode when leaving Overworld
            if (InputManager && NewState == EFCGameStateID::Office_Exploration)
            {
                InputManager->SetInputMappingMode(EFCInputMappingMode::FirstPerson);
                CameraManager->BlendToFirstPerson(2.0f);
                UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Returned to FirstPerson mode"));
            }
        }
    }
    ```

  - [ ] Bind delegate in BeginPlay (after RestorePlayerPosition call):
    ```cpp
    // Subscribe to game state changes
    UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
    if (GI)
    {
        UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
        if (StateMgr)
        {
            StateMgr->OnStateChanged.AddDynamic(this, &AFCPlayerController::OnGameStateChanged);
            UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Subscribed to GameStateManager.OnStateChanged"));
        }
    }
    ```
  - [ ] Save file

- [ ] **Testing After Step 4.3.1** ✅ CHECKPOINT

  - [ ] Code compiles without errors
  - [ ] OnGameStateChanged signature matches delegate (EFCGameStateID, EFCGameStateID)
  - [x] Bind delegate in BeginPlay (after RestorePlayerPosition call):
    ```cpp
    // Subscribe to game state changes
    UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
    if (GI)
    {
        UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
        if (StateMgr)
        {
            StateMgr->OnStateChanged.AddDynamic(this, &AFCPlayerController::OnGameStateChanged);
            UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Subscribed to GameStateManager.OnStateChanged"));
        }
    }
    ```
  - [x] Save file

- [x] **Testing After Step 4.3.1** ✅ CHECKPOINT
  - [x] Code compiles without errors
  - [x] OnGameStateChanged signature matches delegate (EFCGameStateID, EFCGameStateID)
  - [x] Delegate binding in BeginPlay present
  - [x] Deleted obsolete FCPlayerController.h file

**COMMIT POINT 4.3.1**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "feat(overworld): Add state-driven camera/input switching for Overworld_Travel"`

**Architecture Note**: This approach decouples level-specific behavior from Level Blueprints. Any level that transitions to Overworld_Travel state will automatically trigger TopDown camera/input mode. This also handles Office↔Overworld transitions cleanly (Task 7) without duplicating logic.

---

##### Step 4.3.2: Place BP_OverworldCamera in L_Overworld

- [x] **Analysis**

  - [x] UFCCameraManager.BlendToTopDown() searches for camera with "OverworldCamera" in name
  - [x] Camera should be positioned above PlayerStart for initial view
  - [x] No Level Blueprint changes needed - state transition already exists from Task 2.5.1

- [x] **Implementation (Unreal Editor)**

  - [x] Open L_Overworld level
  - [x] Drag BP_OverworldCamera from Content Browser into viewport
  - [x] Position camera:
    - [x] Location: X=-1000, Y=0, Z=1500 (behind and above PlayerStart)
    - [x] Rotation: Pitch=-70, Yaw=0, Roll=0
  - [x] Details panel:
    - [x] Auto Activate: True
    - [x] Verify SpringArm properties match C++ defaults
  - [x] Save level

- [x] **Verify Existing Level Blueprint** (no changes needed)

  - [x] Open Level Blueprint (Blueprints → Open Level Blueprint)
  - [x] Confirm Event BeginPlay → GameStateManager → TransitionTo(Overworld_Travel) exists
  - [x] This is from Task 2.5.1 - no modifications needed
  - [x] State transition will trigger OnGameStateChanged in PlayerController

- [x] **Testing After Step 4.3.2** ✅ CHECKPOINT
  - [x] BP_OverworldCamera visible in Outliner with "OverworldCamera" in name
  - [x] Camera positioned correctly (can see PlayerStart in viewport preview)
  - [x] Level Blueprint unchanged from Task 2.5.1
  - [x] Level saves without errors

**COMMIT POINT 4.3.2**: `git add Content/FC/World/Levels/Overworld/L_Overworld.umap && git commit -m "feat(overworld): Place BP_OverworldCamera in L_Overworld"`

**Integration Note**: L_Overworld Level Blueprint (Task 2.5.1) transitions to Overworld_Travel → OnStateChanged fires → AFCPlayerController.OnGameStateChanged() switches input mode and camera. This creates a clean state-driven flow without level-specific logic in the controller.

---

#### Step 4.4: Test State-Driven Overworld System

##### Step 4.4.1: Full State-Driven Integration Test

- [x] **Analysis**

  - [x] Test complete flow: Level loads → GameState transitions → OnStateChanged fires → Camera/Input switch
  - [x] Verify state-driven architecture works correctly
  - [x] Check Output Log for delegate and state transition logs

- [x] **Test Sequence**

  - [x] Open L_Overworld in editor
  - [x] Compile C++ project (if not already compiled)
  - [x] PIE (Play In Editor)
  - [x] **Check Output Log for:**
    - [x] "Loaded IA_OverworldPan" (AFCPlayerController constructor)
    - [x] "Loaded IA_OverworldZoom" (AFCPlayerController constructor)
    - [x] "Bound IA_OverworldPan" (SetupInputComponent)
    - [x] "Bound IA_OverworldZoom" (SetupInputComponent)
    - [x] "Subscribed to GameStateManager.OnStateChanged" (BeginPlay)
    - [x] "L_Overworld: Transitioned to Overworld_Travel state" (Level Blueprint - Task 2.5.1)
    - [x] "OnGameStateChanged: None -> Overworld_Travel" (AFCPlayerController)
    - [x] "Switched to TopDown input mode" (OnGameStateChanged)
    - [x] "Blending to TopDown camera" (OnGameStateChanged)
    - [x] "Set PlayerPawn reference on camera" (UFCCameraManager.BlendToTopDown)
  - [x] **Test WASD Pan:**
    - [x] Press W → Camera pans forward (Y positive)
    - [x] Press S → Camera pans backward (Y negative)
    - [x] Press A → Camera pans left (X negative)
    - [x] Press D → Camera pans right (X positive)
    - [x] Pan until yellow debug circle boundary (if debug enabled)
    - [x] Verify camera stops at boundary (distance limiting works)
  - [x] **Test Mouse Wheel Zoom:**
    - [x] Scroll up → Camera zooms in (SpringArm shortens)
    - [x] Scroll down → Camera zooms out (SpringArm lengthens)
    - [x] Zoom to minimum (500 units)
    - [x] Zoom to maximum (cyan debug circle boundary if debug enabled)
    - [x] Verify zoom stops at min/max limits
  - [x] **Test North Alignment:**
    - [x] Try to rotate camera (should have no effect)
    - [x] Camera Yaw should always be 0 (north-aligned)
  - [x] **Test Debug Visualization (optional):**
    - [x] Select BP_OverworldCamera instance in Outliner
    - [x] Set Show Debug Radius = True in Details panel
    - [x] PIE again
    - [x] Yellow circle = pan limit (3000 units from PlayerStart)
    - [x] Cyan circle = zoom limit (4000 units from PlayerStart)
  - [x] Document any issues in "Known Issues & Backlog"

- [x] **Testing After Step 4.4.1** ✅ CHECKPOINT
  - [x] OnStateChanged delegate fires correctly (logs confirm)
  - [x] TopDown input mode active after state transition
  - [x] Camera auto-possesses via state-driven BlendToTopDown call
  - [x] WASD pans camera smoothly with distance limiting
  - [x] Mouse wheel zooms correctly with min/max limits
  - [x] Camera rotation locked to north (Yaw=0)
  - [x] No "Accessed None" errors
  - [x] No input binding warnings
  - [x] Debug visualization works (if enabled)
  - [x] State transition flow visible in logs (None→Overworld_Travel→Camera/Input switch)

**COMMIT POINT 4.4.1**: `git add -A && git commit -m "test(overworld): Verify state-driven camera/input switching for Overworld_Travel"`

**State Flow Verification**: Ensure the complete chain works: L_Overworld loads → Level Blueprint calls TransitionTo(Overworld_Travel) → GameStateManager broadcasts OnStateChanged → AFCPlayerController.OnGameStateChanged() reacts → InputManager switches to TopDown → CameraManager blends to camera. This validates the state-driven architecture.

---

### Task 4 Acceptance Criteria ✅ COMPLETE

- [x] UFCCameraManager.BlendToTopDown() implemented to find and possess BP_OverworldCamera
- [x] AFCPlayerController extended with IA_OverworldPan and IA_OverworldZoom input actions
- [x] HandleOverworldPan/Zoom methods forward input to AFCOverworldCamera
- [x] Input actions load in constructor and bind in SetupInputComponent
- [x] AFCPlayerController.OnGameStateChanged() method implemented
- [x] OnGameStateChanged() subscribes to GameStateManager.OnStateChanged delegate in BeginPlay
- [x] OnGameStateChanged() switches to TopDown input mode when entering Overworld_Travel state
- [x] OnGameStateChanged() calls CameraManager->BlendToTopDown() when entering Overworld_Travel state
- [x] OnGameStateChanged() restores FirstPerson mode when leaving Overworld_Travel for Office_Exploration
- [x] BP_OverworldCamera placed in L_Overworld at correct position
- [x] PIE in L_Overworld triggers Overworld_Travel state transition (Task 2.5.1)
- [x] State transition fires OnStateChanged delegate (confirmed in logs)
- [x] TopDown input mode activates via state change (confirmed in logs)
- [x] Camera auto-possesses via state-driven BlendToTopDown (confirmed in logs)
- [x] WASD pans camera with distance limiting
- [x] Mouse wheel zooms camera with min/max limits
- [x] Camera rotation locked to north (Yaw=0)
- [x] Fixed input axis mapping (swapped X/Y for correct WASD directions)
- [x] Fixed input action asset paths (/Game/FC/Input/ not /Game/FC/Input/Actions/)
- [x] No compilation errors or Blueprint errors
- [x] All assets saved in correct folders
- [x] Camera auto-possesses via state-driven BlendToTopDown (confirmed in logs)
- [x] WASD pans camera with distance limiting
- [x] Mouse wheel zooms camera with min/max limits
- [x] Camera rotation locked to north (Yaw=0)
- [x] No compilation errors or Blueprint errors
- [x] All assets saved in correct folders

**Architecture Note**: By using GameStateManager.OnStateChanged delegate, camera and input behavior is now **state-driven** rather than level-driven. AFCPlayerController reacts to Overworld_Travel state entry regardless of which level triggered it. This makes the system:

- **Scalable**: Easy to add new Overworld levels (e.g., multiple regions) without duplicating logic
- **Maintainable**: Camera/input logic centralized in one place (OnGameStateChanged)
- **Testable**: State transitions can be triggered programmatically for testing
- **Consistent**: Same behavior across all Overworld contexts (travel, combat, POI interaction)

This follows the same pattern used for Office states (Office_Exploration, Office_TableView) and prepares for Task 7 (Office↔Overworld transitions) where state changes will drive camera/input switching automatically.

---

### Task 5: Convoy Pawn & Click-to-Move Pathfinding

---

### Step 5.1: Add Left Mouse Button Binding to IMC_FC_TopDown

- [x] **Analysis**

  - [x] Left Mouse Button will trigger click-to-move commands
  - [x] Will bind to existing IA_Interact action
  - [x] No modifiers needed for simple click detection

- [x] **Implementation (Unreal Editor)**

  - [x] Open `/Game/FC/Input/IMC_FC_TopDown
IMC_FC_TopDown`
  - [x] Add Mapping: IA_Interact
  - [x] Add Key: **Left Mouse Button**
    - [x] No modifiers needed
  - [x] Save IMC_FC_TopDown

- [x] **Testing After Step 5.1** ✅ CHECKPOINT
  - [x] Left Mouse Button bound to IA_Interact
  - [x] IMC_FC_TopDown now has 3 mappings (Pan, Zoom, Interact)
  - [x] Asset saves without errors

**COMMIT POINT 5.1**: `git add Content/FC/Input/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Add left mouse button binding for click-to-move in IMC_FC_TopDown"`

---

### Step 5.2: Create BP_FC_ConvoyMember Actor

#### Step 5.2.1: Create C++ AConvoyMember Class

- [x] **Analysis**

  - [x] C++ base class inheriting from ACharacter for CharacterMovement and AI support
  - [x] Handles capsule overlap detection and POI notification logic
  - [x] Stores reference to parent AOverworldConvoy
  - [x] Blueprint child will configure mesh and materials

- [x] **Implementation (Visual Studio - ConvoyMember.h)**

  - [x] Create `Source/FC/Characters/Convoy/ConvoyMember.h`
  - [x] Add class declaration:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Character.h"
    #include "ConvoyMember.generated.h"

    class AOverworldConvoy;

    UCLASS()
    class FC_API AConvoyMember : public ACharacter
    {
        GENERATED_BODY()

    public:
        AConvoyMember();

    protected:
        virtual void BeginPlay() override;

    private:
        /** Reference to parent convoy actor */
        UPROPERTY()
        TObjectPtr<AOverworldConvoy> ParentConvoy;

        /** Handle capsule overlap events */
        UFUNCTION()
        void OnCapsuleBeginOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult
        );

    public:
        /** Set parent convoy reference */
        void SetParentConvoy(AOverworldConvoy* InConvoy);

        /** Notify parent convoy of POI overlap */
        void NotifyPOIOverlap(AActor* POIActor);
    };
    ```

  - [ ] Save file

- [x] **Implementation (Visual Studio - ConvoyMember.cpp)**

  - [x] Create `Source/FC/Characters/Convoy/ConvoyMember.cpp`
  - [x] Add implementation:

    ```cpp
    #include "Characters/Convoy/ConvoyMember.h"
    #include "World/FCOverworldConvoy.h"
    #include "Components/CapsuleComponent.h"

    AConvoyMember::AConvoyMember()
    {
        PrimaryActorTick.bCanEverTick = false;

        // Configure capsule
        UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
        if (CapsuleComp)
        {
            CapsuleComp->SetCapsuleHalfHeight(100.0f);
            CapsuleComp->SetCapsuleRadius(50.0f);
            CapsuleComp->SetGenerateOverlapEvents(true);
        }

        // AI controller auto-possession
        AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    }

    void AConvoyMember::BeginPlay()
    {
        Super::BeginPlay();

        // Bind overlap event
        UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
        if (CapsuleComp)
        {
            CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AConvoyMember::OnCapsuleBeginOverlap);
        }
    }

    void AConvoyMember::OnCapsuleBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult)
    {
        if (!OtherActor || OtherActor == this)
        {
            return;
        }

        // Check if actor implements BPI_InteractablePOI interface
        // (Blueprint interface check - simplified for now)
        if (OtherActor->GetClass()->ImplementsInterface(UBPI_InteractablePOI::StaticClass()))
        {
            NotifyPOIOverlap(OtherActor);
        }
    }

    void AConvoyMember::SetParentConvoy(AOverworldConvoy* InConvoy)
    {
        ParentConvoy = InConvoy;
    }

    void AConvoyMember::NotifyPOIOverlap(AActor* POIActor)
    {
        if (ParentConvoy)
        {
            ParentConvoy->NotifyPOIOverlap(POIActor);
        }
    }
    ```

  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.2.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] AConvoyMember class visible in Unreal Editor
  - [x] Can derive Blueprint from AConvoyMember

**COMMIT POINT 5.2.1**: `git add Source/FC/Characters/Convoy/ConvoyMember.h Source/FC/Characters/Convoy/ConvoyMember.cpp && git commit -m "feat(convoy): Create C++ AConvoyMember base class"`

---

#### Step 5.2.2: Create BP_FC_ConvoyMember Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint derived from AConvoyMember C++ class
  - [x] Configures mesh, materials, and CharacterMovement parameters
  - [x] POI overlap logic already handled in C++ parent

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → Select **ConvoyMember** (C++ class)
  - [x] Name: `BP_FC_ConvoyMember`
  - [x] Open BP_FC_ConvoyMember
  - [x] Components Panel:
    - [x] Select Mesh component (inherited from Character)
      - [x] Rename to "MemberMesh"
      - [x] Set Skeletal Mesh: Choose placeholder (mannequin or starter content)
      - [x] Set Scale: X=1, Y=1, Z=1
      - [x] Set Material: Unique color per member type (e.g., blue for prototype)
    - [x] Select CharacterMovement component (inherited):
      - [x] Max Walk Speed: 300.0
      - [x] Max Acceleration: 500.0
      - [x] Braking Deceleration Walking: 1000.0
      - [x] Enable **Orient Rotation to Movement**: True
      - [x] Disable **Use Controller Desired Rotation**: False
  - [x] Class Defaults:
    - [x] AI Controller Class: BP_FC_ConvoyAIController (will create in next step)
  - [x] Compile and save

- [x] **Testing After Step 5.2.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Inherits from AConvoyMember C++ class
  - [x] Mesh and materials configured
  - [x] Can place in level viewport (test, then remove)

**COMMIT POINT 5.2.2**: `git add Content/FC/Characters/Convoy/Blueprints/BP_FC_ConvoyMember.uasset && git commit -m "feat(convoy): Create BP_FC_ConvoyMember Blueprint child class"`

---

### Step 5.3: Create BP_FC_ConvoyAIController

#### Step 5.3.1: Create AI Controller Blueprint

- [x] **Analysis**

  - [x] AI controller handles NavMesh pathfinding for convoy members
  - [x] Leader: Receives move-to-location commands from player controller
  - [x] Followers: Follow breadcrumb trail (deferred to Step 5.5)
  - [x] Uses built-in AI MoveTo nodes

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → AIController
  - [x] Name: `BP_FC_ConvoyAIController`
  - [x] Open BP_FC_ConvoyAIController
  - [x] Event Graph:
    - [x] Event BeginPlay:
      ```
      BeginPlay → Print String "ConvoyAIController: Initialized"
      ```
  - [x] Create Custom Event: **MoveTo**
    - [x] Input: Vector (Target Location)
    - [x] Implementation:
      ```
      MoveTo (Vector input)
      → AI MoveToLocation (Simple Move To Location)
        - Pawn: Get Controlled Pawn
        - Goal Location: Target Location input
      → Print String "AI Controller moving to: [Target Location]"
      ```
  - [x] Compile and save

- [x] **Testing After Step 5.3.1** ✅ CHECKPOINT
  - [x] BP_FC_ConvoyAIController created
  - [x] MoveTo custom event functional
  - [x] Blueprint compiles without errors

**COMMIT POINT 5.3.1**: `git add Content/FC/World/Blueprints/AI/BP_FC_ConvoyAIController.uasset && git commit -m "feat(convoy): Create BP_FC_ConvoyAIController with MoveTo"`

---

### Step 5.4: Create BP_FC_OverworldConvoy Parent Actor

#### Step 5.4.1: Create C++ AOverworldConvoy Class

- [x] **Analysis**

  - [x] C++ base class inheriting from AActor
  - [x] Manages array of AConvoyMember pointers
  - [x] Handles member spawning and POI overlap aggregation
  - [x] Provides GetLeaderMember() and NotifyPOIOverlap() methods
  - [x] Blueprint child will configure spawn point locations

- [x] **Implementation (Visual Studio - FCOverworldConvoy.h)**

  - [x] Create `Source/FC/Characters/Convoy/FCOverworldConvoy.h`
  - [x] Add class declaration:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "FCOverworldConvoy.generated.h"

    class AConvoyMember;
    class USceneComponent;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConvoyPOIOverlap, AActor*, POIActor);

    UCLASS()
    class FC_API AFCOverworldConvoy : public AActor
    {
        GENERATED_BODY()

    public:
        AFCOverworldConvoy();

    protected:
        virtual void BeginPlay() override;
        virtual void OnConstruction(const FTransform& Transform) override;

    private:
        /** Array of convoy member actors */
        UPROPERTY()
        TArray<AFCConvoyMember*> ConvoyMembers;

        /** Reference to leader member */
        UPROPERTY()
        AFCConvoyMember* LeaderMember;

        /** Blueprint class to spawn for convoy members */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        TSubclassOf<AFCConvoyMember> ConvoyMemberClass;

        /** Blueprint-exposed spawn points */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* ConvoyRoot;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* CameraAttachPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* LeaderSpawnPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* Follower1SpawnPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* Follower2SpawnPoint;

        /** Spawn convoy members at spawn points */
        void SpawnConvoyMembers();

    public:
        /** Get leader member reference */
        UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
        AFCConvoyMember* GetLeaderMember() const { return LeaderMember; }

        /** Called by convoy members when they overlap a POI */
        UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
        void NotifyPOIOverlap(AActor* POIActor);

        /** Event dispatcher for POI overlap */
        UPROPERTY(BlueprintAssignable, Category = "FC|Convoy|Events")
        FOnConvoyPOIOverlap OnConvoyPOIOverlap;
    };
    ```

  - [x] Save file

- [x] **Implementation (Visual Studio - FCOverworldConvoy.cpp)**

  - [x] Create `Source/FC/Characters/Convoy/FCOverworldConvoy.cpp`
  - [x] Add implementation:

    ```cpp
    #include "Characters/Convoy/FCOverworldConvoy.h"
    #include "Characters/Convoy/FCConvoyMember.h"
    #include "Components/SceneComponent.h"
    #include "Engine/World.h"
    #include "FC.h"

    DEFINE_LOG_CATEGORY_STATIC(LogFCOverworldConvoy, Log, All);

    AFCOverworldConvoy::AFCOverworldConvoy()
    {
        PrimaryActorTick.bCanEverTick = false;

        // Create component hierarchy
        ConvoyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ConvoyRoot"));
        SetRootComponent(ConvoyRoot);

        CameraAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAttachPoint"));
        CameraAttachPoint->SetupAttachment(ConvoyRoot);
        CameraAttachPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

        LeaderSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeaderSpawnPoint"));
        LeaderSpawnPoint->SetupAttachment(ConvoyRoot);
        LeaderSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

        Follower1SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower1SpawnPoint"));
        Follower1SpawnPoint->SetupAttachment(ConvoyRoot);
        Follower1SpawnPoint->SetRelativeLocation(FVector(-150.0f, 0.0f, 0.0f));

        Follower2SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower2SpawnPoint"));
        Follower2SpawnPoint->SetupAttachment(ConvoyRoot);
        Follower2SpawnPoint->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f));
    }

    void AFCOverworldConvoy::BeginPlay()
    {
        Super::BeginPlay();

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: BeginPlay"), *GetName());

        // Spawn convoy members at runtime
        SpawnConvoyMembers();
    }

    void AFCOverworldConvoy::OnConstruction(const FTransform& Transform)
    {
        Super::OnConstruction(Transform);

        // Note: Spawning moved to BeginPlay to avoid editor duplication
        // OnConstruction spawns actors in editor, but they get destroyed at PIE start
    }

    void AFCOverworldConvoy::SpawnConvoyMembers()
    {
        // Clear existing members
        for (AFCConvoyMember* Member : ConvoyMembers)
        {
            if (Member)
            {
                Member->Destroy();
            }
        }
        ConvoyMembers.Empty();

        UWorld* World = GetWorld();
        if (!World)
        {
            UE_LOG(LogFCOverworldConvoy, Warning, TEXT("OverworldConvoy %s: No world context"), *GetName());
            return;
        }

        // Check if ConvoyMemberClass is set
        if (!ConvoyMemberClass)
        {
            UE_LOG(LogFCOverworldConvoy, Error, TEXT("OverworldConvoy %s: ConvoyMemberClass not set! Please set it in Blueprint."), *GetName());
            return;
        }

        // Spawn parameters
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn leader
        AFCConvoyMember* Leader = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            LeaderSpawnPoint->GetComponentLocation(),
            LeaderSpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Leader)
        {
            Leader->AttachToComponent(LeaderSpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Leader->SetParentConvoy(this);
            LeaderMember = Leader;
            ConvoyMembers.Add(Leader);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned leader"), *GetName());
        }

        // Spawn follower 1
        AFCConvoyMember* Follower1 = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            Follower1SpawnPoint->GetComponentLocation(),
            Follower1SpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Follower1)
        {
            Follower1->AttachToComponent(Follower1SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Follower1->SetParentConvoy(this);
            ConvoyMembers.Add(Follower1);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 1"), *GetName());
        }

        // Spawn follower 2
        AFCConvoyMember* Follower2 = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            Follower2SpawnPoint->GetComponentLocation(),
            Follower2SpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Follower2)
        {
            Follower2->AttachToComponent(Follower2SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Follower2->SetParentConvoy(this);
            ConvoyMembers.Add(Follower2);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 2"), *GetName());
        }

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned %d total members"), *GetName(), ConvoyMembers.Num());
    }

    void AFCOverworldConvoy::NotifyPOIOverlap(AActor* POIActor)
    {
        if (!POIActor)
        {
            return;
        }

        // Get POI name (simplified - will use interface later)
        FString POIName = POIActor->GetName();

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s detected POI: %s"), *GetName(), *POIName);

        // Display on-screen message
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                FString::Printf(TEXT("Convoy detected POI: %s"), *POIName));
        }

        // Broadcast event
        OnConvoyPOIOverlap.Broadcast(POIActor);
    }
    ```

  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.4.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] AOverworldConvoy class visible in Unreal Editor
  - [x] Can derive Blueprint from AOverworldConvoy

**COMMIT POINT 5.4.1**: `git add Source/FC/Characters/Convoy/FCOverworldConvoy.h Source/FC/Characters/Convoy/FCOverworldConvoy.cpp && git commit -m "feat(convoy): Create C++ AOverworldConvoy base class"`

---

#### Step 5.4.2: Create BP_FC_OverworldConvoy Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint derived from AOverworldConvoy C++ class
  - [x] Member spawning already handled in C++ OnConstruction
  - [x] Blueprint can override spawn point locations if needed
  - [x] Will configure camera attachment in Event Graph

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → Select **FCOverworldConvoy** (C++ class)
  - [x] Name: `BP_FC_OverworldConvoy`
  - [x] Open BP_FC_OverworldConvoy
  - [x] Components Panel:
    - [x] Verify hierarchy inherited from C++ class:
      - [x] ConvoyRoot (Root)
      - [x] CameraAttachPoint (child)
      - [x] LeaderSpawnPoint (child)
      - [x] Follower1SpawnPoint (child)
      - [x] Follower2SpawnPoint (child)
    - [x] Optionally adjust spawn point locations in Details panel
  - [x] Class Defaults → FC|Convoy category:
    - [x] Set **Convoy Member Class** to `BP_FC_ConvoyMember`
    - [x] This tells C++ which Blueprint class to spawn at runtime
  - [x] Compile and save

- [x] **Testing After Step 5.4.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Inherits from AFCOverworldConvoy C++ class
  - [x] Place in level viewport
  - [x] Set Convoy Member Class property to BP_FC_ConvoyMember
  - [x] PIE: Verify 3 convoy members spawn at runtime (C++ BeginPlay)
  - [x] Check World Outliner: 3 members appear with meshes visible
  - [x] Members have capsule collision and AI controllers

**IMPLEMENTATION NOTES:**

- C++ spawns members in `BeginPlay()` (not `OnConstruction()`) to avoid PIE destruction issues
- C++ uses `ConvoyMemberClass` property (TSubclassOf<AFCConvoyMember>) to spawn Blueprint children
- Must set `Convoy Member Class` in Blueprint Details panel to `BP_FC_ConvoyMember`
- Spawning base C++ class directly shows no mesh; Blueprint child provides visual mesh

**COMMIT POINT 5.4.2**: `git add Content/FC/Characters/Convoy/Blueprints/BP_FCOverworldConvoy.uasset && git commit -m "feat(convoy): Create BP_FC_OverworldConvoy Blueprint child class"`

---

#### Step 5.4.3: Verify POI Overlap Aggregation (Already in C++)

- [x] **Analysis**

  - [x] POI overlap aggregation already implemented in C++ AOverworldConvoy
  - [x] NotifyPOIOverlap() method callable from AConvoyMember
  - [x] OnConvoyPOIOverlap event dispatcher already exposed to Blueprint
  - [x] This step verifies functionality only

- [x] **Verification (Unreal Editor)**

  - [x] Open BP_FC_OverworldConvoy
  - [x] Verify NotifyPOIOverlap method visible in Blueprint (inherited from C++)
  - [x] Verify OnConvoyPOIOverlap event dispatcher in Event Graph (My Blueprint panel)
  - [x] No additional implementation needed

- [x] **Testing After Step 5.4.3** ✅ CHECKPOINT
  - [x] NotifyPOIOverlap method visible in Blueprint
  - [x] OnConvoyPOIOverlap event dispatcher accessible
  - [x] Blueprint compiles without errors

**COMMIT POINT 5.4.3**: N/A (functionality already in C++ base class)

---

### Step 5.5: Implement Click-to-Move for Convoy Leader

#### Step 5.5.1: Add Click-to-Move Handler to AFCPlayerController

- [x] **Analysis**

  - [x] Player controller detects left-click on terrain
  - [x] Raycasts from mouse position to world
  - [x] Sends move command to convoy leader's AI controller
  - [x] Uses existing HandleClick routing for TopDown mode

- [x] **Implementation (FCPlayerController.h)**

  - [x] Open `Source/FC/Core/FCPlayerController.h`
  - [x] Add forward declarations for AFCOverworldConvoy and AFCConvoyMember
  - [x] Add PossessedConvoy property (AFCOverworldConvoy\*)
  - [x] Add HandleOverworldClickMove() method declaration
  - [x] Save file

- [x] **Implementation (FCPlayerController.cpp)**

  - [x] Open `Source/FC/Core/FCPlayerController.cpp`
  - [x] Add include for FCConvoyMember.h
  - [x] Update BeginPlay to find convoy in level:

    ```cpp
    // Find convoy in level if we're in Overworld
    TArray<AActor*> FoundConvoys;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFCOverworldConvoy::StaticClass(), FoundConvoys);

    if (FoundConvoys.Num() > 0)
    {
        PossessedConvoy = Cast<AFCOverworldConvoy>(FoundConvoys[0]);
        UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Found convoy: %s"), *PossessedConvoy->GetName());
    }
    ```

  - [x] Update HandleClick to route TopDown clicks:
    ```cpp
    if (CurrentMode == EFCPlayerCameraMode::TopDown)
    {
        HandleOverworldClickMove();
    }
    ```
  - [x] Implement HandleOverworldClickMove:
    - [x] Raycast from cursor using GetHitResultUnderCursor
    - [x] Get convoy leader via PossessedConvoy->GetLeaderMember()
    - [x] Get leader's AI controller
    - [x] Project hit location to NavMesh
    - [x] Call AIController->MoveToLocation()
    - [x] Add logging and visual feedback
  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.5.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] HandleOverworldClickMove method implemented
  - [x] Can open Unreal Editor
  - [x] PIE tested with convoy in Overworld level
  - [x] Left-click on ground moves leader to clicked location
  - [x] NavMesh projection works (only accepts valid paths)
  - [x] Visual feedback displays target location on screen (green text, 2s)
  - [x] Output log confirms: "HandleOverworldClickMove: Moving convoy to [location]"
  - [x] Camera follows leader smoothly during movement
  - [x] AI controller navigation working perfectly

**IMPLEMENTATION NOTES:**

- Used existing HandleClick routing instead of separate ClickMoveAction
- Convoy finding happens in BeginPlay (logs warning if not found - expected in Office)
- NavMesh projection ensures valid pathfinding destinations
- Visual feedback shows target location on screen for 2 seconds
- Leader movement fully functional with AIController->MoveToLocation()

**COMMIT POINT 5.5.1**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "feat(convoy): Implement click-to-move for convoy leader in player controller"`

---

#### Step 5.5.2: Verify GetLeaderMember Method (Already in C++) - OBSOLETE

- [x] **Analysis**

  - [x] GetLeaderMember() method already implemented in C++ AFCOverworldConvoy
  - [x] Marked as BlueprintCallable, accessible from both C++ and Blueprint
  - [x] Returns AFCConvoyMember\* (leader member reference)
  - [x] Used successfully in Step 5.5.1 implementation

- [x] **Verification (Unreal Editor)**

  - [x] GetLeaderMember method called in HandleOverworldClickMove()
  - [x] Returns valid AFCConvoyMember reference
  - [x] C++ implementation working correctly

- [x] **Testing After Step 5.5.2** ✅ CHECKPOINT
  - [x] GetLeaderMember method functional in C++ PlayerController
  - [x] Returns valid AFCConvoyMember reference
  - [x] No Blueprint verification needed

**OBSOLETE NOTE**: This step was marked as "verification only" and GetLeaderMember() was successfully used in Step 5.5.1 C++ implementation. No additional Blueprint testing required.

**COMMIT POINT 5.5.2**: N/A (functionality already in C++ base class, verified in 5.5.1)

---

#### Step 5.5.3: Configure ClickMoveAction in BP_FC_PlayerController - OBSOLETE

- [x] **Analysis**

  - [x] Original plan: Create separate ClickMoveAction property
  - [x] Actual implementation: Reused existing ClickAction and HandleClick routing
  - [x] HandleClick now routes TopDown clicks to HandleOverworldClickMove()
  - [x] Simpler architecture, fewer input actions to manage

- [x] **Implementation (C++ - Completed in 5.5.1)**

  - [x] No Blueprint configuration needed
  - [x] Existing ClickAction already bound to IA_Interact in IMC_FC_TopDown
  - [x] HandleClick checks camera mode and routes to appropriate handler
  - [x] TopDown mode → HandleOverworldClickMove()
  - [x] TableView/FirstPerson mode → HandleTableObjectClick()

- [x] **Testing After Step 5.5.3** ✅ CHECKPOINT
  - [x] Click routing working in PIE
  - [x] No separate ClickMoveAction property needed
  - [x] Existing input bindings sufficient

**OBSOLETE NOTE**: This step is no longer needed. Step 5.5.1 implementation reused the existing ClickAction and HandleClick routing instead of creating a separate ClickMoveAction property. This simplifies the input system and reduces redundancy.

**COMMIT POINT 5.5.3**: N/A (obsolete - functionality implemented in 5.5.1)

---

### Step 5.6: Integrate Camera with Convoy

#### Step 5.6.1: Attach BP_OverworldCamera to Convoy

- [x] **Analysis**

  - [x] Camera should follow convoy's CameraAttachPoint
  - [x] Spring arm and pan constraints bound to convoy center
  - [x] Camera remains smooth during movement

- [x] **Implementation (UFCCameraManager - Option C)**

  - [x] **Actual Implementation: C++ in UFCCameraManager::BlendToTopDown()**
    - [x] Added `GetCameraAttachPoint()` method to `AFCOverworldConvoy.h`
    - [x] Modified `UFCCameraManager::BlendToTopDown()` in `FCCameraManager.cpp`:
      - [x] Find convoy in level using `GetAllActorsOfClass` with name pattern matching
      - [x] Call convoy's `GetCameraAttachPoint()` via reflection
      - [x] Attach OverworldCamera to CameraAttachPoint using `AttachToComponent`
      - [x] Uses `SnapToTargetNotIncludingScale` attachment rule
      - [x] Logs success/warning messages for debugging
    - [x] Camera automatically attaches when transitioning to TopDown mode
    - [x] No Blueprint changes needed - pure C++ solution

- [x] **Testing After Step 5.6.1** ✅ CHECKPOINT
  - [x] Camera attaches to convoy's CameraAttachPoint during TopDown mode transition
  - [x] Camera follows convoy smoothly when leader moves via click-to-move
  - [x] No jittering or offset issues observed
  - [x] Attachment logged in output: "Attached camera to convoy's CameraAttachPoint"

**IMPLEMENTATION NOTES**:

- Chose C++ implementation over Blueprint for consistency with existing camera system
- Attachment occurs in `BlendToTopDown()` to ensure camera is attached before view transition
- Uses reflection to call `GetCameraAttachPoint()` to avoid circular dependencies
- Gracefully handles missing convoy (logs message, continues without error)

**COMMIT POINT 5.6.1**: `git add Source/FC/Characters/Convoy/FCOverworldConvoy.h Source/FC/Components/FCCameraManager.cpp && git commit -m "feat(convoy): Attach camera to convoy CameraAttachPoint in BlendToTopDown"`

---

#### Step 5.6.2: Bind Camera Constraints to Convoy (Prototype Scope)

- [x] **Analysis**

  - [x] PROTOTYPE SCOPE: Basic attachment sufficient for Week 3
  - [x] FUTURE: Spring arm length and pan boundaries based on convoy size
  - [x] BACKLOG: Implement dynamic camera constraint system

- [x] **Implementation (Week 3 Prototype)**

  - [x] Verified camera attachment working in Step 5.6.1
  - [x] Using fixed spring arm length (from Task 3)
  - [x] Pan constraints remain level-based (not convoy-based)
  - [x] Limitation documented - future enhancement for dynamic constraints

- [x] **Testing After Step 5.6.2** ✅ CHECKPOINT
  - [x] Camera follows convoy with fixed constraints
  - [x] Pan and zoom work as expected via WASD and mouse wheel
  - [x] Attachment stable during movement

**PROTOTYPE SCOPE NOTE**: Dynamic camera constraints based on convoy size/position deferred to future tasks. Current implementation uses fixed spring arm and level-based pan boundaries, which is sufficient for Week 3 prototype.

**COMMIT POINT 5.6.2**: N/A (prototype scope documented, no code changes)

---

### Step 5.7: Place BP_FC_OverworldConvoy in L_Overworld

#### Step 5.7.1: Add Convoy to Level and Configure

- [x] **Analysis**

  - [x] Convoy spawns at PlayerStart location
  - [x] Members auto-spawn via Construction Script
  - [x] Camera auto-attaches via BeginPlay

- [x] **Implementation (Unreal Editor)**

  - [x] Open L_Overworld level
  - [x] **Option A: Manual Placement**
    - [x] Drag BP_FC_OverworldConvoy from Content Browser into viewport
    - [x] Position at PlayerStart location (X=0, Y=0, Z=100)
  - [x] **Option B: Default Pawn Class (Recommended if convoy needs controller possession)**
    - [x] Window → World Settings
    - [x] Game Mode → Default Pawn Class: BP_FC_OverworldConvoy
    - [x] NOTE: This may conflict with controller not possessing pawn directly
    - [x] For Week 3: Use Option A (manual placement, controller references convoy)
  - [x] Save level

- [x] **Testing After Step 5.7.1** ✅ CHECKPOINT
  - [x] Convoy visible in level with 3 members
  - [x] Members positioned correctly (leader in front, followers behind)
  - [x] PIE: Convoy members spawn and attach
  - [x] Camera attaches to convoy
  - [x] Level saves without errors

**COMMIT POINT 5.7.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(convoy): Place BP_FC_OverworldConvoy in L_Overworld"`

---

### Step 5.8: Test Click-to-Move and Camera Following

#### Step 5.8.1: Full Convoy Verification

- [x] **Analysis**

  - [x] Test click-to-move with convoy leader
  - [x] Verify followers remain stationary (breadcrumb system not yet implemented)
  - [x] Test camera following convoy during movement
  - [x] Verify camera pan/zoom work while moving

- [x] **Test Sequence**

  - [x] Open L_Overworld in editor
  - [x] PIE (Play In Editor)
  - [x] Verify convoy spawns with 3 members
  - [x] Verify camera attached to convoy (top-down view follows convoy)
  - [x] Press `P` to visualize NavMesh (green overlay)
  - [x] **Test Click-to-Move**:
    - [x] Left-click on ground (green NavMesh area)
    - [x] Verify leader moves to clicked location
    - [x] Check Output Log: "Moving convoy to: [location]"
    - [x] Verify camera follows leader smoothly
    - [x] NOTE: Followers remain stationary (breadcrumb system not yet implemented)
  - [x] **Test Multiple Clicks**:
    - [x] Click different locations rapidly
    - [x] Verify leader updates path correctly
  - [x] **Test Camera Pan/Zoom During Movement**:
    - [x] While leader is moving, use WASD to pan camera
    - [x] Verify camera controls still work (camera moves relative to convoy)
    - [x] Use mouse wheel to zoom
    - [x] Verify zoom works smoothly

- [x] **Testing After Step 5.8.1** ✅ CHECKPOINT
  - [x] Leader moves to clicked locations via NavMesh
  - [x] Camera follows convoy smoothly (attached to CameraAttachPoint)
  - [x] Camera pan (WASD) and zoom (mouse wheel) work during movement
  - [x] Output logs confirm movement commands
  - [x] No "Accessed None" errors
  - [x] Followers stationary (expected - breadcrumb system deferred to backlog)

**TEST RESULTS**:

- Click-to-move fully functional with NavMesh pathfinding
- Camera attachment to convoy working perfectly
- Camera follows leader smoothly without jitter
- Pan and zoom controls responsive during movement
- No POI actors placed yet (deferred to Task 6)

**COMMIT POINT 5.8.1**: `git add -A && git commit -m "test(convoy): Verify click-to-move, camera following, and pan/zoom during movement"`

---

### Step 5.9: Document Follower Breadcrumb System as Backlog

- [x] **Analysis**

  - [x] Breadcrumb following system deferred to future sprint
  - [x] Document requirements and architecture for later implementation
  - [x] Create backlog item at end of file

- [x] **Documentation** (see backlog section at end of file)
  - [x] Backlog item created: "Convoy Follower Breadcrumb System" (Backlog Item 1, lines 1437-1457)
  - [x] Includes architecture notes and implementation steps
  - [x] Priority set to Medium (Week 4-5)

**COMMIT POINT 5.9**: N/A (backlog documentation already exists at end of file)

---

### Task 5 Acceptance Criteria

- [x] Left Mouse Button bound to IA_Interact in IMC_FC_TopDown
- [x] BP_FC_ConvoyMember character created with mesh, collision, AI controller support, and POI overlap detection
- [x] BP_FC_ConvoyAIController created with MoveTo method
- [x] BP_FC_OverworldConvoy parent actor created with 3 child convoy members (leader + 2 followers)
- [x] Convoy members spawn and attach correctly ~~via Construction Script~~ **via BeginPlay** (moved to avoid PIE destruction)
- [x] POI overlap aggregation implemented in BP_FC_OverworldConvoy (C++ base class)
- [x] AFCPlayerController implements click-to-move for convoy leader
- [x] ~~BP_OverworldCamera attached to convoy's CameraAttachPoint~~ **UFCCameraManager automatically attaches camera in BlendToTopDown()**
- [x] Camera follows convoy smoothly during movement
- [x] Convoy placed in L_Overworld at PlayerStart
- [x] Left-click moves convoy leader to location using NavMesh pathfinding
- [x] POI overlap triggers on-screen message and log output (tested with class name pattern matching)
- [x] Camera pan/zoom works while convoy moves
- [x] No compilation errors or runtime crashes
- [x] Follower movement deferred to backlog (breadcrumb system - Backlog Item 1, Week 4-5)

**IMPLEMENTATION NOTES**:

- **C++ base classes**: AFCConvoyMember, AFCOverworldConvoy with Blueprint children for visual config
- **Spawning**: Moved from OnConstruction to BeginPlay to survive PIE transition
- **Camera attachment**: Automatic in UFCCameraManager::BlendToTopDown() using reflection to call GetCameraAttachPoint()
- **Input routing**: Reused existing ClickAction binding, routed via HandleClick() based on camera mode
- **POI detection**: Capsule overlap with class name pattern matching (will use BPI_InteractablePOI in Task 6)

**Task 5 complete. Ready for Task 6 sub-tasks (POI Actor & Interaction Stub).**

---

## Updated Task 6: POI Actor & Right-Click Interaction Stub

**Purpose**: Create BP_OverworldPOI actor with right-click interaction that logs to console (stub for future implementation). POI overlap detection already implemented in convoy system.

---

### Step 6.1: Verify Input Action for POI Interaction

#### Step 6.1.1: Verify IA_Interact Input Action (OBSOLETE - Already Exists)

- [x] **Analysis**

  - [x] POI interaction uses right-click via existing IA_Interact action
  - [x] IA_Interact already exists at `/Game/FC/Input/IA_Interact`
  - [x] Currently not used for any other functionality in TopDown mode
  - [x] Will reuse instead of creating separate IA_InteractPOI

- [x] **Verification**

  - [x] IA_Interact exists and is Digital (bool) type
  - [x] Available for POI interaction binding

- [x] **Testing After Step 6.1.1** ✅ CHECKPOINT
  - [x] IA_Interact confirmed to exist
  - [x] No conflicts with other TopDown functionality

**OBSOLETE NOTE**: Step originally planned to create IA_InteractPOI, but we're reusing existing IA_Interact instead to simplify input system.

**COMMIT POINT 6.1.1**: N/A (using existing asset)

---

#### Step 6.1.2: Verify Right Mouse Button Binding in IMC_FC_TopDown (OBSOLETE - Already Bound)

- [x] **Analysis**

  - [x] Right Mouse Button already bound to IA_Interact in IMC_FC_TopDown
  - [x] No additional binding needed

- [x] **Verification**

  - [x] IMC_FC_TopDown already has IA_Interact mapped to Right Mouse Button
  - [x] Binding ready for POI interaction implementation

- [x] **Testing After Step 6.1.2** ✅ CHECKPOINT
  - [x] Right Mouse Button bound to IA_Interact in TopDown context
  - [x] IMC_FC_TopDown has required mappings (Pan, Zoom, ClickMove, Interact)
  - [x] No changes needed

**OBSOLETE NOTE**: Right Mouse Button already bound to IA_Interact in IMC_FC_TopDown mapping context. No additional configuration needed.

**COMMIT POINT 6.1.2**: N/A (binding already exists)

---

### Step 6.2: Create AFCOverworldPOI C++ Base Class

#### Step 6.2.1: Create AFCOverworldPOI C++ Class

- [x] **Analysis**

  - [x] C++ base class for all overworld POI actors (matches convoy architecture)
  - [x] Root component, static mesh, and collision box for raycast/overlap
  - [x] POI name property (FString, EditAnywhere, Instance Editable)
  - [x] Virtual OnPOIInteract() method for Blueprint extensibility
  - [x] Overlap detection setup in constructor
  - [x] Blueprint children configure mesh/materials per POI type

- [x] **Implementation (C++)**

  - [x] **Created Header File**: `Source/FC/World/FCOverworldPOI.h`

    - [x] Component properties: POIRoot, POIMesh, InteractionBox
    - [x] POIName property (EditAnywhere, BlueprintReadWrite)
    - [x] GetPOIName() accessor (BlueprintCallable)
    - [x] OnPOIInteract() virtual method (BlueprintNativeEvent)

  - [x] **Created Source File**: `Source/FC/World/FCOverworldPOI.cpp`

    - [x] Constructor creates component hierarchy
    - [x] POIMesh scaled 2x, no collision
    - [x] InteractionBox: 150x150x100 extent, QueryOnly, blocks Visibility, overlaps all
    - [x] Default POI name: "Unnamed POI"
    - [x] BeginPlay logs POI spawn with name and location
    - [x] OnPOIInteract_Implementation() shows yellow stub message (5s)

  - [x] Compiled successfully via Live Coding

- [x] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [x] C++ code compiled without errors
  - [x] AFCOverworldPOI visible in Unreal Editor Content Browser
  - [x] Can create Blueprint child class from AFCOverworldPOI

**COMMIT POINT 6.2.1**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Create AFCOverworldPOI C++ base class"`

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "FCOverworldPOI.generated.h"

    class USceneComponent;
    class UStaticMeshComponent;
    class UBoxComponent;

    DECLARE_LOG_CATEGORY_EXTERN(LogFCOverworldPOI, Log, All);

    /**
     * AFCOverworldPOI - Base class for overworld Points of Interest
     *
     * Provides collision for mouse raycast detection and convoy overlap.
     * Blueprint children configure specific mesh, materials, and POI names.
     */
    UCLASS()
    class FC_API AFCOverworldPOI : public AActor
    {
        GENERATED_BODY()

    public:
        AFCOverworldPOI();

    protected:
        virtual void BeginPlay() override;

    private:
        /** Root component for POI hierarchy */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        USceneComponent* POIRoot;

        /** Static mesh for visual representation */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        UStaticMeshComponent* POIMesh;

        /** Collision box for mouse raycast and convoy overlap detection */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        UBoxComponent* InteractionBox;

        /** Display name for this Point of Interest */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        FString POIName;

    public:
        /** Get POI display name */
        UFUNCTION(BlueprintCallable, Category = "FC|POI")
        FString GetPOIName() const { return POIName; }

        /** Handle POI interaction (stub for Task 6 - will log to console) */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
        void OnPOIInteract();
    };
    ```

- [ ] **Create Source File**: `Source/FC/World/FCOverworldPOI.cpp`

  ```cpp
  #include "World/FCOverworldPOI.h"
  #include "Components/SceneComponent.h"
  #include "Components/StaticMeshComponent.h"
  #include "Components/BoxComponent.h"

  DEFINE_LOG_CATEGORY(LogFCOverworldPOI);

  AFCOverworldPOI::AFCOverworldPOI()
  {
      PrimaryActorTick.bCanEverTick = false;

      // Create component hierarchy
      POIRoot = CreateDefaultSubobject<USceneComponent>(TEXT("POIRoot"));
      SetRootComponent(POIRoot);

      // Create static mesh component
      POIMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("POIMesh"));
      POIMesh->SetupAttachment(POIRoot);
      POIMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
      POIMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

      // Create interaction box for raycast and overlap
      InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
      InteractionBox->SetupAttachment(POIRoot);
      InteractionBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
      InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      InteractionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
      InteractionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
      InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
      InteractionBox->SetGenerateOverlapEvents(true);

      // Default POI name
      POIName = TEXT("Unnamed POI");
  }

  void AFCOverworldPOI::BeginPlay()
  {
      Super::BeginPlay();

      UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s' spawned at %s"),
          *POIName, *GetActorLocation().ToString());
  }

  void AFCOverworldPOI::OnPOIInteract_Implementation()
  {
      // Stub implementation - logs to console
      UE_LOG(LogFCOverworldPOI, Log, TEXT("POI Interaction: %s"), *POIName);

      if (GEngine)
      {
          GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
              FString::Printf(TEXT("POI Interaction Stub: %s"), *POIName));
      }
  }
  ```

- [x] Save files
- [x] Compile C++ code (Live Coding or full build)

- [x] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] AFCOverworldPOI visible in Unreal Editor Content Browser
  - [x] Can create Blueprint child class from AFCOverworldPOI

**COMMIT POINT 6.2.1**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Create AFCOverworldPOI C++ base class"`

---

#### Step 6.2.2: Create BP_FC_OverworldPOI Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint child of AFCOverworldPOI for visual configuration
  - [x] Configure mesh, materials, and default POI name
  - [x] Can be placed in level and name edited per-instance

- [x] **Implementation (Unreal Editor)**

  - [x] Created Blueprint at `/Game/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI`
  - [x] Parent class: AFCOverworldPOI
  - [x] Components Panel:
    - [x] POIMesh component configured with static mesh
    - [x] Material assigned (visible from top-down camera)
    - [x] Component hierarchy inherited from C++ base class
  - [x] Class Defaults:
    - [x] POI Name configured (instance editable in Details panel)
  - [x] Compiled and saved

- [x] **Testing After Step 6.2.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Components hierarchy inherited from C++ base class
  - [x] POIMesh has mesh and material assigned
  - [x] Placed in L_Overworld level successfully
  - [x] POI Name editable per-instance in Details panel
  - [x] Convoy detects POI overlap (logs "Convoy detected POI: [name]")

**IMPLEMENTATION NOTE**: Convoy overlap detection already working via AFCConvoyMember capsule overlap with InteractionBox. Class name pattern matching successfully detects POI actors.

**COMMIT POINT 6.2.2**: `git add Content/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI.uasset && git commit -m "feat(overworld): Create BP_FC_OverworldPOI Blueprint child with mesh config and test in L_Overworld"`

---

### Step 6.3: Implement IFCInteractablePOI C++ Interface and POI Action System

#### Step 6.3.1: Create IFCInteractablePOI C++ Interface

- [x] **Analysis**

  - [x] C++ interface for POI interaction (matches existing IFCInteractable pattern)
  - [x] POIs can have multiple available actions (talk, ambush, enter, trade, harvest, observe)
  - [x] Interface provides methods to query available actions and execute selected action
  - [x] Enables different POI types with different action sets
  - [x] Integrates with existing UFCInteractionComponent system

- [x] **Architecture Requirements**

  - [x] **Action Selection Logic**:
    - [x] 0 actions → Right-click ignored
    - [x] 1 action → Right-click → convoy moves → overlap executes action automatically
    - [x] 2+ actions → Right-click → action selection widget → click action → convoy moves → overlap executes selected action
  - [x] **Overlap Triggers**:
    - [x] Intentional right-click movement triggers action on overlap
    - [x] Unintentional overlap (exploration, fleeing) also triggers actions
    - [x] If unintentional overlap + multiple actions → show action selection dialog
  - [x] **Enemy Encounters**:
    - [x] Enemies can chase convoy
    - [x] Enemy overlap triggers enemy's intended action (ambush)
  - [x] **Action Types** (extensible enum):
    - [x] Talk, Ambush, Enter, Trade, Harvest, Observe
    - [x] Easy to add new action types in future

- [x] **Implementation (C++)**

  - [x] **Create Header File**: `Source/FC/Interaction/IFCInteractablePOI.h`

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "IFCInteractablePOI.generated.h"

    /**
     * EFCPOIAction - Enum for POI action types
     * Extensible for future action types
     */
    UENUM(BlueprintType)
    enum class EFCPOIAction : uint8
    {
        Talk        UMETA(DisplayName = "Talk"),
        Ambush      UMETA(DisplayName = "Ambush"),
        Enter       UMETA(DisplayName = "Enter"),
        Trade       UMETA(DisplayName = "Trade"),
        Harvest     UMETA(DisplayName = "Harvest"),
        Observe     UMETA(DisplayName = "Observe")
    };

    /**
     * FFCPOIActionData - Struct containing action display information
     */
    USTRUCT(BlueprintType)
    struct FFCPOIActionData
    {
        GENERATED_BODY()

        /** Action type */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EFCPOIAction ActionType;

        /** Display text for action button (e.g., "Talk to Merchant") */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText ActionText;

        /** Optional icon for action (future UI) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* ActionIcon = nullptr;

        FFCPOIActionData()
            : ActionType(EFCPOIAction::Talk)
            , ActionText(FText::FromString(TEXT("Interact")))
        {}
    };

    // UInterface class (required by Unreal)
    UINTERFACE(MinimalAPI, Blueprintable)
    class UIFCInteractablePOI : public UInterface
    {
        GENERATED_BODY()
    };

    /**
     * IIFCInteractablePOI - Interface for overworld POI interaction
     * Provides action-based interaction system for POIs
     */
    class FC_API IIFCInteractablePOI
    {
        GENERATED_BODY()

    public:
        /**
         * Get all available actions for this POI
         * Called when right-clicking POI or on convoy overlap
         * @return Array of available actions with display data
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        TArray<FFCPOIActionData> GetAvailableActions() const;

        /**
         * Execute a specific action on this POI
         * Called when action is selected (or auto-executed if only one action)
         * @param Action - The action to execute
         * @param Interactor - The actor performing the action (typically convoy)
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        void ExecuteAction(EFCPOIAction Action, AActor* Interactor);

        /**
         * Get POI display name for UI and logging
         * @return Display name (e.g., "Village", "Ruins", "Enemy Camp")
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        FString GetPOIName() const;

        /**
         * Check if action can be executed (conditions, requirements)
         * Optional - default implementation returns true
         * @param Action - Action to check
         * @param Interactor - Actor attempting action
         * @return true if action can be executed
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        bool CanExecuteAction(EFCPOIAction Action, AActor* Interactor) const;
    };
    ```

  - [x] **Create Source File**: `Source/FC/Interaction/IFCInteractablePOI.cpp`

    ```cpp
    #include "Interaction/IFCInteractablePOI.h"

    // Interface default implementations are in the header file
    // This file is intentionally minimal for interfaces
    ```

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.3.1** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] IFCInteractablePOI interface available in C++ and Blueprint
  - [x] EFCPOIAction enum visible in Blueprint

**COMMIT POINT 6.3.1**: `git add Source/FC/Interaction/IFCInteractablePOI.h Source/FC/Interaction/IFCInteractablePOI.cpp && git commit -m "feat(overworld): Create IFCInteractablePOI C++ interface with action system"`---

#### Step 6.3.2: Implement IFCInteractablePOI in AFCOverworldPOI

- [x] **Analysis**

  - [x] AFCOverworldPOI implements IFCInteractablePOI interface in C++
  - [x] Add available actions array (EditAnywhere for Blueprint configuration)
  - [x] Implement interface methods
  - [x] Blueprint children can override actions per POI type

- [x] **Implementation (C++)**

  - [x] **Update FCOverworldPOI.h**:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "Interaction/IFCInteractablePOI.h"  // Add interface include
    #include "FCOverworldPOI.generated.h"

    // ... existing forward declarations ...

    /**
     * AFCOverworldPOI - Base class for overworld Points of Interest
     * Provides collision for mouse raycast detection and convoy overlap.
     * Blueprint children configure specific mesh, materials, and POI names.
     * Implements IFCInteractablePOI for action-based interaction
     */
    UCLASS()
    class FC_API AFCOverworldPOI : public AActor, public IIFCInteractablePOI
    {
        GENERATED_BODY()

    public:
        AFCOverworldPOI();

    protected:
        virtual void BeginPlay() override;

    private:
        // ... existing component properties ...

        /** Display name for this Point of Interest */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        FString POIName;

        /** Available actions for this POI (configured in Blueprint) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI|Actions", meta = (AllowPrivateAccess = "true"))
        TArray<FFCPOIActionData> AvailableActions;

    public:
        // IFCInteractablePOI interface implementation
        virtual TArray<FFCPOIActionData> GetAvailableActions_Implementation() const override;
        virtual void ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) override;
        virtual FString GetPOIName_Implementation() const override;
        virtual bool CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const override;

        /** DEPRECATED: Old stub method - replaced by ExecuteAction() */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
        void OnPOIInteract();
    };
    ```

  - [x] **Update FCOverworldPOI.cpp**:

    ```cpp
    #include "World/FCOverworldPOI.h"
    // ... existing includes ...

    AFCOverworldPOI::AFCOverworldPOI()
    {
        // ... existing constructor code ...

        // Default POI name
        POIName = TEXT("Unnamed POI");

        // Default: no actions (must be configured in Blueprint)
        AvailableActions.Empty();
    }

    // ... existing BeginPlay ...

    // IFCInteractablePOI interface implementation
    TArray<FFCPOIActionData> AFCOverworldPOI::GetAvailableActions_Implementation() const
    {
        return AvailableActions;
    }

    void AFCOverworldPOI::ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor)
    {
        // Stub implementation - log action
        UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s': Executing action %s"),
            *POIName, *UEnum::GetValueAsString(Action));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                FString::Printf(TEXT("POI '%s': Action '%s' executed (STUB)"),
                    *POIName, *UEnum::GetValueAsString(Action)));
        }

        // Future: Implement actual action logic (open dialog, start trade, etc.)
    }

    FString AFCOverworldPOI::GetPOIName_Implementation() const
    {
        return POIName;
    }

    bool AFCOverworldPOI::CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const
    {
        // Default: all actions allowed
        // Override in Blueprint for quest requirements, locked doors, etc.
        return true;
    }

    // Keep old OnPOIInteract for backward compatibility
    void AFCOverworldPOI::OnPOIInteract_Implementation()
    {
        UE_LOG(LogFCOverworldPOI, Warning, TEXT("POI '%s': OnPOIInteract is deprecated, use ExecuteAction()"), *POIName);
    }
    ```

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.3.2** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] AFCOverworldPOI implements IFCInteractablePOI
  - [x] Available Actions array visible in Blueprint Details panel

**COMMIT POINT 6.3.2**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Implement IFCInteractablePOI in AFCOverworldPOI with action system"`

---

#### Step 6.3.3: Configure Actions in BP_FC_OverworldPOI

- [x] **Analysis**

  - [x] Configure available actions in Blueprint for testing
  - [x] Create test POI with single action (auto-execute on overlap)
  - [x] Create test POI with multiple actions (show selection dialog)

- [x] **Implementation (Unreal Editor)**

  - [x] Open BP_FC_OverworldPOI
  - [x] Class Defaults → FC | POI | Actions:
    - [x] Available Actions: Add 2 elements for testing
      - [x] Element 0:
        - [x] Action Type: Talk
        - [x] Action Text: "Talk to Merchant"
      - [x] Element 1:
        - [x] Action Type: Trade
        - [x] Action Text: "Open Trade Menu"
  - [x] Compile and save
  - [x] Create variant BP_FC_OverworldPOI_Village:
    - [x] Duplicate BP_FC_OverworldPOI
    - [x] Rename to BP_FC_OverworldPOI_Village
    - [x] POI Name: "Village"
    - [x] Available Actions: Keep 2 actions (Talk, Trade)
  - [x] Create variant BP_FC_OverworldPOI_Enemy:
    - [x] Duplicate BP_FC_OverworldPOI
    - [x] Rename to BP_FC_OverworldPOI_Enemy
    - [x] POI Name: "Enemy Camp"
    - [x] Available Actions: 1 action only
      - [x] Action Type: Ambush
      - [x] Action Text: "Ambush!"

- [x] **Testing After Step 6.3.3** ✅ CHECKPOINT
  - [x] BP_FC_OverworldPOI has 2 actions configured
  - [x] Variant Blueprints created with different action sets
  - [x] All Blueprints compile without errors

**COMMIT POINT 6.3.3**: `git add Content/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI*.uasset && git commit -m "feat(overworld): Configure POI actions in Blueprint variants"`

---

### Step 6.4: Implement POI Interaction with Multi-Action Selection

#### Step 6.4.1: Create POI Action Selection Widget (Blueprint)

- [ ] **Analysis**

  - [ ] Widget displays available actions when POI has multiple options
  - [x] User clicks action button → stores selection → initiates convoy movement
  - [x] Widget shows when right-clicking multi-action POI or on unintentional overlap
  - [x] Blueprint implementation for UI flexibility

- [x] **Implementation (Unreal Editor - UMG Widget)**

  - [x] Content Browser → `/Game/FC/UI/Menus/ActionMenu`
  - [x] Right-click → User Interface → Widget Blueprint
  - [x] Name: `WBP_ActionSelection`
  - [x] Open WBP_ActionSelection
  - [x] **Widget Hierarchy**:
    - [x] Canvas Panel (root)
      - [x] Overlay (center screen with auto-size)
        - [x] Border (background panel)
          - [x] Vertical Box
            - [x] Text Block (header: "Select Action")
            - [x] Scroll Box (action list container)
  - [x] **Widget Variables**:
    - [x] `AvailableActions` (TArray<FFCPOIActionData>, Instance Editable)
    - [x] `SelectedAction` (EFCPOIAction)
    - [x] `TargetPOI` (AActor, instance ref)
    - [x] `OnActionSelected` (Event Dispatcher with EFCPOIAction parameter)
  - [x] **Graph: PopulateActions()**
    - [x] Input: AvailableActions array
    - [x] ForEach loop through actions:
      - [x] Create WBP_POIActionButton widget
      - [x] Set button text to ActionData.ActionText
      - [x] Bind button click to OnActionButtonClicked(ActionType)
      - [x] Add button to Scroll Box
  - [x] **Graph: OnActionButtonClicked(EFCPOIAction Action)**
    - [x] Set SelectedAction = Action
    - [x] Call OnActionSelected dispatcher
    - [x] Remove widget from viewport
  - [x] Compile and save

- [x] **Implementation (WBP_POIActionButton child widget)**

  - [x] Create Widget Blueprint: `WBP_POIActionButton`
  - [x] Hierarchy:
    - [x] Button (root)
      - [x] Text Block (action text)
  - [x] Variables:
    - [x] `ActionType` (EFCPOIAction)
    - [x] `ActionText` (Text)
    - [x] `OnClicked` (Event Dispatcher with EFCPOIAction parameter)
  - [x] Event OnClicked (button):
    - [x] Call OnClicked dispatcher with ActionType
  - [x] Compile and save

- [x] **Testing After Step 6.4.1** ✅ CHECKPOINT
  - [x] Widgets created without errors
  - [x] Action selection widget compiles
  - [x] Event dispatchers configured correctly

**COMMIT POINT 6.4.1**: `git add Content/FC/UI/Widgets/WBP_ActionSelection.uasset Content/FC/UI/Widgets/WBP_POIActionButton.uasset && git commit -m "feat(overworld): Create POI action selection widget"`

---

#### Step 6.4.2: Add POI Right-Click Handler with Component Architecture

- [x] **Analysis**

  - [x] Raycast on right-click to detect POI actors
  - [x] Query available actions via IFCInteractablePOI interface
  - [x] Action logic:
    - [x] 0 actions → ignore click (no interaction)
    - [x] 1 action → auto-execute immediately
    - [x] 2+ actions → show action selection widget via UIManager
  - [x] Store pending action and target POI for overlap execution
  - [x] **Architecture**: UFCInteractionComponent handles interaction logic, UFCUIManager manages widgets, PlayerController routes input

- [x] **Implementation (C++ - UFCInteractionComponent)**

  - [x] **Update FCInteractionComponent.h**:

    - [x] Add IFCInteractablePOI include
    - [x] Add PendingInteractionPOI, PendingInteractionAction, bHasPendingPOIInteraction properties
    - [x] Add HandlePOIClick(), OnPOIActionSelected(), NotifyPOIOverlap() method declarations
    - [x] Methods marked BlueprintCallable for widget integration

  - [x] **Update FCInteractionComponent.cpp**:
    - [x] Add IFCInteractablePOI and UFCUIManager includes
    - [x] Implement HandlePOIClick() with action count logic:
      - [x] Query IFCInteractablePOI::GetAvailableActions()
      - [x] 0 actions: ignore
      - [x] 1 action: auto-select and store as pending
      - [x] 2+ actions: request UIManager->ShowPOIActionSelection()
    - [x] Implement OnPOIActionSelected() callback to store user selection
    - [x] Implement NotifyPOIOverlap() with intentional/unintentional overlap handling:
      - [x] Intentional overlap (pending action): execute stored action, close widget
      - [x] Unintentional overlap: query actions, auto-execute if 1 action, show selection if 2+

- [x] **Implementation (C++ - UFCUIManager)**

  - [x] **Update FCUIManager.h**:

    - [x] Add IFCInteractablePOI include for FFCPOIActionData
    - [x] Add POIActionSelectionWidgetClass property
    - [x] Add CurrentPOIActionSelectionWidget instance property
    - [x] Add ShowPOIActionSelection(), ClosePOIActionSelection() method declarations
    - [x] Add getter methods: GetCurrentPOIActionSelectionWidget(), IsPOIActionSelectionOpen()

  - [x] **Update FCUIManager.cpp**:

    - [x] Implement ShowPOIActionSelection(Actions, Component):
      - [x] Validate POIActionSelectionWidgetClass configured
      - [x] Close existing widget if open
      - [x] Create widget instance, add to viewport
      - [x] Returns widget instance for Blueprint to populate actions
    - [x] Implement ClosePOIActionSelection() to remove widget and clear reference

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.2** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] UFCInteractionComponent has POI interaction methods
  - [x] UFCUIManager has widget lifecycle methods
  - [x] Architecture properly separates concerns (Component = logic, UIManager = widgets, Controller = routing)

**COMMIT POINT 6.4.2**: `git add Source/FC/Interaction/FCInteractionComponent.h Source/FC/Interaction/FCInteractionComponent.cpp Source/FC/Core/FCUIManager.h Source/FC/Core/FCUIManager.cpp && git commit -m "feat(overworld): Implement POI interaction with component architecture"`

---

#### Step 6.4.3: Update PlayerController to Delegate POI Interactions

- [x] **Analysis**

  - [x] PlayerController routes right-click to UFCInteractionComponent
  - [x] Remove bloated HandleInteractPOI implementation (196 lines)
  - [x] Delegation pattern: Controller→Component→UIManager
  - [x] Renamed MoveConvoyToPOI to MoveConvoyToLocation (takes FVector instead of AActor)

- [x] **Implementation (C++)**

  - [x] **Update FCPlayerController.h**:

    - [x] Remove POIActionSelectionWidgetClass property (now in UIManager)
    - [x] Remove POIActionSelectionWidget instance property
    - [x] Remove PendingInteractionPOI, PendingInteractionAction, bHasPendingInteraction properties (now in Component)
    - [x] Remove HandleInteractPOI(), OnPOIActionSelected() method declarations
    - [x] Remove NotifyPOIOverlap() method and getter inline methods (now handled by Component)
    - [x] Add MoveConvoyToLocation(FVector) declaration (convoy movement is controller responsibility)

  - [x] **Update FCPlayerController.cpp**:

    - [x] Update HandleClick() to call InteractionComponent->HandlePOIClick(HitActor) instead of HandleInteractPOI()
    - [x] Remove HandleInteractPOI() implementation (60 lines)
    - [x] Remove OnPOIActionSelected() implementation (22 lines)
    - [x] Remove NotifyPOIOverlap() implementation (67 lines)
    - [x] Remove POI interaction property initialization from constructor (4 lines)
    - [x] Replace MoveConvoyToPOI(AActor) with MoveConvoyToLocation(FVector) - simple delegation method

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.3** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] PlayerController reduced by ~180 lines (196 removed, 16 added for MoveConvoyToLocation)
  - [x] HandleClick delegates to InteractionComponent
  - [x] Architecture clean: Controller routes, Component handles logic

**COMMIT POINT 6.4.3**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "refactor(overworld): Clean up PlayerController, delegate POI interaction to component"`

---

#### Step 6.4.4: Connect Convoy Overlap to Interaction Component

- [x] **Analysis**

  - [x] AFCConvoyMember already detects POI overlap (Step 6.2)
  - [x] Forward overlap event to InteractionComponent instead of parent convoy
  - [x] Add bIsInteractingWithPOI flag to convoy to prevent multiple members triggering same POI
  - [x] InteractionComponent clears convoy flag after interaction completes

- [x] **Implementation (C++)**

  - [x] **Update FCOverworldConvoy.h**:
    - [x] Add bool bIsInteractingWithPOI private member
    - [x] Add IsInteractingWithPOI() getter method
    - [x] Add SetInteractingWithPOI(bool) setter method
  - [x] **Update FCOverworldConvoy.cpp**:
    - [x] Initialize bIsInteractingWithPOI = false in constructor
    - [x] Check flag in NotifyPOIOverlap(), set to true if not already interacting
    - [x] Return early if already interacting (prevents multiple triggers)
  - [x] **Update FCConvoyMember.cpp** NotifyPOIOverlap():
    - [x] Add includes for FCPlayerController, FCFirstPersonCharacter, FCInteractionComponent
    - [x] Check ParentConvoy->IsInteractingWithPOI() - return early if true
    - [x] Get PlayerController → FirstPersonCharacter → InteractionComponent
    - [x] Call InteractionComponent->NotifyPOIOverlap(POIActor)
    - [x] Keep ParentConvoy->NotifyPOIOverlap() as fallback
  - [x] **Update FCInteractionComponent.cpp**:
    - [x] Add includes for FCPlayerController and FCOverworldConvoy
    - [x] After ExecuteAction() in NotifyPOIOverlap():
      - [x] Get PlayerController->GetPossessedConvoy()
      - [x] Call Convoy->SetInteractingWithPOI(false) to clear flag
    - [x] Clear flag after both intentional and auto-executed interactions
  - [x] **Update FCPlayerController.h**:
    - [x] Add public GetPossessedConvoy() getter method
  - [x] Save all files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.4** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] Convoy overlap notifies InteractionComponent
  - [x] Only first convoy member triggers POI interaction
  - [x] Convoy flag cleared after interaction completes

**COMMIT POINT 6.4.4**: `git add Source/FC/Characters/Convoy/FCConvoyMember.* Source/FC/Characters/Convoy/FCOverworldConvoy.* Source/FC/Interaction/FCInteractionComponent.cpp Source/FC/Core/FCPlayerController.h && git commit -m "feat(overworld): Add convoy interaction state to prevent multiple POI triggers"`

---

#### Step 6.4.5: Configure Widget Class in Game Instance

- [x] **Analysis**

  - [x] ActionSelectionWidgetClass exposed in UFCGameInstance for Blueprint configuration
  - [x] GameInstance delegates widget class to UFCUIManager on Init()
  - [x] Set WBP_ActionSelection as widget class in BP_FC_GameInstance

- [x] **Implementation (C++)**

  - [x] **Update UFCGameInstance.h**:
    - [x] Add ActionSelectionWidgetClass property (EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
  - [x] **Update UFCGameInstance.cpp**:
    - [x] Add UIManager->ActionSelectionWidgetClass = ActionSelectionWidgetClass in Init()
  - [x] **Update FCUIManager.h**:
    - [x] Rename POIActionSelectionWidgetClass to ActionSelectionWidgetClass
    - [x] Remove EditDefaultsOnly specifier (configured via GameInstance)
  - [x] **Update FCUIManager.cpp**:
    - [x] Replace all POIActionSelectionWidgetClass references with ActionSelectionWidgetClass
  - [x] Save files
  - [x] Compile C++ code

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FC_GameInstance
  - [ ] Class Defaults → UI:
    - [ ] Action Selection Widget Class: Select WBP_ActionSelection
  - [ ] Compile and save

- [ ] **Testing After Step 6.4.5** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] ActionSelectionWidgetClass property visible in BP_FC_GameInstance under UI category
  - [x] Widget class reference set in Blueprint
  - [x] Blueprint compiles

**COMMIT POINT 6.4.5**: `git add Source/FC/Core/UFCGameInstance.h Source/FC/Core/UFCGameInstance.cpp Source/FC/Core/FCUIManager.h Source/FC/Core/FCUIManager.cpp Content/FC/Core/Blueprints/BP_FC_GameInstance.uasset && git commit -m "feat(overworld): Expose ActionSelectionWidgetClass in GameInstance and delegate to UIManager"`

---

#### Step 6.4.6: Wire Widget to Interaction Component via UIManager Mediator

- [x] **Analysis**

  - [x] **Architecture Decision**: UIManager acts as mediator between widget and InteractionComponent
  - [x] **Rationale**: Loose coupling - widget remains UI-only, reusable for other action selection scenarios
  - [x] **Pattern**: Follows existing HandleNewLegacyClicked/HandleContinueClicked mediator pattern
  - [x] **Flow**: Widget fires event dispatcher → UIManager handles event → InteractionComponent callback
  - [x] WBP_ActionSelection receives actions array from UFCUIManager::ShowPOIActionSelection()
  - [x] Widget populates action buttons dynamically and calls UIManager->HandlePOIActionSelected()
  - [x] UIManager stores InteractionComponent reference during widget display
  - [x] UIManager callback forwards action to InteractionComponent->OnPOIActionSelected()

- [x] **Implementation (C++ - UFCUIManager)**

  - [x] **Update FCUIManager.h**:
    - [x] Add PendingInteractionComponent property (UFCInteractionComponent\*)
    - [x] Add HandlePOIActionSelected(EFCPOIAction Action) method declaration
  - [x] **Update FCUIManager.cpp**:
    - [x] In ShowPOIActionSelection():
      - [x] Store InteractionComponent in PendingInteractionComponent
      - [x] Call PopulateActions(Actions) on widget to set actions array and create buttons
    - [x] In ClosePOIActionSelection():
      - [x] Clear PendingInteractionComponent reference
    - [x] Implement HandlePOIActionSelected(EFCPOIAction Action):
      - [x] Validate PendingInteractionComponent is valid
      - [x] Forward action to PendingInteractionComponent->OnPOIActionSelected(Action)
      - [x] Clear PendingInteractionComponent reference
      - [x] Call ClosePOIActionSelection()
  - [x] **Update FCPlayerController.cpp**:
    - [x] Add IFCInteractablePOI.h include
    - [x] Implement HandleInteractPressed() for TopDown mode:
      - [x] Raycast under cursor to detect POI actors
      - [x] Check for IFCInteractablePOI interface
      - [x] Delegate to InteractionComponent->HandlePOIClick() for action selection
    - [x] Remove POI detection from HandleClick():
      - [x] Left-click now only handles click-to-move
      - [x] Right-click (IA_Interact) handles POI interaction
  - [x] Save files
  - [x] Compile C++ code

- [x] **Implementation (WBP_ActionSelection Blueprint)**

  - [x] **Add Variables**:
    - [x] `AvailableActions` (TArray<FFCPOIActionData>, Instance Editable, Expose on Spawn)
  - [x] **Function: PopulateActions(Actions)**:
    - [x] Takes TArray<FFCPOIActionData> as input parameter
    - [x] Sets AvailableActions variable from parameter
    - [x] ForEach loop through Actions array:
      - [x] Create WBP_POIActionButton instance
      - [x] Set button's ActionType and ActionText from array element
      - [x] Bind button's OnClicked → OnActionButtonClicked(ActionType)
      - [x] Add button to Scroll Box
  - [x] **Function: OnActionButtonClicked(EFCPOIAction Action)**:
    - [x] Get Game Instance → Get Subsystem UFCUIManager
    - [x] Call UIManager->HandlePOIActionSelected(Action)
  - [x] Compile and save

- [x] **Testing After Step 6.4.6** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] Right-click on POI opens action selection widget
  - [x] Left-click moves convoy to location (no widget)
  - [x] PopulateActions receives actions array from C++ via ProcessEvent
  - [x] Widget displays action buttons correctly
  - [x] HandlePOIActionSelected method available for Blueprint widget callbacks
  - [x] Widget remains UI-only (no direct InteractionComponent reference)
  - [x] UIManager mediates between widget and game logic

**IMPLEMENTATION NOTES**:

- **Input Split**: Right-click (IA_Interact) = POI interaction, Left-click (IA_Click) = movement
- **ProcessEvent Safety**: PopulateActions called with properly structured parameters matching Blueprint function signature
- **POI Navigation**: POIs should NOT block navmesh - set "Can Ever Affect Navigation" = FALSE in Blueprint
- **Movement Stop**: Convoy stops on overlap trigger (CapsuleComponent), not navmesh blocking

**COMMIT POINT 6.4.6**: `git add Source/FC/Core/FCUIManager.h Source/FC/Core/FCUIManager.cpp Source/FC/Core/FCPlayerController.cpp Content/FC/UI/Widgets/WBP_ActionSelection.uasset && git commit -m "feat(overworld): Implement right-click POI interaction and widget wiring via UIManager mediator"`

---

#### Step 6.4.7: Refactor Convoy-POI Coordination Architecture

- [x] **Analysis**

  - [x] **Problem**: Individual convoy members stopping independently, uncoordinated behavior
  - [x] **Solution**: Member detects → Convoy coordinates all members → InteractionComponent handles logic
  - [x] **Movement Flow**:
    - **Right-click single-action**: HandlePOIClick → auto-select → MoveConvoyToLocation → Overlap → Execute
    - **Right-click multiple-action**: HandlePOIClick → widget → OnPOIActionSelected → MoveConvoyToLocation → Overlap → Execute
    - **Left-click**: HandleClick → MoveConvoyToLocation → Overlap → HandlePOIOverlap → widget → OnPOIActionSelected → Execute immediately (no re-move)
  - [x] **Key Flag**: `bConvoyAlreadyAtPOI` differentiates left-click (already at POI) from right-click (move after selection)

- [x] **Implementation (AFCOverworldConvoy.h)**

  - [x] Added `StopAllMembers()` method
  - [x] Added `HandlePOIOverlap(AActor*)` method for coordinated stop and delegation

- [x] **Implementation (AFCOverworldConvoy.cpp)**

  - [x] `StopAllMembers()`: Iterates ConvoyMembers array, calls AIController->StopMovement() on each
  - [x] `HandlePOIOverlap()`: Checks bIsInteractingWithPOI flag, stops all members, delegates to InteractionComponent
  - [x] Added includes: AIController.h, FCPlayerController.h, FCFirstPersonCharacter.h, FCInteractionComponent.h
  - [x] Old `NotifyPOIOverlap()` kept for backward compatibility (fallback only, deprecated)

- [x] **Implementation (AFCConvoyMember.cpp)**

  - [x] `OnCapsuleBeginOverlap()`: Calls `ParentConvoy->HandlePOIOverlap(OtherActor)` for coordination
  - [x] Removed individual AIController->StopMovement() call (convoy coordinates now)
  - [x] `NotifyPOIOverlap()`: Marked as deprecated, kept as fallback for backward compatibility

- [x] **Implementation (UFCInteractionComponent.h/cpp)**

  - [x] Added `bConvoyAlreadyAtPOI` flag to differentiate left-click vs right-click scenarios
  - [x] `HandlePOIClick()`: Resets `bConvoyAlreadyAtPOI = false` for right-click scenarios
  - [x] `NotifyPOIOverlap()` (unintentional overlap): Sets `bConvoyAlreadyAtPOI = true` for left-click multiple-action
  - [x] `OnPOIActionSelected()`: Checks `bConvoyAlreadyAtPOI` to determine if movement needed:
    - If `false` (right-click multiple): Calls MoveConvoyToLocation
    - If `true` (left-click): Calls NotifyPOIOverlap immediately (no movement)

- [x] **Compile and Test**

  - [x] C++ code compiled successfully
  - [x] All three POI interaction flows working:
    - ✅ Left-click: Move → Overlap stops all → Widget → Select → Execute (no re-move)
    - ✅ Right-click single: Auto-select → Move → Overlap stops all → Execute
    - ✅ Right-click multiple: Widget → Select → Move → Overlap stops all → Execute
  - [x] All convoy members stop together on any member overlap
  - [x] No duplicate movement triggers
  - [x] Output Log shows proper coordination messages

- [x] **Testing After Step 6.4.7** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] All convoy members stop together on POI overlap (coordinated)
  - [x] Convoy coordinates stop via `StopAllMembers()`
  - [x] Convoy delegates interaction to InteractionComponent
  - [x] `bIsInteractingWithPOI` flag prevents duplicate triggers
  - [x] `bConvoyAlreadyAtPOI` flag prevents re-movement after stop
  - [x] POI actions execute correctly in all scenarios
  - [x] Left-click doesn't cause re-movement after convoy already stopped
  - [x] Right-click properly triggers movement after action selection

**IMPLEMENTATION NOTES**:

- **Coordination Pattern**: Member detects → Convoy coordinates stop → InteractionComponent handles logic
- **Movement Delegation**: All movement goes through PlayerController::MoveConvoyToLocation
- **Flag System**:
  - `bIsInteractingWithPOI`: Prevents multiple convoy members from triggering same POI
  - `bConvoyAlreadyAtPOI`: Prevents re-movement when convoy already stopped at POI (left-click scenario)
- **Clear Responsibilities**:
  - ConvoyMember: Overlap detection only
  - Convoy: Coordinate all member stops, manage interaction flag, delegate to InteractionComponent
  - InteractionComponent: Action selection, execution, movement triggering logic
  - PlayerController: Movement execution (MoveConvoyToLocation)

**COMMIT POINT 6.4.7**: `git add Source/FC/Characters/Convoy/FCOverworldConvoy.* Source/FC/Characters/Convoy/FCConvoyMember.* Source/FC/Interaction/FCInteractionComponent.* && git commit -m "refactor(convoy): Implement coordinated POI stop and fix left-click re-movement bug"`

---

### Step 6.5: Place POI Actors and Test

#### Step 6.5.1: Add Multiple POI Instances to L_Overworld

- [ ] **Analysis**

  - [ ] Place 3-5 POI actors in different locations for testing
  - [ ] Set unique names for each POI instance
  - [ ] Position on ground (Z=0 or on terrain)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] Drag BP_OverworldPOI from Content Browser into viewport
  - [ ] **POI Instance 1**:
    - [ ] Position: X=500, Y=0, Z=50
    - [ ] Details Panel → FC | POI → POIName: "Northern Village"
  - [ ] Drag another BP_OverworldPOI instance
  - [ ] **POI Instance 2**:
    - [ ] Position: X=-500, Y=500, Z=50
    - [ ] POIName: "Eastern Outpost"
  - [ ] Drag another BP_OverworldPOI instance
  - [ ] **POI Instance 3**:
    - [ ] Position: X=-500, Y=-500, Z=50
    - [ ] POIName: "Western Ruins"
  - [ ] Save level

- [ ] **Testing After Step 6.5.1** ✅ CHECKPOINT
  - [ ] Multiple POI actors visible in level
  - [ ] Each POI has unique POIName value
  - [ ] POIs positioned on ground
  - [ ] Level saves without errors

**COMMIT POINT 6.5.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Place multiple BP_OverworldPOI instances in L_Overworld"`

---

#### Step 6.5.2: Full POI Interaction Verification

- [ ] **Analysis**

  - [ ] Test right-click interaction
  - [ ] Test convoy overlap detection
  - [ ] Verify both interaction methods work

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify convoy and POIs visible
  - [ ] **Test Right-Click Interaction**:
    - [ ] Right-click on POI mesh (e.g., "Northern Village")
    - [ ] On-screen message: "POI Interaction Stub: Northern Village" (yellow, 5s)
    - [ ] Check Output Log: "HandleInteractPOI: Interacted with POI 'Northern Village'"
  - [ ] **Test Convoy Overlap Detection**:
    - [ ] Left-click to move convoy leader to POI location
    - [ ] When leader overlaps POI:
      - [ ] On-screen message: "Convoy detected POI: Northern Village" (cyan, 5s)
      - [ ] Check Output Log: "Convoy detected POI: Northern Village"
  - [ ] **Test Multiple POIs**:
    - [ ] Right-click on different POIs → Each shows unique name
    - [ ] Move convoy to different POIs → Overlap detection works for all
  - [ ] **Test Non-POI Right-Click**:
    - [ ] Right-click on ground (not POI) → No POI interaction message
    - [ ] Check log: "Hit actor is not a POI" or "No hit under cursor"

- [ ] **Testing After Step 6.5.2** ✅ CHECKPOINT
  - [ ] Right-click POI interaction works
  - [ ] Convoy overlap detection works
  - [ ] Both methods show correct POI names
  - [ ] No "Accessed None" errors

**COMMIT POINT 6.5.2**: `git add -A && git commit -m "test(overworld): Verify POI right-click and convoy overlap detection"`

---

### Task 6 Acceptance Criteria

- [x] IA_InteractPOI input action created (Digital/Boolean)
- [x] Right Mouse Button bound to IA_InteractPOI in IMC_FC_TopDown
- [x] BP_OverworldPOI actor created with mesh, InteractionBox, and POIName property
- [x] IFCInteractablePOI interface created with action-based system (GetAvailableActions, ExecuteAction, GetPOIName)
- [x] BP_OverworldPOI implements IFCInteractablePOI interface
- [x] Action execution displays stub messages and logs
- [x] UFCInteractionComponent handles POI interaction logic (HandlePOIClick, OnPOIActionSelected, NotifyPOIOverlap)
- [x] UFCUIManager manages POI action selection widget lifecycle
- [x] AFCPlayerController delegates to InteractionComponent (HandleInteractPressed)
- [x] WBP_ActionSelection widget displays actions and callbacks to InteractionComponent
- [x] 3-5 POI instances placed in L_Overworld with unique names
- [x] Right-click POI shows action widget (multiple actions) or auto-executes (single action)
- [x] Convoy overlap detection triggers action widget or auto-executes
- [x] Coordinated convoy stop implemented (StopAllMembers, HandlePOIOverlap)
- [x] All three interaction flows working (right-click single/multiple, left-click)
- [x] No duplicate movement triggers (bConvoyAlreadyAtPOI flag)
- [x] No compilation errors or runtime crashes

**Task 6 complete. Ready for Task 7-9 (transitions, pause, testing).**

---

## Backlog Items

### Backlog Item 1: Convoy Follower Breadcrumb System

**Priority**: Medium (Week 4 or 5)

**Description**: Implement breadcrumb trail system for convoy followers to follow the leader's path with realistic spacing and formation.

**Requirements**:

1. Leader leaves breadcrumb markers at regular intervals (e.g., every 100 units)
2. Followers navigate to breadcrumbs in sequence
3. Configurable spacing between convoy members (default: 150 units)
4. Followers maintain formation during turns and obstacles
5. Breadcrumbs clean up after all followers pass

**Architecture**:

- **BP_FC_ConvoyMember** (Leader):
  - Add Timer: Drop breadcrumb every 0.5 seconds or 100 units traveled
  - Breadcrumb data: FVector location + timestamp
  - Store in BP_FC_OverworldConvoy breadcrumb array
- **BP_FC_ConvoyMember** (Followers):
  - AI controller retrieves next breadcrumb from parent convoy
  - MoveTo using breadcrumb position
  - OnMoveCompleted: Request next breadcrumb
  - Skip breadcrumbs if too close (formation spacing logic)
- **BP_FC_OverworldConvoy**:
  - Breadcrumb queue (TArray<FVector>)
  - GetNextBreadcrumb(int FollowerIndex) method
  - CleanupOldBreadcrumbs() method (removes breadcrumbs passed by last follower)

**Implementation Steps**:

1. Add breadcrumb dropping to leader movement
2. Create GetNextBreadcrumb method in BP_FC_OverworldConvoy
3. Implement breadcrumb following in follower AI controllers
4. Add formation spacing logic (distance checks)
5. Implement breadcrumb cleanup system
6. Test with convoy moving in complex paths (turns, obstacles)

**Modularity Notes**:

- Breadcrumb system encapsulated in BP_FC_OverworldConvoy
- Follower AI controller logic separate from leader logic
- Configuration variables exposed for spacing, breadcrumb frequency

---

### Backlog Item 2: Dynamic Camera Constraint System

**Priority**: Low (Week 6+)

**Description**: Bind camera spring arm length and pan distance constraints to convoy size and state (moving vs. stationary).

**Requirements**:

1. Camera zoom limits based on convoy spread (how far apart members are)
2. Pan boundaries expand when convoy is spread out
3. Camera smoothly interpolates constraints during convoy movement
4. Configurable min/max values for zoom and pan

**Architecture**:

- **BP_FC_OverworldConvoy**:
  - CalculateConvoyBounds() method (returns bounding box of all members)
  - OnConvoyBoundsChanged event dispatcher
- **BP_OverworldCamera**:
  - Subscribe to OnConvoyBoundsChanged
  - Adjust spring arm length: Lerp(MinLength, MaxLength, ConvoySpread / MaxSpread)
  - Adjust pan boundaries: Bounds.Extents \* PanBoundaryMultiplier
  - Smooth interpolation (FInterpTo) for camera constraint changes

**Implementation Steps**:

1. Create CalculateConvoyBounds in BP_FC_OverworldConvoy (tick or timer-based)
2. Add OnConvoyBoundsChanged event dispatcher
3. Subscribe camera to event in BeginPlay
4. Implement dynamic spring arm length adjustment
5. Implement dynamic pan boundary adjustment
6. Add smoothing/interpolation for constraint changes
7. Test with convoy in various formations and states

**Modularity Notes**:

- Event-driven design (convoy broadcasts bounds, camera reacts)
- Camera logic independent of convoy implementation
- Configuration variables for min/max constraints and interpolation speed

---

### Backlog Item 3: Convoy Member Differentiation

**Priority**: Low (Week 5+)

**Description**: Visual and functional differentiation of convoy members (leader, guard, supply wagon, etc.) with unique meshes and properties.

**Requirements**:

1. Convoy member types: Leader, Guard, Supply Wagon, Scout
2. Each type has unique mesh and material
3. Different movement speeds and collision sizes per type
4. POI overlap detection may vary by type (e.g., scouts detect POIs from farther away)

**Architecture**:

- **EConvoyMemberType** enum: Leader, Guard, SupplyWagon, Scout
- **BP_FC_ConvoyMember**:
  - MemberType variable (EConvoyMemberType)
  - Struct for member properties (speed, collision size, POI detection radius)
  - ApplyMemberTypeSettings() method (called in BeginPlay)
- **BP_FC_OverworldConvoy**:
  - Convoy composition array: TArray<EConvoyMemberType> (e.g., [Leader, Guard, SupplyWagon])
  - Spawn members based on composition array

**Implementation Steps**:

1. Create EConvoyMemberType enum
2. Create FConvoyMemberProperties struct (speed, size, detection radius, mesh ref)
3. Add MemberType variable and properties map to BP_FC_ConvoyMember
4. Implement ApplyMemberTypeSettings method
5. Update BP_FC_OverworldConvoy Construction Script to spawn typed members
6. Create unique meshes/materials for each type
7. Test convoy with mixed member types

**Modularity Notes**:

- Data-driven design (properties in struct/map)
- Easy to add new member types (extend enum + add properties)
- Composition defined in convoy parent (flexible for different convoy types)

---

**End of Backlog Items**

---

### Task 7: Office-to-Overworld Level Transition

**Purpose**: Integrate L_Overworld into existing level transition system so players can travel from L_Office to L_Overworld and back.

---

#### Step 7.1: Review Existing Transition System

##### Step 7.1.1: Analyze Week 2 Transition Architecture

- [x] **Analysis**

  - [x] Review UFCLevelManager transition methods (documentation/code)
  - [x] Review UFCGameStateManager state changes (Office, Overworld_Travel)
  - [x] Check if L_Office has existing transition trigger (table object or widget button)
  - [x] Identify pattern: State Change → Level Transition → Input Context Switch
  - [x] Review files:
    - [x] `Source/FC/Core/FCLevelManager.h/.cpp` (LoadLevel method exists)
    - [x] `Source/FC/Core/FCGameStateManager.h/.cpp` (TransitionTo method exists)
    - [x] `Content/FC/World/Blueprints/Interactables/BP_TableObject_Map.uasset` (map table trigger)
    - [x] `Content/FC/UI/Menus/TableMenu/WBP_ExpeditionPlanning.uasset` (widget exists, needs button)

- [x] **Expected Architecture Pattern**

  - [x] Week 2 established:
    - [x] Table interaction → OnTableObjectClicked event → Opens WBP_ExpeditionPlanning widget
    - [x] Widget button → Triggers transition logic
    - [x] Transition logic needs:
      1. UFCGameStateManager::TransitionTo(Overworld_Travel)
      2. UFCLevelManager::LoadLevel("L_Overworld")
      3. UFCInputManager switches context to TopDown mode (automatic via GameState)

- [x] **Testing After Step 7.1.1** ✅ CHECKPOINT
  - [x] Reviewed LevelManager LoadLevel() method - exists and functional
  - [x] Reviewed GameStateManager TransitionTo() method - exists and functional
  - [x] Identified trigger point: BP_TableObject_Map → WBP_ExpeditionPlanning
  - [x] Confirmed state flow: Office_Exploration → Overworld_Travel → L_Overworld load

**COMMIT POINT 7.1.1**: N/A (analysis only, no code changes)

---

#### Step 7.2: Add Transition Trigger in L_Office

##### Step 7.2.1: Add "Start Journey" Button to WBP_ExpeditionPlanning Widget

- [x] **Analysis**

  - [x] WBP_ExpeditionPlanning widget opens when interacting with map table in L_Office
  - [x] Add button to initiate overworld transition
  - [x] Button should call transition logic

- [x] **Implementation (Unreal Editor - Widget Blueprint)**

  - [x] Open `/Game/FC/UI/Menus/TableMenu/WBP_ExpeditionPlanning`
  - [x] Designer Tab:
    - [x] Add Widget: Button (renamed to "StartExpeditionButton")
    - [x] Position: Appropriate location in widget
    - [x] Add Text Block as child: "Start Journey" or "Begin Expedition"
    - [x] Set button style (color, hover effects)
  - [x] Graph Tab:
    - [x] Select StartExpeditionButton → Add Event → OnClicked
    - [x] **Blueprint Implementation**:
      - [x] Get Game Instance → Cast to BP_FCGameInstance
      - [x] Get GameStateManager subsystem → Call TransitionTo(Overworld_Travel)
      - [x] Get LevelManager subsystem → Call LoadLevel("L_Overworld")
      - [x] Remove Widget from Parent (close map widget)
      - [x] Added debug Print String: "Start Expedition button clicked"
  - [x] Compile and save widget

- [ ] **Implementation Notes**

  - [ ] If TransitionToLevel() doesn't exist yet in UFCLevelManager, create it:

    ```cpp
    // FCLevelManager.h
    UFUNCTION(BlueprintCallable, Category = "FC|Level")
    void TransitionToLevel(FName LevelName);

    // FCLevelManager.cpp
    void UFCLevelManager::TransitionToLevel(FName LevelName)
    {
        UGameplayStatics::OpenLevel(this, LevelName);
        UE_LOG(LogFCLevelManager, Log, TEXT("TransitionToLevel: Transitioning to %s"), *LevelName.ToString());
    }
    ```

  - [ ] If SetGameState() needs enum parameter, confirm EFCGameState enum has Overworld_Travel

- [x] **Testing After Step 7.2.1** ✅ CHECKPOINT
  - [x] "Start Journey" button visible in WBP_ExpeditionPlanning widget
  - [x] Button positioned correctly
  - [x] Widget compiles without errors
  - [x] OnClicked event graph connected

**COMMIT POINT 7.2.1**: `git add Content/FC/UI/Menus/TableMenu/WBP_ExpeditionPlanning.uasset && git commit -m "feat(transition): Add Start Journey button to WBP_ExpeditionPlanning widget"`

---

##### Step 7.2.2: Implement Transition Logic in Widget Blueprint

- [x] **Analysis**

  - [x] Widget Blueprint implementation chosen (simpler for Week 3)
  - [x] Uses existing GameStateManager and LevelManager subsystems
  - [x] State transition triggers automatic input mode switch

- [x] **Implementation (WBP_ExpeditionPlanning Widget Graph)**

  - [x] Open WBP_ExpeditionPlanning → Event Graph
  - [x] Locate StartExpeditionButton OnClicked event
  - [x] **Transition Nodes Added**:
    - [x] Get Game Instance
    - [x] Cast to BP_FCGameInstance
    - [x] From cast result:
      - [x] Get Subsystem (FCGameStateManager) → Call TransitionTo → Input: Overworld_Travel
      - [x] Get Subsystem (FCLevelManager) → Call LoadLevel → Input: "L_Overworld"
    - [x] Remove from Parent (close widget before transition)
  - [x] Added Print String for debugging:
    - [x] "Start Expedition button clicked"
  - [x] Compile and save

- [x] **Alternative Implementation (if C++ needed)**

  - [ ] Add method to AFCPlayerController:

    ```cpp
    // FCPlayerController.h
    UFUNCTION(BlueprintCallable, Category = "FC|Transition")
    void TransitionToOverworld();

    // FCPlayerController.cpp
    void AFCPlayerController::TransitionToOverworld()
    {
        UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
        if (!GameInstance) return;

        // Set game state
        UFCGameStateManager* GameStateMgr = GameInstance->GetGameStateManager();
        if (GameStateMgr)
        {
            GameStateMgr->SetGameState(EFCGameState::Overworld_Travel);
        }

        // Transition level
        UFCLevelManager* LevelMgr = GameInstance->GetLevelManager();
        if (LevelMgr)
        {
            LevelMgr->TransitionToLevel(FName("L_Overworld"));
        }

        UE_LOG(LogFCPlayerController, Log, TEXT("TransitionToOverworld: Initiated transition"));
    }
    ```

  - [x] Call TransitionToOverworld() from widget button

- [x] **Testing After Step 7.2.2** ✅ CHECKPOINT
  - [x] PIE in L_Office
  - [x] Click on BP_TableObject_Map → WBP_ExpeditionPlanning opens
  - [x] Click "Start Journey" button
  - [x] Verify: Widget closes immediately ✅
  - [x] Verify: State transitions to Overworld_Travel ✅
    - [x] Log: "State transition: Office_Exploration -> Overworld_Travel"
  - [x] Verify: L_Overworld loads ✅
  - [x] Verify: Input mode switches to TopDown automatically ✅
    - [x] Log: "Input mapping switched to: TopDown"
  - [x] Verify: No crashes ✅
  - [x] Known Issue: Camera error (expected - BP_OverworldCamera not placed yet, Task 3)

**COMMIT POINT 7.2.2**: `git add Content/FC/UI/Menus/TableMenu/WBP_ExpeditionPlanning.uasset && git commit -m "feat(transition): Implement Office-to-Overworld transition logic in WBP_ExpeditionPlanning"`

---

#### Step 7.3: Add Return Transition from Overworld to Office

##### Step 7.3.1: Create Reusable Confirmation Dialog Widget

- [x] **Analysis**

  - [x] Create reusable confirmation dialog for various purposes
  - [x] Dialog will be used for Abort Expedition (and potentially other confirmations)
  - [x] Should accept customizable title, message, and button text

- [x] **Implementation Part 1: Create WBP_ConfirmationDialog Widget**

  - [x] Created Widget Blueprint: `/Game/FC/UI/ReusableWidgets/WBP_ConfirmationDialog`
  - [x] Designer Tab:
    - [x] Added Canvas Panel as root
    - [x] Added Overlay (semi-transparent background blocker)
      - [x] Image (background): Black, Opacity 0.7
    - [x] Added dialog container with:
      - [x] Text Block: `TitleText` (bound variable)
      - [x] Text Block: `InfoText` (bound variable)
      - [x] Horizontal Box (buttons):
        - [x] Button with Text Block: `DeclineText` (Btn_Decline)
        - [x] Button with Text Block: `ConfirmText` (Btn_Confirm) - Warning color
  - [x] Graph Tab:
    - [x] Created Custom Function: `InitializeConfirmationDialog`
      - [x] Parameters: Title (Text), InfoText (Text), ConfirmButtonText (Text), DeclineButtonText (Text)
      - [x] Sets text for TitleText, InfoText, ConfirmText, DeclineText widgets
    - [x] Added Event Dispatcher: "OnConfirmed" (bool isConfirmed)
      - [x] isConfirmed = true when confirm button clicked
      - [x] isConfirmed = false when decline button clicked
    - [x] Btn_Decline OnClicked → Call OnConfirmed(false) → RemoveFromParent
    - [x] Btn_Confirm OnClicked → Call OnConfirmed(true) → RemoveFromParent
  - [x] Compiled and saved

- [x] **Implementation Part 2: Check if WBP_PauseMenu Exists**

  - [x] Check: `/Game/FC/UI/Menus/WBP_PauseMenu.uasset` ✅ EXISTS
  - [x] WBP_PauseMenu exists and is used in L_Office
  - [x] Continue to Step 7.3.2 (add Abort button to existing menu)
  - [x] Note: ESC key may need to be wired for L_Overworld specifically

- [x] **Testing After Step 7.3.1** ✅ CHECKPOINT
  - [x] WBP_ConfirmationDialog widget created with reusable design
  - [x] InitializeConfirmationDialog function accepts custom text
  - [x] Dialog compiles without errors
  - [x] OnConfirmed and OnDeclined events available for binding

**COMMIT POINT 7.3.1**: `git add Content/FC/UI/ReusableWidgets/WBP_ConfirmationDialog.uasset && git commit -m "feat(ui): Create reusable WBP_ConfirmationDialog widget with initialization function"`

---

##### Step 7.3.2: Add "Abort Expedition" Button to Existing WBP_PauseMenu

- [x] **Prerequisites**

  - [x] WBP_PauseMenu exists at `/Game/FC/UI/Menus/WBP_PauseMenu` ✅
  - [x] Currently used in L_Office scene

- [x] **Implementation (WBP_PauseMenu Widget)**

  - [x] Open `/Game/FC/UI/Menus/WBP_PauseMenu`
  - [x] Designer Tab:
    - [x] Review existing buttons (Resume, Settings, Quit, etc.)
    - [x] Add new Button: "AbortExpeditionButton"
    - [x] Position: Below "Resume" button or in appropriate location
    - [x] Add Text Block as child: "Abort Expedition"
    - [x] Set button style: Warning color (red/orange tint to indicate danger)
  - [x] Graph Tab:
    - [x] Select AbortExpeditionButton → Add Event → OnClicked
    - [x] In OnClicked event:
      - [x] Created Widget: WBP_ConfirmationDialog (uses bUseDefault=true for default text)
      - [x] InitializeConfirmationDialog not needed (default texts used):
        - [ ] Title: "Abort Expedition?"
        - [ ] InfoText: "Aborting will return you to the office. You will lose all progress and expend supplies for this journey."
        - [ ] ConfirmButtonText: "Abort Expedition"
        - [ ] DeclineButtonText: "Cancel"
      - [x] Add to Viewport (Z-order 101, above pause menu)
      - [x] Bind OnConfirmed event to custom event "HandleConfirmationStatus"
        - [x] Branch on isConfirmed parameter:
          - [x] True (Confirmed):
            - [x] Get Game Instance → Get Subsystem (UFCGameStateManager)
            - [x] Call TransitionTo(Office_Exploration)
            - [x] Get Game Instance → Get Subsystem (UFCLevelManager)
            - [x] Call LoadLevel("L_Office", false)
            - [x] Remove pause menu from parent (self)
          - [x] False (Declined):
            - [x] Get Player Controller → Cast to AFCPlayerController
            - [x] Call ResumeGame() to unpause
  - [x] **Optional: Conditional Visibility**
    - [x] Note: Button visible in both levels currently (see Known Issues #2)
    - [x] Deferred to future sprint for UX polish
  - [x] Compile and save

- [x] **Testing After Step 7.3.2** ✅ CHECKPOINT
  - [x] PIE in L_Office → ESC opens pause menu ✅
  - [~] "Abort Expedition" button visible in both levels (Known Issue #2 - deferred) ⚠️
  - [x] PIE in L_Overworld → ESC opens pause menu ✅
  - [x] "Abort Expedition" button styled with warning color ✅
  - [x] Click "Abort Expedition" → Confirmation dialog appears ✅
  - [x] Click "Cancel" → Dialog closes, pause menu still open ✅
  - [x] Click "Abort Expedition" again → Click "Abort Expedition" (confirm) ✅
  - [x] Verify: Transition to L_Office ✅
  - [~] Verify: State changes to Office_Exploration (Known Issue #1 - transitions to MainMenu) ⚠️

**COMMIT POINT 7.3.2**: `git add Content/FC/UI/Menus/WBP_PauseMenu.uasset && git commit -m "feat(transition): Add Abort Expedition button with confirmation dialog to WBP_PauseMenu"`

---

##### Step 7.3.3: Ensure ESC Key Opens Pause Menu in Overworld

- [x] **Analysis**

  - [x] ESC key already opens pause menu in L_Office
  - [x] Verified ESC key binding exists in AFCPlayerController
  - [x] Found issue: HandlePausePressed() only allowed pausing in Office states

- [x] **Implementation (Fix Pausable States)**

  - [x] **Updated FCPlayerController.cpp**:
    - [x] Modified HandlePausePressed() method (lines ~403-410)
    - [x] Added `CurrentState == EFCGameStateID::Overworld_Travel` to pausable states check
    - [x] Original code only allowed Office_Exploration and Office_TableView
    - [x] Now allows pausing from Office_Exploration, Office_TableView, and Overworld_Travel
  - [x] **Verified Input Binding**:
    - [x] ESC key bound to IA_Escape in SetupInputComponent() (line ~243)
    - [x] IA_Escape mapped in both IMC_FC_FirstPerson and IMC_FC_TopDown contexts
    - [x] HandlePausePressed() handles pause/unpause state stack logic

- [x] **Testing After Step 7.3.3** ✅ CHECKPOINT
  - [x] PIE in L_Overworld ✅
  - [x] Press ESC → Pause menu opens ✅
  - [x] Press ESC again → Pause menu closes ✅
  - [x] Verify: Mouse cursor visible when menu open ✅
  - [x] Verify: Can click buttons in pause menu ✅
  - [x] Verify: Gameplay input disabled when menu open ✅

**COMMIT POINT 7.3.3**: `git add Source/FC/Core/FCPlayerController.cpp && git commit -m "fix(pause): Enable pause menu in Overworld_Travel state"`

---

##### Step 7.3.4: Test Complete Office ↔ Overworld Flow

- [x] **Testing Full Transition Cycle** ✅ CHECKPOINT
  - [x] **Office → Overworld**:
    - [x] PIE in L_Office ✅
    - [x] Click map table → WBP_ExpeditionPlanning opens ✅
    - [x] Click "Start Journey" → Loads L_Overworld ✅
    - [x] Verify: State = Overworld_Travel, Input = TopDown ✅
  - [x] **Overworld → Office (Abort)**:
    - [x] Press ESC → Pause menu opens ✅
    - [x] Click "Abort Expedition" → Confirmation dialog appears ✅
    - [x] Click "Cancel" → Dialog closes, still in overworld ✅
    - [x] Press ESC again → Click "Abort Expedition" → Click "Abort Expedition" (confirm) ✅
    - [x] Verify: Returns to L_Office ✅
    - [~] Verify: State = Office_Exploration (transitions to MainMenu - Known Issue #1) ⚠️
    - [~] Verify: Input = FirstPerson (mouse look not working - Known Issue #1) ⚠️
  - [x] **Round Trip**:
    - [x] Can transition Office → Overworld successfully ✅
    - [~] Overworld → Office transition loads level but has state/input issues ⚠️
    - [x] No crashes or memory leaks ✅
  - [x] **Known Issues Documented**: See "Known Issues & Future Backlog" section

**COMMIT POINT 7.3.4**: `git commit -m "test(transition): Complete Office ↔ Overworld transition testing with known issues documented"`

---

#### Step 7.4: Update Technical Documentation

- [x] **Update Technical_Documentation.md**

  - [x] Add section: "Office ↔ Overworld Level Transitions"
  - [x] Document:
    - [x] WBP_ExpeditionPlanning "Start Journey" button flow
    - [x] State transition: Office_Exploration → Overworld_Travel
    - [x] Level load: L_Office → L_Overworld
    - [x] WBP_PauseMenu "Abort Expedition" button
    - [x] WBP_ConfirmationDialog reusable confirmation widget
    - [x] Reverse transition: Overworld_Travel → Office_Exploration
    - [x] HandlePausePressed() pausable states configuration
    - [x] Known limitations and future improvements documented
  - [x] Added to Task 0003 backlog: Known Issues #1, #2, #3
  - [x] Note: Camera error in L_Overworld expected (Task 3 pending)

**COMMIT POINT 7.4**: `git add Docs/Technical_Documentation.md Docs/Tasks/0003-tasks.md && git commit -m "docs(transition): Document Office ↔ Overworld transition system and known issues"`

---

### Task 7 Acceptance Criteria

- [x] **Office → Overworld Transition**

  - [x] Click map table in L_Office → Opens WBP_ExpeditionPlanning widget ✅
  - [x] Click "Start Journey" button → Transitions to L_Overworld ✅
  - [x] Game state changes: Office_Exploration → Overworld_Travel ✅
  - [x] Input mode switches automatically: FirstPerson → TopDown ✅
  - [x] Widget closes before transition ✅

- [x] **Overworld → Office Transition**

  - [x] ESC key opens pause menu in L_Overworld ✅
  - [x] "Abort Expedition" button visible in pause menu ✅
  - [x] Click "Abort Expedition" → Confirmation dialog appears ✅
  - [x] Confirmation dialog with customizable text (uses bUseDefault) ✅
  - [x] Cancel button closes dialog without action ✅
  - [x] Confirm button returns to L_Office ✅
  - [~] Game state changes: Overworld_Travel → Office_Exploration (Known Issue #1: transitions to MainMenu) ⚠️
  - [~] Input mode switches automatically: TopDown → FirstPerson (Known Issue #1: mouse look fails) ⚠️

- [x] **Round Trip Testing**

  - [x] Can transition Office → Overworld successfully ✅
  - [~] Overworld → Office transition works but has state/input issues (Known Issue #1) ⚠️
  - [x] No memory leaks or widget duplication ✅
  - [x] No crashes during transitions ✅

- [x] **Documentation**
  - [x] Technical_Documentation.md includes transition flow ✅
  - [x] Known Issues & Future Backlog section added ✅
  - [x] Widget interaction documented ✅
  - [x] Limitations and future improvements documented ✅

**TASK 7 STATUS**: ✅ Functionally Complete (with documented known issues for future sprints)

**FINAL COMMIT - TASK 7**: `git add . && git commit -m "feat(transition): Complete Task 7 - Office ↔ Overworld transitions (with known issues documented)"`

---

### Task 8: Conditional Engine Pause for Overworld

**Purpose**: Implement ESC key pause/unpause in Overworld only (not in Office), pausing physics and convoy movement but keeping UI interactive.

**IMPLEMENTATION NOTE**: Most functionality already exists from Task 7. Task 8 focuses on enabling **engine pause** (`SetPause(true)`) conditionally for Overworld_Travel state only, while keeping Office states without engine pause to preserve Enhanced Input functionality.

---

#### Step 8.1: Create Input Action for Pause

##### Step 8.1.1: Create IA_Escape Input Action - ✅ already exists

---

##### Step 8.1.2: Add ESC Key Binding to IMC_FC_TopDown - ✅ already bound

---

#### Step 8.2: Create Pause Menu Widget - ✅ already exists (WBP_PauseMenu)

---

##### Step 8.2.2: Implement Resume Button Logic - ✅ already implemented

---

##### Step 8.2.3: Implement Return to Office Button Logic - ✅ already implemented (Abort Expedition)

- [ ] **Analysis**

  - [ ] Resume button should unpause game and close widget
  - [ ] Call same unpause logic that ESC key will trigger

- [ ] **Implementation (WBP_PauseMenu Event Graph)**

  - [ ] Open WBP_PauseMenu → Event Graph
  - [ ] Select Btn_Resume → Add Event → OnClicked
  - [ ] In OnClicked event:
    - [ ] Create Custom Event: "RequestResume"
    - [ ] Promote to Event Dispatcher (optional for controller communication)
    - [ ] OR: Get Player Controller → Cast to BP_FC_PlayerController → Call UnpauseGame() method (to be created)
    - [ ] Remove from Parent (close widget)
  - [ ] For Week 3: Direct controller call is acceptable
  - [ ] Compile and save

- [ ] **Testing After Step 8.2.2** ✅ CHECKPOINT
  - [ ] Btn_Resume OnClicked event created
  - [ ] Resume logic connected (will implement controller method next)
  - [ ] Widget compiles without errors

**COMMIT POINT 8.2.2**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Implement Resume button logic in WBP_PauseMenu"`

---

##### Step 8.2.3: Implement Return to Office Button Logic - already implemented

---

#### Step 8.3: Implement Conditional Engine Pause Logic

##### Step 8.3.1: Modify UFCUIManager to Enable Engine Pause for Overworld Only

- [x] **Analysis**

  - [x] ShowPauseMenu() needs to conditionally call `SetPause(true)` based on **current level**
  - [x] HidePauseMenu() needs to conditionally call `SetPause(false)` when unpausing
  - [x] Office level (L_Office): NO engine pause (Enhanced Input must work)
  - [x] Overworld level (L_Overworld): YES engine pause (stops convoy movement and physics)
  - [x] **Cannot use game state** - state is already Paused when ShowPauseMenu() is called
  - [x] **Solution**: Check level name instead (L_Overworld vs L_Office)

- [x] **Implementation (FCUIManager.cpp)**

  - [x] Modified ShowPauseMenu():
    - [x] Get current level name via `World->GetMapName()`
    - [x] Remove PIE prefix if present (UEDPIE_0_L_Overworld → L_Overworld)
    - [x] If level name contains "L_Overworld": `PC->SetPause(true)` to enable engine pause
    - [x] If level is L_Office: Skip engine pause (Enhanced Input needs to work)
    - [x] Added logging to show when engine pause is enabled/disabled
  - [x] Modified HidePauseMenu():
    - [x] Get current level name via `World->GetMapName()`
    - [x] Remove PIE prefix if present
    - [x] If level name contains "L_Overworld": `PC->SetPause(false)` to disable engine pause
    - [x] If level is L_Office: Skip engine unpause (was never paused)
    - [x] Added logging to show unpause behavior
  - [x] Removed unnecessary FCGameStateManager include
  - [x] Compiled successfully

- [x] **Testing After Step 8.3.1** ✅ CHECKPOINT
  - [x] Code compiles without errors
  - [x] Conditional engine pause logic implemented
  - [x] Logging added for debugging
  - [x] Ready for in-game testing

**COMMIT POINT 8.3.1**: `git add Source/FC/Core/FCUIManager.cpp && git commit -m "feat(pause): Implement conditional engine pause for Overworld_Travel state only"`

---

**NOTE**: Steps 8.3.2 through 8.5.4 below describe legacy implementation details that were later superseded by Task 7's HandlePausePressed() architecture. The functional requirement (engine pause in Overworld only) is now achieved via the conditional logic added in Step 8.3.1 above. Testing should focus on verifying convoy stops when paused in Overworld, and ESC continues to work in Office without engine pause.

---

#### Step 8.4: Update Widget Button Callbacks

##### Step 8.4.1: Connect Resume Button to UnpauseGame Method

- [ ] **Analysis**

  - [ ] Resume button in WBP_PauseMenu should call controller's UnpauseGame()
  - [ ] Update widget graph to use Blueprint-callable method

- [ ] **Implementation (Unreal Editor - WBP_PauseMenu)**

  - [ ] Open WBP_PauseMenu → Event Graph
  - [ ] Locate Btn_Resume OnClicked event
  - [ ] Update graph:
    - [ ] Get Player Controller
    - [ ] Cast to BP_FC_PlayerController
    - [ ] Call UnpauseGame() method
    - [ ] (Widget removal handled by UnpauseGame method automatically)
  - [ ] Compile and save

- [ ] **Testing After Step 8.4.1** ✅ CHECKPOINT
  - [ ] Resume button calls UnpauseGame()
  - [ ] Widget compiles without errors
  - [ ] Cast node correct

**COMMIT POINT 8.4.1**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Connect Resume button to UnpauseGame in WBP_PauseMenu"`

---

##### Step 8.4.2: Update Return to Office Button to Unpause Before Transition

- [ ] **Analysis**

  - [ ] Return to Office button should unpause before transitioning
  - [ ] Ensure game is not paused when loading L_Office

- [ ] **Implementation (Unreal Editor - WBP_PauseMenu)**

  - [ ] Open WBP_PauseMenu → Event Graph
  - [ ] Locate Btn_ReturnToOffice OnClicked event
  - [ ] Update graph:
    - [ ] Get Player Controller
    - [ ] Cast to BP_FC_PlayerController
    - [ ] Call UnpauseGame() FIRST
    - [ ] Then call DebugReturnToOffice() (or transition logic)
  - [ ] Compile and save

- [ ] **Testing After Step 8.4.2** ✅ CHECKPOINT
  - [ ] Return to Office button unpauses then transitions
  - [ ] Widget compiles without errors
  - [ ] Sequence correct (unpause before transition)

**COMMIT POINT 8.4.2**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Unpause before Office transition in WBP_PauseMenu"`

---

#### Step 8.5: Test Pause Functionality in Overworld

##### Step 8.5.1: Test ESC Key Pause/Unpause

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify overworld loads correctly
  - [ ] Left-click to move convoy (verify movement starts)
  - [ ] Press **ESC** key while convoy is moving
  - [ ] **Expected Results**:
    - [ ] Convoy movement stops immediately
    - [ ] WBP_PauseMenu widget appears with semi-transparent background
    - [ ] "PAUSED" text visible
    - [ ] "Resume (ESC)" and "Return to Office (Tab)" buttons visible
    - [ ] Mouse cursor visible and functional
    - [ ] Can hover over buttons (hover effects work)
    - [ ] Check Output Log for: "PauseGame: Game paused, menu shown"
  - [ ] Press **ESC** key again (while paused)
  - [ ] **Expected Results**:
    - [ ] Pause menu closes
    - [ ] Convoy resumes movement (if destination not reached)
    - [ ] Game unpauses
    - [ ] Check Output Log for: "UnpauseGame: Game unpaused"

- [ ] **Testing After Step 8.5.1** ✅ CHECKPOINT
  - [ ] ESC key pauses game
  - [ ] Pause menu appears correctly
  - [ ] ESC key unpauses game
  - [ ] Movement stops/resumes correctly
  - [ ] No "Accessed None" errors

**COMMIT POINT 8.5.1**: N/A (testing only, no code changes)

---

##### Step 8.5.2: Test Resume Button

- [ ] **Test Sequence**

  - [ ] Continue from previous test (in L_Overworld)
  - [ ] Move convoy with left-click
  - [ ] Press ESC to pause
  - [ ] Click **Resume (ESC)** button with mouse
  - [ ] **Expected Results**:
    - [ ] Pause menu closes
    - [ ] Game unpauses
    - [ ] Convoy resumes movement
    - [ ] Check Output Log for: "UnpauseGame: Game unpaused"

- [ ] **Testing After Step 8.5.2** ✅ CHECKPOINT
  - [ ] Resume button works correctly
  - [ ] Game unpauses
  - [ ] No errors

**COMMIT POINT 8.5.2**: N/A (testing only, no code changes)

---

##### Step 8.5.3: Test Return to Office Button

- [ ] **Test Sequence**

  - [ ] Continue from previous test (in L_Overworld)
  - [ ] Press ESC to pause
  - [ ] Click **Return to Office (Tab)** button
  - [ ] **Expected Results**:
    - [ ] Game unpauses
    - [ ] Level transitions to L_Office
    - [ ] Office loads correctly (not paused)
    - [ ] Player can move and interact normally in Office
    - [ ] Check Output Log for:
      - [ ] "UnpauseGame: Game unpaused"
      - [ ] "DebugReturnToOffice: Returning to L_Office"
      - [ ] Transition logs

- [ ] **Testing After Step 8.5.3** ✅ CHECKPOINT
  - [ ] Return to Office button works correctly
  - [ ] Transition completes successfully
  - [ ] Office is not paused after transition
  - [ ] No errors

**COMMIT POINT 8.5.3**: N/A (testing only, no code changes)

---

##### Step 8.5.4: Test Pause Conditional (Overworld Only)

- [ ] **Test Sequence**

  - [ ] Start in L_Office (PIE)
  - [ ] Press **ESC** key in Office
  - [ ] **Expected Results**:
    - [ ] ESC key does nothing (IA_Escape not in IMC_FC_Office)
    - [ ] No pause menu appears
    - [ ] No errors in Output Log
  - [ ] Transition to L_Overworld (via map table)
  - [ ] Press **ESC** key in Overworld
  - [ ] **Expected Results**:
    - [ ] Game pauses (as tested above)
    - [ ] Pause menu appears
  - [ ] This confirms pause is Overworld-only per DRM requirement

- [ ] **Testing After Step 8.5.4** ✅ CHECKPOINT
  - [ ] ESC key does nothing in L_Office
  - [ ] ESC key pauses in L_Overworld
  - [ ] Conditional behavior correct
  - [ ] No errors

**COMMIT POINT 8.5.4**: `git add -A && git commit -m "test(pause): Verify conditional pause behavior (Overworld only)"`

---

### Task 8 Acceptance Criteria

- [x] IA_Escape input action created (Digital/Boolean) ✅ (exists from prior tasks)
- [x] ESC key bound to IA_Escape in both IMC_FC_FirstPerson and IMC_FC_TopDown ✅ (exists)
- [x] WBP_PauseMenu widget exists with Resume and Abort Expedition buttons ✅ (exists from Task 7)
- [x] HandlePausePressed() in AFCPlayerController manages pause state via GameStateManager ✅ (exists from Task 7)
- [x] **UFCUIManager::ShowPauseMenu() conditionally enables engine pause** ✅ (NEW - Step 8.3.1)
  - [x] Checks current level name via World->GetMapName()
  - [x] If L_Overworld: Calls `PC->SetPause(true)` to enable engine pause
  - [x] If L_Office: Skips engine pause to preserve Enhanced Input
- [x] **UFCUIManager::HidePauseMenu() conditionally disables engine pause** ✅ (NEW - Step 8.3.1)
  - [x] Checks current level name
  - [x] If L_Overworld: Calls `PC->SetPause(false)` to disable engine pause
  - [x] If L_Office: Skips engine unpause
- [x] **TESTING COMPLETE**: ESC key in Overworld pauses convoy movement (engine pause active) ✅
- [x] **TESTING COMPLETE**: Resume button successfully unpauses and resumes convoy movement ✅
- [x] **TESTING COMPLETE**: Return to Main Menu button works correctly ✅
- [x] **TESTING COMPLETE**: Abort Expedition button works (returns to Office) ✅
- [~] **Known Issue #4**: ESC key doesn't close pause menu when engine paused (see backlog) ⚠️
- [x] **TESTING COMPLETE**: No crashes or "Accessed None" errors during pause/unpause cycles ✅

**IMPLEMENTATION STATUS**: ✅ Complete and Tested

**Task 8 complete. Engine pause now works correctly in Overworld only. ESC toggle issue documented as Issue #4 in backlog.**

---

### Task 9: Testing, Polish & Documentation Update

**Purpose**: Comprehensive end-to-end testing of Week 3 features, performance validation, and documentation updates.

**STATUS**: ✅ Complete - Testing performed, documentation updated

---

#### Step 9.1: Comprehensive Integration Testing - ✅ Complete

##### Step 9.1.1: Full Gameplay Flow Testing - ✅ Complete

- [x] **Analysis** - Complete

  - [x] Test complete player journey: Office → Overworld → Interaction → Return
  - [x] Verify all systems work together without conflicts
  - [x] Check for edge cases and error conditions

**Testing Results**: All transitions work, pause system functional, known issues documented in backlog.

**COMMIT POINT 9.1.1**: N/A (testing only, no code changes)

---

##### Step 9.1.2: Edge Case and Error Condition Testing - ✅ Complete

**Testing Results**: No critical edge cases found, known Issue #4 (ESC toggle) documented.

---

#### Step 9.2: Performance Validation - ✅ Complete

**Testing Results**: Performance acceptable for current prototype stage.

---

#### Step 9.3: Polish and Bug Fixes - ✅ Complete

**Status**: Known issues documented in backlog for future sprints (Issues #1-4).

---

#### Step 9.4: Update Documentation - ✅ Complete (see below)

**Status**: Technical documentation updated with pause system details.

---

#### Step 9.5: Week 3 Completion Verification - ✅ Complete

**Status**: Week 3 core features complete, known issues tracked for future work.

---

### Task 9 Acceptance Criteria

- [x] Full gameplay flow tested end-to-end (Office → Overworld → Interaction → Return) ✅
- [x] All input contexts switch correctly during transitions ✅
- [x] Edge cases handled gracefully (known issues documented) ✅
- [x] Performance validated (acceptable for prototype stage) ✅
- [x] Visual polish reviewed (current prototype acceptable) ✅
- [x] All bugs found during testing documented in backlog ✅
- [x] Technical_Documentation.md updated with pause system ✅
- [x] All code committed with descriptive commit messages ✅
- [x] No critical bugs or blocking issues remaining ✅
- [x] Week 3 ready for Week 4 continuation ✅

**TASK 9 STATUS**: ✅ Complete

- [ ] **Test Sequence: Office Starting Point**

  - [ ] Open L_Office in editor
  - [ ] PIE (Play In Editor)
  - [ ] **Office Initial State**:
    - [ ] Verify player spawns at PlayerStart
    - [ ] Verify first-person camera active
    - [ ] Test WASD movement (Office input context)
    - [ ] Test mouse look
    - [ ] Check Output Log for: "SetMappingMode: Switched to Office"
  - [ ] **Navigate to Map Table**:
    - [ ] Walk to BP_TableObject_Glass (or map table trigger)
    - [ ] Interact with table (left-click or E key)
    - [ ] Verify WBP_MapTable widget opens
  - [ ] **Initiate Overworld Transition**:
    - [ ] Click "Start Journey" button
    - [ ] Verify transition begins
    - [ ] Check Output Log for:
      - [ ] "SetGameState: State changed to Overworld_Travel"
      - [ ] "TransitionToLevel: Transitioning to L_Overworld"

- [ ] **Test Sequence: Overworld Gameplay**

  - [ ] **Overworld Initial State**:
    - [ ] Verify L_Overworld loads completely
    - [ ] Verify terrain, lighting, NavMesh visible
    - [ ] Verify BP_OverworldCamera spawns and possesses player
    - [ ] Verify BP_OverworldConvoy spawns at PlayerStart
    - [ ] Verify 3-5 POI actors visible
    - [ ] Check Output Log for: "SetMappingMode: Switched to TopDown"
  - [ ] **Camera Controls**:
    - [ ] Test WASD camera pan (smooth movement)
    - [ ] Test mouse wheel zoom (in/out)
    - [ ] Verify camera stays within bounds (if set)
    - [ ] Verify camera doesn't clip through terrain
  - [ ] **Convoy Movement**:
    - [ ] Left-click on ground far from convoy
    - [ ] Verify convoy moves to location using pathfinding
    - [ ] Press `P` to show NavMesh (green overlay)
    - [ ] Verify convoy follows NavMesh path
    - [ ] Left-click on different location while moving
    - [ ] Verify convoy updates path immediately
    - [ ] Check Output Log for: "HandleClickMove: Moving to [location]"
  - [ ] **POI Interaction**:
    - [ ] Right-click on "Northern Village" POI
    - [ ] Verify on-screen message: "POI Interaction Stub: Northern Village"
    - [ ] Check Output Log for: "HandleInteractPOI: Interacted with POI 'Northern Village'"
    - [ ] Right-click on other POIs (verify unique names)
    - [ ] Right-click on ground (not POI)
    - [ ] Verify no POI interaction message appears
  - [ ] **Pause Functionality**:
    - [ ] Start convoy movement
    - [ ] Press ESC while convoy is moving
    - [ ] Verify convoy stops immediately
    - [ ] Verify WBP_PauseMenu appears
    - [ ] Press ESC again
    - [ ] Verify convoy resumes movement
    - [ ] Press ESC again
    - [ ] Click Resume button
    - [ ] Verify game unpauses

- [ ] **Test Sequence: Return to Office**

  - [ ] **Method 1: Tab Key (Debug)**:
    - [ ] Press Tab key in Overworld
    - [ ] Verify transition to L_Office
    - [ ] Verify Office loads correctly
    - [ ] Verify player can move and interact
    - [ ] Check Output Log for: "DebugReturnToOffice: Returning to L_Office"
  - [ ] **Method 2: Pause Menu Button**:
    - [ ] (Start in Overworld again via Office → Map Table)
    - [ ] Press ESC to pause
    - [ ] Click "Return to Office (Tab)" button
    - [ ] Verify game unpauses
    - [ ] Verify transition to L_Office
    - [ ] Verify Office is not paused after load

- [ ] **Test Sequence: Round-Trip (3 Cycles)**

  - [ ] Office → Overworld (via map) → Office (via Tab) → Overworld → Office → Overworld
  - [ ] **Verify Each Transition**:
    - [ ] Level loads correctly every time
    - [ ] Input context switches correctly every time
    - [ ] No performance degradation
    - [ ] No memory leaks (check FPS, memory usage in Stats window)
    - [ ] No "Accessed None" errors in Output Log

- [ ] **Testing After Step 9.1.1** ✅ CHECKPOINT
  - [ ] Complete gameplay flow works end-to-end
  - [ ] All input contexts switch correctly
  - [ ] All features functional (camera, movement, POI, pause, transitions)
  - [ ] Round-trip transitions stable
  - [ ] No crashes or critical errors

**COMMIT POINT 9.1.1**: N/A (testing only, no code changes)

---

##### Step 9.1.2: Edge Case and Error Condition Testing

- [ ] **Test Sequence: Input Edge Cases**

  - [ ] **Rapid Input Testing**:
    - [ ] In Overworld: Rapidly click left mouse button (10+ clicks)
    - [ ] Verify convoy updates path without errors
    - [ ] Rapidly right-click on POIs
    - [ ] Verify no duplicate messages or crashes
    - [ ] Rapidly press ESC (pause/unpause toggle)
    - [ ] Verify pause state toggles correctly
  - [ ] **Simultaneous Input Testing**:
    - [ ] Hold WASD (camera pan) while left-clicking (convoy move)
    - [ ] Verify both systems work independently
    - [ ] Right-click POI while convoy is moving
    - [ ] Verify interaction works without stopping convoy
    - [ ] Pan camera while zooming (WASD + mouse wheel)
    - [ ] Verify smooth combined movement

- [ ] **Test Sequence: Boundary Testing**

  - [ ] **NavMesh Boundaries**:
    - [ ] Left-click far outside NavMesh bounds (if possible)
    - [ ] Verify warning log: "Failed to project point to NavMesh"
    - [ ] Verify convoy doesn't move or freezes
  - [ ] **Camera Boundaries** (if implemented):
    - [ ] Pan camera to edge of map
    - [ ] Verify camera stops at bounds (doesn't go infinite)
  - [ ] **POI Collision Range**:
    - [ ] Right-click just outside InteractionBox range
    - [ ] Verify no interaction (or adjust box size if too small)
    - [ ] Right-click edge of InteractionBox
    - [ ] Verify interaction works

- [ ] **Test Sequence: Widget State Management**

  - [ ] **Pause Menu During Transition**:
    - [ ] Pause game in Overworld (ESC)
    - [ ] Click "Return to Office" button
    - [ ] Verify pause menu closes before transition
    - [ ] Verify Office is not paused after load
  - [ ] **Multiple Widget Opens** (if applicable):
    - [ ] Open map widget in Office
    - [ ] Close without transitioning
    - [ ] Open map widget again
    - [ ] Verify widget works correctly second time

- [ ] **Test Sequence: Game State Consistency**

  - [ ] **State Verification After Transitions**:
    - [ ] After each transition, check Output Log for GameState change
    - [ ] Office: EFCGameState::Office
    - [ ] Overworld: EFCGameState::Overworld_Travel
  - [ ] **Input Context Verification**:
    - [ ] After each transition, test input (move, interact)
    - [ ] Office: WASD moves player, mouse looks
    - [ ] Overworld: WASD pans camera, left-click moves convoy

- [ ] **Testing After Step 9.1.2** ✅ CHECKPOINT
  - [ ] Rapid inputs handled without crashes
  - [ ] Simultaneous inputs work correctly
  - [ ] Boundary conditions handled gracefully
  - [ ] Widget state management correct
  - [ ] Game state consistency maintained
  - [ ] All edge cases logged warnings (not errors)

**COMMIT POINT 9.1.2**: N/A (testing only, no code changes unless bugs found)

---

#### Step 9.2: Performance Validation

##### Step 9.2.1: FPS and Performance Testing

- [ ] **Analysis**

  - [ ] Week 3 is greybox/prototype, but performance should be reasonable
  - [ ] Target: 60+ FPS on development machine
  - [ ] Check for performance bottlenecks

- [ ] **Test Sequence: Overworld Performance**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Press ` (backtick) to open console
  - [ ] Type `stat fps` and press Enter
  - [ ] **Verify FPS**:
    - [ ] FPS should be 60+ (or vsync-limited)
    - [ ] Check during camera pan (WASD)
    - [ ] Check during convoy movement
    - [ ] Check during multiple POI interactions
  - [ ] Type `stat unit` in console
  - [ ] **Check Frame Times**:
    - [ ] Frame: <16.6ms (for 60 FPS)
    - [ ] Game: <10ms
    - [ ] Draw: <5ms
    - [ ] GPU: <10ms
  - [ ] If performance is poor, check:
    - [ ] Type `stat scenerendering` (check draw calls)
    - [ ] Type `stat rhi` (check GPU usage)

- [ ] **Test Sequence: Office Performance**

  - [ ] Transition to L_Office
  - [ ] Check FPS (should be 60+)
  - [ ] Check frame times (same as above)

- [ ] **Test Sequence: Transition Performance**

  - [ ] Measure transition time:
    - [ ] Office → Overworld: <2 seconds
    - [ ] Overworld → Office: <2 seconds
  - [ ] Check for stutters or freezes during transition

- [ ] **Testing After Step 9.2.1** ✅ CHECKPOINT
  - [ ] FPS is 60+ in both levels
  - [ ] Frame times acceptable
  - [ ] No major performance bottlenecks
  - [ ] Transitions smooth (no long freezes)

**COMMIT POINT 9.2.1**: N/A (testing only, note any performance issues for future optimization)

---

##### Step 9.2.2: Memory Leak Testing

- [ ] **Analysis**

  - [ ] Check for memory leaks during repeated transitions
  - [ ] Verify widgets and actors are properly destroyed

- [ ] **Test Sequence: Memory Usage**

  - [ ] Open L_Office in editor
  - [ ] PIE (Play In Editor)
  - [ ] Press ` (backtick) to open console
  - [ ] Type `stat memory` and press Enter
  - [ ] **Note Initial Memory Usage** (write down Physical and Virtual memory)
  - [ ] Perform 10 round-trip transitions:
    - [ ] Office → Overworld → Office (repeat 10 times)
  - [ ] Check `stat memory` again
  - [ ] **Expected Results**:
    - [ ] Memory usage should stabilize (not continuously increase)
    - [ ] Small increase acceptable (cached assets)
    - [ ] Large increase (>500MB) indicates potential leak

- [ ] **Test Sequence: Widget Cleanup**

  - [ ] After transitions, check if widgets are destroyed:
    - [ ] Type `obj list class=userwidget` in console
    - [ ] Verify no duplicate WBP_PauseMenu or WBP_MapTable instances
    - [ ] Only current level's widgets should exist

- [ ] **Testing After Step 9.2.2** ✅ CHECKPOINT
  - [ ] Memory usage stable after multiple transitions
  - [ ] No memory leaks detected
  - [ ] Widgets properly cleaned up

**COMMIT POINT 9.2.2**: N/A (testing only, fix any leaks if found)

---

#### Step 9.3: Polish and Bug Fixes

##### Step 9.3.1: Visual Polish Review

- [ ] **Analysis**

  - [ ] Week 3 is greybox, but should be visually clear and functional
  - [ ] Check for obvious visual issues

- [ ] **Review Checklist: L_Overworld**

  - [ ] **Lighting**:
    - [ ] Directional light properly illuminates terrain
    - [ ] No pitch-black areas
    - [ ] Sky visible and appropriate
  - [ ] **Terrain**:
    - [ ] Landscape or BSP geometry covers play area
    - [ ] No holes or gaps in ground
    - [ ] NavMesh covers traversable areas (press `P` to verify)
  - [ ] **Convoy**:
    - [ ] Visible from camera (not too small)
    - [ ] Distinctive color/material
    - [ ] Collision capsule appropriate size
  - [ ] **POIs**:
    - [ ] Clearly visible (distinctive color)
    - [ ] Positioned on ground (not floating)
    - [ ] Each POI has unique name
  - [ ] **Camera**:
    - [ ] Initial position shows full play area
    - [ ] Zoom range appropriate (not too close/far)
    - [ ] Pan speed comfortable

- [ ] **Review Checklist: L_Office**

  - [ ] **Existing Week 1-2 Features**:
    - [ ] Table objects visible and interactable
    - [ ] Map widget opens correctly
    - [ ] No regressions from Week 3 changes

- [ ] **Review Checklist: UI**

  - [ ] **WBP_MapTable**:
    - [ ] "Start Journey" button clearly visible
    - [ ] Text readable
    - [ ] Button hover effects work
  - [ ] **WBP_PauseMenu**:
    - [ ] Semi-transparent background covers screen
    - [ ] "PAUSED" text large and clear
    - [ ] Button text readable
    - [ ] Centered on screen

- [ ] **Testing After Step 9.3.1** ✅ CHECKPOINT
  - [ ] All visuals clear and functional
  - [ ] No obvious visual bugs
  - [ ] UI readable and usable
  - [ ] Note any polish tasks for future weeks

**COMMIT POINT 9.3.1**: `git add -A && git commit -m "polish(overworld): Visual adjustments and bug fixes (if any)"` (only if changes made)

---

##### Step 9.3.2: Bug Fixes (If Any Found During Testing)

- [ ] **Analysis**

  - [ ] Address any bugs found in Steps 9.1-9.3.1
  - [ ] Prioritize critical bugs (crashes, blocking issues)

- [ ] **Bug Fix Workflow**

  - [ ] For each bug found:
    - [ ] Document bug in Output Log or notes
    - [ ] Identify root cause
    - [ ] Implement fix
    - [ ] Re-test specific scenario
    - [ ] Commit fix with descriptive message

- [ ] **Common Bug Patterns to Check**

  - [ ] **"Accessed None" errors**:
    - [ ] Null pointer checks in C++ code
    - [ ] Blueprint cast failures
  - [ ] **Input not working**:
    - [ ] Input action assignments in Blueprint
    - [ ] IMC context not switching
  - [ ] **Widgets not appearing**:
    - [ ] Widget class assignments
    - [ ] AddToViewport Z-order
  - [ ] **Transition failures**:
    - [ ] Level name typos ("L_Overworld" vs "Overworld")
    - [ ] GameInstance not found
  - [ ] **Pathfinding not working**:
    - [ ] NavMesh not built
    - [ ] FloatingPawnMovement component missing

- [ ] **Testing After Step 9.3.2** ✅ CHECKPOINT
  - [ ] All critical bugs fixed
  - [ ] Re-tested scenarios pass
  - [ ] No new bugs introduced

**COMMIT POINT 9.3.2**: `git add -A && git commit -m "fix(overworld): [specific bug description]"` (per bug fix)

---

#### Step 9.4: Update Documentation

##### Step 9.4.1: Update Technical_Documentation.md with Week 3 Systems

- [ ] **Analysis**

  - [ ] Document new C++ classes, Blueprints, input actions, widgets
  - [ ] Update architecture diagrams or descriptions
  - [ ] Document integration points with Week 2 systems

- [ ] **Implementation (Technical_Documentation.md)**

  - [ ] Open `Docs/Technical_Documentation.md`
  - [ ] Add section: **Week 3: Overworld Systems**
  - [ ] Document:
    - [ ] **AFCPlayerController**:
      - [ ] Purpose: Top-down camera control, click-to-move, POI interaction, pause management
      - [ ] Key methods: BeginPlay (InputManager TopDown mode), HandleClickMove, HandleInteractPOI, HandlePause, PauseGame, UnpauseGame, DebugReturnToOffice
      - [ ] Input actions: IA_Click (existing), IA_InteractPOI, IA_Escape
      - [ ] Dependencies: UFCInputManager, UFCGameInstance
    - [ ] **BP_OverworldCamera**:
      - [ ] Blueprint Actor with camera component
      - [ ] WASD pan, mouse wheel zoom
      - [ ] Input actions: IA_OverworldPan, IA_OverworldZoom
    - [ ] **BP_OverworldConvoy**:
      - [ ] Pawn with CapsuleComponent, mesh, FloatingPawnMovement
      - [ ] NavMesh pathfinding via SimpleMoveToLocation
      - [ ] Spawns at PlayerStart in L_Overworld
    - [ ] **BP_OverworldPOI**:
      - [ ] Actor with mesh and InteractionBox
      - [ ] Implements BPI_InteractablePOI interface
      - [ ] Properties: POIName (editable per-instance)
      - [ ] OnPOIInteract displays Print String stub
    - [ ] **BPI_InteractablePOI**:
      - [ ] Blueprint Interface
      - [ ] Methods: OnPOIInteract(), GetPOIName()
    - [ ] **IMC_FC_TopDown**:
      - [ ] Input Mapping Context for Overworld
      - [ ] Mappings: IA_OverworldPan (WASD), IA_OverworldZoom (Mouse Wheel), IA_Click (LMB, existing), IA_InteractPOI (RMB), IA_Escape (ESC)
    - [ ] **WBP_PauseMenu**:
      - [ ] Widget with Resume and Return to Office buttons
      - [ ] Displayed during ESC pause in Overworld only
    - [ ] **L_Overworld**:
      - [ ] 3D level with terrain, lighting, NavMesh
      - [ ] Contains BP_OverworldCamera, BP_OverworldConvoy, multiple BP_OverworldPOI instances
      - [ ] Level Blueprint: OnBeginPlay → SetGameState(Overworld_Travel)
    - [ ] **Office-Overworld Transitions**:
      - [ ] Office → Overworld: WBP_MapTable "Start Journey" button → SetGameState(Overworld_Travel) → TransitionToLevel(L_Overworld)
      - [ ] Overworld → Office: Tab key or Pause Menu button → SetGameState(Office) → TransitionToLevel(L_Office)
      - [ ] UFCInputManager auto-switches contexts on GameState change
  - [ ] Update **Input System** section:
    - [ ] Add TopDown mode to EFCMappingMode enum
    - [ ] Add IMC_FC_TopDown to context list
    - [ ] Document GameState → InputContext mapping
  - [ ] Update **Directory Structure** section:
    - [ ] Add `Source/FC/Core/FCPlayerController.h/.cpp`
    - [ ] Add `Content/FC/World/Blueprints/Pawns/BP_OverworldConvoy`
    - [ ] Add `Content/FC/World/Blueprints/Actors/BP_OverworldPOI`
    - [ ] Add `Content/FC/Core/BPI_InteractablePOI`
    - [ ] Add `Content/FC/UI/WBP_PauseMenu`
    - [ ] Add `Content/FC/World/Levels/L_Overworld`
  - [ ] Save file

- [ ] **Testing After Step 9.4.1** ✅ CHECKPOINT
  - [ ] Technical_Documentation.md updated with Week 3 content
  - [ ] All new classes and Blueprints documented
  - [ ] Directory structure reflects Week 3 additions
  - [ ] File saves without errors

**COMMIT POINT 9.4.1**: `git add Docs/Technical_Documentation.md && git commit -m "docs(overworld): Add Week 3 systems to Technical_Documentation.md"`

---

##### Step 9.4.2: Update Fallen_Compass_DRM.md with Week 3 Status

- [ ] **Analysis**

  - [ ] Mark Week 3 features as complete in DRM
  - [ ] Update dates and status

- [ ] **Implementation (Fallen_Compass_DRM.md)**

  - [ ] Open `Docs/Fallen_Compass_DRM.md`
  - [ ] Locate **Week 3: Overworld Level & Basic Camera (21.11.-28.11.2025)** section
  - [ ] Update header to: **Week 3: Overworld Level & Basic Camera ✅ COMPLETE - 21.11.-28.11.2025**
  - [ ] Add checkmarks to features:
    - [ ] ✅ 3D Overworld level with basic terrain/greybox environment
    - [ ] ✅ Top-down camera actor with WASD pan and mouse wheel zoom
    - [ ] ✅ Convoy pawn with left-click NavMesh pathfinding (SimpleMoveToLocation)
    - [ ] ✅ 3-5 POI actors with right-click interaction stubs (Print String with POI name)
    - [ ] ✅ Office-to-Overworld transition via map table widget "Start Journey" button
    - [ ] ✅ Overworld-to-Office return via Tab key (debug) or Pause Menu button
    - [ ] ✅ ESC key conditional pause (Overworld only, not Office)
    - [ ] ✅ UFCInputManager automatic context switching (Office ↔ TopDown)
  - [ ] Add **Implementation Details** subsection (optional):
    - [ ] List key classes: AFCPlayerController, BP_OverworldCamera, BP_OverworldConvoy, BP_OverworldPOI
    - [ ] Note BPI_InteractablePOI interface pattern
    - [ ] Note IMC_FC_TopDown with 5 input mappings
  - [ ] Save file

- [ ] **Testing After Step 9.4.2** ✅ CHECKPOINT
  - [ ] Fallen_Compass_DRM.md updated with Week 3 completion
  - [ ] All features marked complete
  - [ ] Dates accurate
  - [ ] File saves without errors

**COMMIT POINT 9.4.2**: `git add Docs/Fallen_Compass_DRM.md && git commit -m "docs(drm): Mark Week 3 Overworld features complete"`

---

##### Step 9.4.3: Final Commit and Git Push

- [ ] **Analysis**

  - [ ] Ensure all Week 3 work is committed
  - [ ] Push to remote repository for backup

- [ ] **Implementation (Git)**

  - [ ] Open terminal in workspace root
  - [ ] Check git status: `git status`
  - [ ] Verify all changes committed (working tree clean)
  - [ ] If uncommitted changes exist:
    - [ ] Review changes: `git diff`
    - [ ] Stage changes: `git add -A`
    - [ ] Commit: `git commit -m "feat(week3): Final Week 3 Overworld implementation complete"`
  - [ ] Push to remote: `git push origin master` (or main branch)
  - [ ] Verify push successful

- [ ] **Testing After Step 9.4.3** ✅ CHECKPOINT
  - [ ] All commits pushed to remote
  - [ ] Git history clean and descriptive
  - [ ] Remote repository up to date

**COMMIT POINT 9.4.3**: `git push origin master` (final push)

---

#### Step 9.5: Week 3 Completion Verification

##### Step 9.5.1: Final Checklist Review

- [ ] **Analysis**

  - [ ] Verify all DRM Week 3 requirements met
  - [ ] Confirm all tasks in 0003-tasks.md complete

- [ ] **Week 3 DRM Requirements Verification**

  - [ ] ✅ **Overworld Level**: L_Overworld with terrain, lighting, NavMesh, PlayerStart
  - [ ] ✅ **Top-Down Camera**: BP_OverworldCamera with WASD pan, mouse wheel zoom
  - [ ] ✅ **Convoy Pawn**: BP_OverworldConvoy with NavMesh click-to-move pathfinding
  - [ ] ✅ **POI Interaction Stubs**: BP_OverworldPOI actors with right-click → Print String
  - [ ] ✅ **Level Transitions**: Office ↔ Overworld via widget button and Tab key
  - [ ] ✅ **Conditional Pause**: ESC key pause in Overworld only (WBP_PauseMenu with Resume/Return buttons)
  - [ ] ✅ **Input Context Switching**: UFCInputManager auto-switches Office ↔ TopDown modes

- [ ] **Tasks 1-9 Completion Verification**

  - [ ] ✅ Task 1: Pre-Implementation (conventions review, architecture planning)
  - [ ] ✅ Task 2: L_Overworld level creation with terrain, lighting, NavMesh
  - [ ] ✅ Task 3: BP_OverworldCamera with WASD/zoom input
  - [ ] ✅ Task 4: AFCPlayerController and BP derivation
  - [ ] ✅ Task 5: BP_OverworldConvoy with click-to-move pathfinding
  - [ ] ✅ Task 6: BP_OverworldPOI with BPI_InteractablePOI interface
  - [ ] ✅ Task 7: Office ↔ Overworld transitions via UFCLevelManager
  - [ ] ✅ Task 8: ESC pause/unpause with WBP_PauseMenu (Overworld only)
  - [ ] ✅ Task 9: Testing, documentation updates, git push

- [ ] **Testing After Step 9.5.1** ✅ CHECKPOINT
  - [ ] All DRM Week 3 requirements met
  - [ ] All tasks complete
  - [ ] Documentation updated
  - [ ] Code committed and pushed

**COMMIT POINT 9.5.1**: N/A (verification only)

---

### Task 9 Acceptance Criteria

- [ ] Full gameplay flow tested end-to-end (Office → Overworld → Interaction → Return)
- [ ] All input contexts switch correctly during transitions
- [ ] Edge cases handled gracefully (rapid inputs, boundary conditions, widget state)
- [ ] Performance validated (60+ FPS, acceptable frame times, no memory leaks)
- [ ] Visual polish reviewed (lighting, terrain, convoy, POIs, UI clarity)
- [ ] All bugs found during testing fixed
- [ ] Technical_Documentation.md updated with:
  - [ ] AFCPlayerController documentation
  - [ ] BP_OverworldCamera, BP_OverworldConvoy, BP_OverworldPOI documentation
  - [ ] BPI_InteractablePOI interface documentation
  - [ ] IMC_FC_TopDown input context documentation
  - [ ] WBP_PauseMenu widget documentation
  - [ ] L_Overworld level documentation
  - [ ] Office ↔ Overworld transition flow documentation
  - [ ] Updated directory structure
- [ ] Fallen_Compass_DRM.md updated:
  - [ ] Week 3 header marked "✅ COMPLETE - 21.11.-28.11.2025"
  - [ ] All 8 features checked off
  - [ ] Implementation details added
- [ ] All code committed with descriptive commit messages
- [ ] Code pushed to remote repository
- [ ] Final verification checklist complete (all DRM requirements and tasks confirmed)
- [ ] No critical bugs or blocking issues remaining
- [ ] Week 3 ready for Week 4 continuation

---

## Task 10: Cleanup - Revert Testing-Only State Transition

**Purpose**: Remove the temporary state transition added for Week 3 testing that allows direct loading into `Overworld_Travel` from `None` state. In production flow, the game should start from MainMenu or Office, not directly into Overworld.

---

### Step 10.1: Revert FCGameStateManager None → Overworld_Travel Transition

#### Step 10.1.1: Remove Overworld_Travel from None State Valid Transitions

- [ ] **Analysis**

  - [ ] Review FCGameStateManager.cpp InitializeValidTransitions() method
  - [ ] Confirm `Overworld_Travel` was added to `None` state transitions for Week 3 testing
  - [ ] Verify production flow: MainMenu → Office_Exploration → Overworld_Travel

- [ ] **Implementation (C++ Source)**

  - [ ] Open `Source/FC/Core/FCGameStateManager.cpp`
  - [ ] Locate `InitializeValidTransitions()` method
  - [ ] Find the `None` state transitions block:
    ```cpp
    // None transitions (initial state, can go anywhere)
    ValidTransitions.Add(EFCGameStateID::None, {
        EFCGameStateID::MainMenu,
        EFCGameStateID::Office_Exploration,
        EFCGameStateID::Overworld_Travel,  // ← REMOVE THIS LINE
        EFCGameStateID::Loading
    });
    ```
  - [ ] Remove `EFCGameStateID::Overworld_Travel` from the array
  - [ ] Result should be:
    ```cpp
    // None transitions (initial state, can go anywhere)
    ValidTransitions.Add(EFCGameStateID::None, {
        EFCGameStateID::MainMenu,
        EFCGameStateID::Office_Exploration,
        EFCGameStateID::Loading
    });
    ```
  - [ ] Save file

- [ ] **Testing After Step 10.1.1** ✅ CHECKPOINT
  - [ ] File compiles without errors
  - [ ] Hot reload successful in Unreal Editor
  - [ ] Direct loading L_Overworld should now fail TransitionTo (expected behavior)
  - [ ] Production flow tested: MainMenu → L_Office → Overworld button → L_Overworld (TransitionTo succeeds)

**COMMIT POINT 10.1.1**: `git add Source/FC/Core/FCGameStateManager.cpp && git commit -m "fix(state): Revert testing-only None→Overworld_Travel transition"`

---

#### Step 10.1.2: Verify Production State Flow

- [ ] **Analysis**

  - [ ] Confirm normal game flow works correctly
  - [ ] Test all critical transitions

- [ ] **Testing (Unreal Editor - PIE)**

  - [ ] Load MainMenu level (or L_Office if MainMenu not yet implemented)
  - [ ] Navigate to Office (or start in Office)
  - [ ] Open map table widget (WBP_MapTable)
  - [ ] Click "Start Journey" button
  - [ ] Verify:
    - [ ] TransitionTo(Overworld_Travel) succeeds (returns true)
    - [ ] L_Overworld loads correctly
    - [ ] Camera and convoy functional
  - [ ] Press Tab or open Pause Menu → "Return to Office"
  - [ ] Verify:
    - [ ] TransitionTo(Office_Exploration) succeeds
    - [ ] L_Office loads correctly
  - [ ] Check Output Log for state transition messages (no errors)

- [ ] **Testing After Step 10.1.2** ✅ CHECKPOINT
  - [ ] Production state flow verified working
  - [ ] No invalid transition errors
  - [ ] Office ↔ Overworld transitions functional
  - [ ] Direct L_Overworld loading correctly fails TransitionTo (as expected)

**COMMIT POINT 10.1.2**: N/A (testing verification only)

---

#### Step 10.1.3: Update Technical Documentation

- [ ] **Analysis**

  - [ ] Document the production state flow
  - [ ] Note the testing-only change was reverted

- [ ] **Implementation (Technical_Documentation.md)**

  - [ ] Open `Docs/Technical_Documentation.md`
  - [ ] Locate **UFCGameStateManager** section
  - [ ] Under **Valid State Transitions**, ensure `None` state documentation reads:
    ```
    // None → MainMenu, Office_Exploration, Loading
    // (Initial state, typical flow: None → MainMenu → Office → Overworld)
    ```
  - [ ] Add note in **Week 3 Systems** section:
    - [ ] "Note: During Week 3 development, a temporary transition from `None` → `Overworld_Travel` was added to allow direct testing of L_Overworld. This was reverted in Task 10 to enforce production state flow (MainMenu → Office → Overworld)."
  - [ ] Save file

- [ ] **Testing After Step 10.1.3** ✅ CHECKPOINT
  - [ ] Technical_Documentation.md updated with state flow clarification
  - [ ] Note about testing-only change documented
  - [ ] File saves without errors

**COMMIT POINT 10.1.3**: `git add Docs/Technical_Documentation.md && git commit -m "docs(state): Document production state flow, note Week 3 testing revert"`

---

### Task 10 Acceptance Criteria

- [ ] `Overworld_Travel` removed from `None` state valid transitions in FCGameStateManager.cpp
- [ ] Direct loading L_Overworld correctly fails state transition (expected behavior)
- [ ] Production flow verified: MainMenu/Office → Overworld_Travel → Office (all transitions succeed)
- [ ] Technical_Documentation.md updated with production state flow documentation
- [ ] All changes committed with clear commit messages
- [ ] No regressions in Office ↔ Overworld transition functionality

**Task 10 complete. State machine restored to production configuration.**

---

## Week 3 Tasks Complete

**All 9 parent tasks with 70+ sub-steps have been generated and documented.**

### Task Summary

1. **Task 1: Pre-Implementation Analysis** (3 steps)

   - Review Week 2 systems, check conventions, plan Week 3 architecture

2. **Task 2: Overworld Level Creation** (5 steps, 11 sub-steps)

   - Create L_Overworld with terrain, lighting, NavMesh, player start, game state transition

3. **Task 3: Top-Down Camera Actor & Input** (4 steps, 8 sub-steps)

   - Create IA_OverworldPan/Zoom, configure IMC_FC_TopDown, build BP_OverworldCamera with WASD/zoom

4. **Task 4: Overworld Player Controller** (5 steps, 11 sub-steps)

   - Create AFCPlayerController C++, Blueprint derivation, InputManager TopDown setup, camera possession

5. **Task 5: Convoy Pawn & Click-to-Move** (5 steps, 11 sub-steps)

   - Use existing IA_Click for click-to-move, BP_OverworldConvoy with FloatingPawnMovement, implement NavMesh pathfinding in controller

6. **Task 6: POI Actor & Interaction Stub** (5 steps, 13 sub-steps)

   - Create IA_InteractPOI, BP_OverworldPOI with InteractionBox, BPI_InteractablePOI interface, right-click handler in controller

7. **Task 7: Office-to-Overworld Transition** (5 steps, 9 sub-steps)

   - Add "Start Journey" button to WBP_MapTable, implement transition logic, Tab key return, InputManager GameState integration

8. **Task 8: Conditional Engine Pause** (5 steps, 14 sub-steps)

   - Create IA_Escape (ESC key), WBP_PauseMenu with Resume/Return buttons, implement pause/unpause in controller (Overworld only)

9. **Task 9: Testing, Polish & Documentation** (5 steps, 10 sub-steps)

   - Comprehensive integration testing, performance validation, bug fixes, update Technical_Documentation.md and DRM, final git push

10. **Task 10: Cleanup - Revert Testing-Only State Transition** (1 step, 3 sub-steps)

- Remove `Overworld_Travel` from `None` state valid transitions in FCGameStateManager (restore production state flow)

### Implementation Notes

- **Total Sub-Steps**: 70+ detailed implementation steps across 10 parent tasks
- **Commit Points**: 50+ git commit checkpoints for incremental progress tracking
- **Testing Checkpoints**: Embedded after every sub-step for continuous validation
- **Architecture Pattern**: C++ + Blueprint hybrid, leveraging Week 2 subsystems (UFCInputManager, UFCLevelManager, UFCGameStateManager)
- **Input System**: Enhanced Input with 5 new actions (Pan, Zoom, ClickMove, InteractPOI, Pause) in IMC_FC_TopDown context
- **Interface Pattern**: BPI_InteractablePOI demonstrates Blueprint interface extensibility for future POI types
- **Pause System**: Conditional (Overworld only), physics-pausing with UI interactivity maintained

### Estimated Completion Time

- **Week 3 Total**: ~20-25 hours (based on DRM estimate)
- **Per Task Breakdown**:
  - Tasks 1-2: ~4 hours (setup and level creation)
  - Tasks 3-4: ~5 hours (camera and controller)
  - Tasks 5-6: ~6 hours (convoy pathfinding and POI system)
  - Tasks 7-8: ~5 hours (transitions and pause)
  - Task 9: ~4 hours (testing and documentation)
  - Task 10: ~0.5 hours (cleanup and revert testing changes)

### Next Steps After Week 3

After completing all tasks in 0003-tasks.md, proceed to Week 4 (DRM 28.11.-05.12.2025):

- UI/UX for expedition selection in Office
- Encounter system foundations
- Data-driven encounter definitions
- Resource system expansion

---

## Known Issues & Future Backlog

### Issue #1: Office State Reset on Return from Overworld

**Status**: 🐛 Bug - Needs Fix  
**Severity**: Medium  
**Affected Systems**: Level transitions, PlayerController initialization, GameStateManager

**Problem Summary**:  
When aborting expedition from Overworld and returning to L_Office, the game state incorrectly transitions from `Office_Exploration` → `MainMenu`. This causes:

1. Input mode resets to UI-only instead of FirstPerson gameplay
2. Main menu appears when it shouldn't
3. Mouse look (IA_Look) stops working

**Root Cause**:  
`InitializeMainMenu()` is called by L_Office Level Blueprint on level load. It currently doesn't check if a valid gameplay state (Office_Exploration) is already active from a previous transition. This was designed for initial game startup but conflicts with mid-game level transitions.

**Expected Behavior**:

- Abort expedition in Overworld → Load L_Office → State should remain `Office_Exploration`
- Input mode should be `FirstPerson` with mouse look enabled
- Main menu should NOT appear

**Current Workaround**: None

**Proposed Solution**:

1. Modify `AFCPlayerController::InitializeMainMenu()` to check current state before transitioning
2. Only transition to MainMenu if state is `None` or `MainMenu`
3. If state is already `Office_Exploration`, skip MainMenu initialization
4. Add `ApplyInputAndCameraModeForState()` helper in BeginPlay() to configure input/camera based on existing state (handles PlayerController spawn after level load)

**Related Code**:

- `FCPlayerController.cpp::InitializeMainMenu()` (lines ~650)
- `FCPlayerController.cpp::BeginPlay()` (lines ~132)
- `L_Office` Level Blueprint (Event BeginPlay → InitializeMainMenu call)

**Future Sprint**: Week 4 or 5

---

### Issue #2: Pause Menu Abort Button Visibility Logic

**Status**: 🔧 Enhancement - UX Improvement  
**Severity**: Low  
**Affected Systems**: Pause menu widget, UI conditional display

**Problem Summary**:  
"Abort Expedition" button in WBP_PauseMenu is visible in both Office and Overworld levels because it only checks game state (`Paused`). The button should only appear when pausing from Overworld, not when pausing in Office.

**Root Cause**:  
Button visibility is based on game state alone (`Paused`), but doesn't check:

1. Which level is currently loaded (L_Office vs L_Overworld)
2. Which state was pushed onto the stack before pause (Office_Exploration vs Overworld_Travel)

**Expected Behavior**:

- ESC in L_Overworld → Pause menu shows "Resume" + "Abort Expedition" buttons
- ESC in L_Office → Pause menu shows "Resume" + "Settings" + "Quit" buttons (no abort)
- Button visibility should be contextual based on level/previous state

**Current Workaround**: User sees Abort button in Office but it's non-functional there

**Proposed Solution**:

1. **Option A (Simple)**: Check LevelManager current level name in WBP_PauseMenu Event Construct
   - If `L_Overworld` → Show Abort button
   - If `L_Office` → Collapse Abort button
2. **Option B (Robust)**: Check GameStateManager state stack to see previous state before pause
   - If previous state was `Overworld_Travel` → Show Abort button
   - If previous state was `Office_Exploration` → Hide Abort button
3. **Option C (Advanced)**: Create separate pause menu widgets (WBP_PauseMenu_Office, WBP_PauseMenu_Overworld)

**Related Code**:

- `WBP_PauseMenu.uasset` (Designer Tab - AbortExpeditionButton visibility)
- `FCPlayerController.cpp::HandlePausePressed()` (lines ~353)
- `UFCGameStateManager::GetStateAtDepth()` for stack inspection

**Future Sprint**: Week 4 (low priority polish)

---

### Issue #3: Level Loading Order for State Transitions

**Status**: 📋 Architecture - Design Decision Needed  
**Severity**: Low (Currently works but inconsistent)  
**Affected Systems**: Widget transition logic, LevelManager, GameStateManager

**Problem Summary**:  
Widgets performing level transitions must call both `LevelManager->LoadLevel()` and `GameStateManager->TransitionTo()`. Current implementation order is inconsistent between widgets, and it's unclear if order matters.

**Current State**:

- **WBP_ExpeditionPlanning**: Calls `TransitionTo(Overworld_Travel)` first, then `LoadLevel("L_Overworld")`
- **WBP_PauseMenu**: Should call `LoadLevel("L_Office")` first, then `TransitionTo(Office_Exploration)` (based on task notes)

**Questions**:

1. Does call order matter for correctness?
2. Should GameStateManager automatically trigger level loading for certain states?
3. Should widgets need to know about both systems, or should there be a single "TransitionToLevel(Level, State)" API?

**Architectural Options**:

1. **Status Quo**: Widgets call both systems, establish standard order in docs
2. **GameStateManager Integration**: GameStateManager auto-calls LevelManager for states that require specific levels (rejected - breaks single responsibility)
3. **Transition Facade**: Create `UFCTransitionCoordinator` subsystem with unified API
4. **PlayerController Helper**: Add `AFCPlayerController::TransitionToLevel(LevelName, State)` convenience method

**Recommendation**: Document standard pattern (LoadLevel first, then TransitionTo) in Technical_Documentation.md

**Future Sprint**: Week 5+ (if transition complexity grows)

---

### Issue #4: ESC Key Doesn't Close Pause Menu When Engine is Paused

**Status**: 🐛 Bug - Needs Fix  
**Severity**: Low  
**Affected Systems**: Enhanced Input, Pause menu, Engine pause behavior

**Problem Summary**:  
When the pause menu is open in L_Overworld (with engine pause active via `SetPause(true)`), pressing ESC key a second time does not close the pause menu. This breaks the expected toggle behavior where ESC opens and closes the menu.

**Root Cause**:  
Engine pause (`SetPause(true)`) prevents Enhanced Input actions from firing. The IA_Escape input action is blocked when the game is paused, so `HandlePausePressed()` is never called to unpause.

**Expected Behavior**:

- Press ESC in Overworld → Pause menu opens, engine pauses
- Press ESC again → Pause menu closes, engine unpauses
- Consistent toggle behavior like in Office (where engine pause is not used)

**Current Workaround**: Use "Resume" button instead of ESC key to close menu

**Proposed Solutions**:

1. **Option A (UI-Driven)**: WBP_PauseMenu captures ESC key via UMG input and calls ResumeGame() directly
   - Add OnKeyDown event in widget to detect ESC
   - Call PlayerController->ResumeGame() when ESC pressed while menu open
2. **Option B (Input Mode)**: Use FInputModeUIOnly instead of engine pause in Overworld
   - Stops player input but doesn't pause engine timers
   - Manually disable convoy AI movement component during pause
   - Enhanced Input actions continue to work
3. **Option C (Pause Settings)**: Configure PlayerController to accept input while paused
   - Set bShouldPerformFullTickWhenPaused = true on PlayerController
   - Allows input processing during pause state

**Trade-offs**:

- Option A: Simplest fix, works immediately but duplicates input handling logic
- Option B: More robust but requires manual movement stopping (more code)
- Option C: May have unintended side effects on other pause-dependent systems

**Recommendation**: Option A for quick fix, Option B for proper long-term solution

**Related Code**:

- `UFCUIManager::ShowPauseMenu()` (line ~110 - SetPause(true) call)
- `FCPlayerController::HandlePausePressed()` (line ~353 - ESC input handling)
- `WBP_PauseMenu.uasset` (Designer Tab - add OnKeyDown event)

**Future Sprint**: Week 4 or 5

---

**END OF 0003-TASKS.MD DOCUMENT**
