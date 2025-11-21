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
  - Source/FC/Core/FCOverworldPlayerController.h/.cpp
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

- `/Source/FC/Core/FCOverworldPlayerController.h/.cpp` - Top-down player controller for Overworld
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
- `/Content/FC/Input/Actions/IA_OverworldClickMove.uasset` - Left-click move command (Boolean)
- `/Content/FC/Input/Actions/IA_OverworldInteract.uasset` - Right-click interaction (Boolean)

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
- AFCOverworldPlayerController C++ class
- BP_OverworldConvoy pawn with click-to-move
- BP_OverworldPOI interaction stub
- Input Actions: IA_OverworldPan, IA_OverworldZoom, IA_OverworldClickMove, IA_OverworldInteract
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
  - [x] C++ Classes: `AFCOverworldPlayerController`, `AFCOverworldPawn` (A prefix for Actors)
  - [x] Blueprints: `BP_OverworldCamera`, `BP_OverworldConvoy`, `BP_OverworldPOI` (BP\_ prefix)
  - [x] Input Actions: `IA_OverworldPan`, `IA_OverworldZoom`, `IA_OverworldClickMove`, `IA_OverworldInteract` (IA\_ prefix)
  - [x] Input Mapping Context: `IMC_FC_TopDown` (already exists from Week 2)
  - [x] Level: `L_Overworld` (L\_ prefix)
  - [x] Folders: `/Source/FC/Core/` (controllers), `/Source/FC/World/` (pawns/actors), `/Content/FC/World/Levels/`, `/Content/FC/World/Blueprints/`

**Key Conventions Applied to Week 3:**

1. **Controller Pattern:**

   ```cpp
   // FCOverworldPlayerController.h
   UCLASS()
   class FC_API AFCOverworldPlayerController : public APlayerController
   {
       GENERATED_BODY()
   public:
       AFCOverworldPlayerController();
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
- Naming: All FC-prefixed (AFCOverworldPlayerController), BP* for Blueprints, IA* for input actions
- Folders match asset types per UE_NamingConventions.md structure

##### Step 1.0.3: Architecture Planning & Implementation Strategy

- [x] **Document Classes to Create**

  ```
  C++ Classes:
  1. AFCOverworldPlayerController (Source/FC/Core/)
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
  7. IA_OverworldClickMove (Boolean) - Left mouse button
  8. IA_OverworldInteract (Boolean) - Right mouse button
  9. IMC_FC_TopDown (configure with above actions)
  ```

- [x] **Document Expected Call Flow**

  ```
  Office → Overworld Transition:
  1. Player clicks "Start Test Expedition" in WBP_TableMap
  2. Button calls UFCLevelManager::LoadLevel("L_Overworld")
  3. UFCTransitionManager fades out, OpenLevel loads L_Overworld
  4. L_Overworld Level Blueprint calls UFCGameStateManager::TransitionTo(Overworld_Travel)
  5. AFCOverworldPlayerController::BeginPlay() sets InputManager to TopDown mode
  6. Camera possesses BP_OverworldCamera, player can pan/zoom

  Convoy Movement:
  1. Player left-clicks on ground
  2. AFCOverworldPlayerController::HandleClickMoveInput() raycasts to world
  3. Calls ControlledConvoy->SimpleMoveToLocation(HitLocation)
  4. BP_OverworldConvoy uses NavMesh pathfinding to move

  POI Interaction:
  1. Player right-clicks on BP_OverworldPOI
  2. AFCOverworldPlayerController::HandleInteractInput() raycasts to actors
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

**Purpose**: Implement BP_OverworldCamera with WASD panning, mouse wheel zoom, and optional edge scrolling for top-down view control.

---

#### Step 3.1: Create Input Actions for Overworld Camera

##### Step 3.1.1: Create IA_OverworldPan (Axis2D for WASD)

- [ ] **Analysis**

  - [ ] Check existing Input Actions in `/Content/FC/Input/Actions/` (IA_Move, IA_Look from Week 1)
  - [ ] Review IA_Move configuration for Axis2D pattern
  - [ ] Confirm naming: IA_OverworldPan (IA\_ prefix)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_OverworldPan`
  - [ ] Open IA_OverworldPan
  - [ ] Set Value Type: Axis2D (Vector2D)
  - [ ] Save asset

- [ ] **Testing After Step 3.1.1** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Axis2D
  - [ ] Asset saves without errors
  - [ ] No warnings in Output Log

**COMMIT POINT 3.1.1**: `git add Content/FC/Input/Actions/IA_OverworldPan.uasset && git commit -m "feat(overworld): Create IA_OverworldPan input action (Axis2D)"`

---

##### Step 3.1.2: Create IA_OverworldZoom (Axis1D for Mouse Wheel)

- [ ] **Analysis**

  - [ ] Review UE Enhanced Input mouse wheel axis configuration
  - [ ] Confirm value type: Axis1D (float) for scroll delta

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_OverworldZoom`
  - [ ] Open IA_OverworldZoom
  - [ ] Set Value Type: Axis1D (float)
  - [ ] Save asset

- [ ] **Testing After Step 3.1.2** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Axis1D
  - [ ] Asset saves without errors

**COMMIT POINT 3.1.2**: `git add Content/FC/Input/Actions/IA_OverworldZoom.uasset && git commit -m "feat(overworld): Create IA_OverworldZoom input action (Axis1D)"`

---

#### Step 3.2: Configure IMC_FC_TopDown Input Mapping Context

##### Step 3.2.1: Add WASD Bindings to IA_OverworldPan

- [ ] **Analysis**

  - [ ] Check IMC_FC_FirstPerson for WASD swizzle modifier patterns (Week 1)
  - [ ] Confirm IMC_FC_TopDown exists (created in Week 2, should be empty)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [ ] Add Mapping: IA_OverworldPan
  - [ ] Add Key: **W** (forward/up movement)
    - [ ] Add Modifier: Swizzle Input Axis Values
    - [ ] Set Order: YXZ (maps W to Y-axis positive)
  - [ ] Add Key: **S** (backward/down movement)
    - [ ] Add Modifier: Swizzle Input Axis Values (YXZ)
    - [ ] Add Modifier: Negate (makes Y negative)
  - [ ] Add Key: **D** (right movement)
    - [ ] No modifiers (X-axis positive by default)
  - [ ] Add Key: **A** (left movement)
    - [ ] Add Modifier: Negate (makes X negative)
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 3.2.1** ✅ CHECKPOINT
  - [ ] All 4 keys (WASD) bound to IA_OverworldPan
  - [ ] Modifiers configured correctly (Swizzle for W/S, Negate for S/A)
  - [ ] Asset saves without errors

**COMMIT POINT 3.2.1**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Configure WASD bindings for camera pan in IMC_FC_TopDown"`

---

##### Step 3.2.2: Add Mouse Wheel Binding to IA_OverworldZoom

- [ ] **Analysis**

  - [ ] Review UE mouse wheel input key name (Mouse Wheel Axis)
  - [ ] Determine if negate needed (wheel up = positive zoom or negative zoom)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown` (if not already open)
  - [ ] Add Mapping: IA_OverworldZoom
  - [ ] Add Key: **Mouse Wheel Axis**
    - [ ] No modifiers (or add Negate if zoom direction feels inverted)
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 3.2.2** ✅ CHECKPOINT
  - [ ] Mouse Wheel Axis bound to IA_OverworldZoom
  - [ ] Asset saves without errors
  - [ ] IMC_FC_TopDown now has 2 mappings (Pan and Zoom)

**COMMIT POINT 3.2.2**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Configure mouse wheel binding for camera zoom in IMC_FC_TopDown"`

---

#### Step 3.3: Create BP_OverworldCamera Blueprint

##### Step 3.3.1: Create Camera Actor Blueprint

- [ ] **Analysis**

  - [ ] Check if starter content has CameraActor examples
  - [ ] Review Week 1 BP_MenuCamera for camera actor patterns
  - [ ] Confirm folder: `/Content/FC/World/Blueprints/Cameras/`

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/World/Blueprints/` (create Cameras subfolder if needed)
  - [ ] Right-click → Blueprint Class → Camera Actor
  - [ ] Name: `BP_OverworldCamera`
  - [ ] Open BP_OverworldCamera
  - [ ] Components Panel:
    - [ ] Root: CameraComponent (default from CameraActor parent)
    - [ ] Optional: Add SpringArm component as root (for smoother zoom, attach camera to spring arm tip)
  - [ ] Select CameraComponent:
    - [ ] Set Location: X=0, Y=0, Z=0 (relative to root or spring arm)
    - [ ] Set Rotation: Pitch=-70, Yaw=0, Roll=0 (top-down angle)
  - [ ] If using SpringArm:
    - [ ] Set Target Arm Length: 1500 (starting zoom distance)
    - [ ] Enable Camera Lag: False (instant response for Week 3)
  - [ ] Compile and save Blueprint

- [ ] **Testing After Step 3.3.1** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] Camera component configured with top-down rotation
  - [ ] Asset saved in correct folder
  - [ ] Can place in L_Overworld viewport (test placement, then delete)

**COMMIT POINT 3.3.1**: `git add Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset && git commit -m "feat(overworld): Create BP_OverworldCamera actor with top-down configuration"`

---

##### Step 3.3.2: Add Pan Input Handling (Event Graph)

- [ ] **Analysis**

  - [ ] Review Enhanced Input action binding in Blueprint (Enhanced Input Action events)
  - [ ] Plan camera movement: AddActorWorldOffset based on input vector

- [ ] **Implementation (BP_OverworldCamera Event Graph)**

  - [ ] Add Event: Enhanced Input Action IA_OverworldPan (Triggered)
    - [ ] If event not available: Ensure Enhanced Input plugin enabled in Project Settings
  - [ ] Get Action Value:
    - [ ] From IA_OverworldPan event, get Action Value (Vector2D)
  - [ ] Calculate Movement Vector:
    - [ ] Multiply Vector2D by PanSpeed (add variable: PanSpeed, float, default 500.0)
    - [ ] Multiply by Delta Time (to make frame-rate independent)
  - [ ] Convert 2D to 3D:
    - [ ] Make Vector: X = ActionValue.X, Y = ActionValue.Y, Z = 0
  - [ ] Move Camera:
    - [ ] Add Actor World Offset (Target: Self, Delta Location: MovementVector, Sweep: False)
  - [ ] Add Variable: PanSpeed
    - [ ] Type: Float
    - [ ] Category: "Camera|Movement"
    - [ ] Default Value: 500.0
    - [ ] Instance Editable: True (Blueprint Details panel checkbox)
  - [ ] Compile and save

- [ ] **Testing After Step 3.3.2** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] PanSpeed variable visible in Details panel
  - [ ] Event graph wired correctly (no disconnected pins)
  - [ ] Save successful

**COMMIT POINT 3.3.2**: `git add Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset && git commit -m "feat(overworld): Implement WASD pan input handling in BP_OverworldCamera"`

---

##### Step 3.3.3: Add Zoom Input Handling (Event Graph)

- [ ] **Analysis**

  - [ ] Plan zoom behavior: Adjust SpringArm TargetArmLength OR move camera Z position
  - [ ] Determine zoom limits (min/max distance)

- [ ] **Implementation (BP_OverworldCamera Event Graph)**

  - [ ] Add Event: Enhanced Input Action IA_OverworldZoom (Triggered)
  - [ ] Get Action Value:
    - [ ] From IA_OverworldZoom event, get Action Value (float)
  - [ ] Calculate New Zoom:
    - [ ] **If using SpringArm:**
      - [ ] Get SpringArm → Get Target Arm Length
      - [ ] Subtract (ActionValue \* ZoomSpeed) - Note: subtract because wheel up (positive) should zoom in (decrease distance)
      - [ ] Clamp result between ZoomMin and ZoomMax
      - [ ] Set SpringArm Target Arm Length to clamped value
    - [ ] **If NOT using SpringArm:**
      - [ ] Get Actor Location (Z component)
      - [ ] Subtract (ActionValue \* ZoomSpeed)
      - [ ] Clamp Z between ZoomMinHeight and ZoomMaxHeight
      - [ ] Set Actor Location (keep X/Y, update Z)
  - [ ] Add Variables:
    - [ ] ZoomSpeed (Float, default 100.0, Instance Editable)
    - [ ] ZoomMin (Float, default 500.0, Instance Editable)
    - [ ] ZoomMax (Float, default 3000.0, Instance Editable)
    - [ ] All in Category: "Camera|Zoom"
  - [ ] Compile and save

- [ ] **Testing After Step 3.3.3** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] Zoom variables visible in Details panel
  - [ ] Event graph wired correctly
  - [ ] Save successful

**COMMIT POINT 3.3.3**: `git add Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset && git commit -m "feat(overworld): Implement mouse wheel zoom input handling in BP_OverworldCamera"`

---

##### Step 3.3.4: Add Edge Scrolling (Optional - Can Defer to Polish Phase)

- [ ] **Analysis**

  - [ ] Edge scrolling checks mouse position near screen edges
  - [ ] Requires Get Mouse Position in Viewport, compare to screen bounds
  - [ ] This is optional for Week 3 - can be deferred if time constrained

- [ ] **Implementation (BP_OverworldCamera Event Graph) - OPTIONAL**

  - [ ] Add Event: Event Tick (only if implementing edge scroll)
  - [ ] Get Player Controller → Get Mouse Position
  - [ ] Get Viewport Size
  - [ ] Check Edge Proximity:
    - [ ] If MouseX < EdgeThreshold (e.g., 50 pixels): Pan left
    - [ ] If MouseX > (ViewportWidth - EdgeThreshold): Pan right
    - [ ] If MouseY < EdgeThreshold: Pan forward
    - [ ] If MouseY > (ViewportHeight - EdgeThreshold): Pan backward
  - [ ] Calculate Pan Direction (Vector2D based on edge proximity)
  - [ ] Apply Pan (similar to WASD pan logic in 3.3.2)
  - [ ] Add Variable: EdgeScrollThreshold (Float, default 50.0, Instance Editable)
  - [ ] Compile and save

- [ ] **Alternative: Skip Edge Scrolling for Week 3**

  - [ ] Document in "Known Issues & Backlog" section as "Edge scrolling deferred to Week 4 polish"
  - [ ] Focus on core WASD/zoom functionality

- [ ] **Testing After Step 3.3.4** ✅ CHECKPOINT
  - [ ] If implemented: Mouse near edges pans camera
  - [ ] If skipped: Document in backlog
  - [ ] Blueprint compiles without errors

**COMMIT POINT 3.3.4**: `git add Content/FC/World/Blueprints/Cameras/BP_OverworldCamera.uasset && git commit -m "feat(overworld): Add edge scrolling support (optional)" OR "docs(overworld): Defer edge scrolling to Week 4 polish"`

---

### Task 3 Acceptance Criteria

- [ ] IA_OverworldPan input action created (Axis2D)
- [ ] IA_OverworldZoom input action created (Axis1D)
- [ ] IMC_FC_TopDown configured with WASD (pan) and Mouse Wheel (zoom) bindings
- [ ] BP_OverworldCamera created with top-down camera angle
- [ ] WASD pan functionality implemented and responds to input
- [ ] Mouse wheel zoom functionality implemented with min/max limits
- [ ] (Optional) Edge scrolling implemented OR documented as deferred
- [ ] All Blueprints compile without errors
- [ ] Assets saved in correct folders

**Task 3 complete. Ready for Task 4 sub-tasks (Overworld Player Controller)? Respond with 'Go' to continue.**

---

### Task 4: Overworld Player Controller & Input Context

**Purpose**: Create AFCOverworldPlayerController C++ class that uses UFCInputManager to switch to TopDown input mode and possesses BP_OverworldCamera.

---

#### Step 4.1: Create AFCOverworldPlayerController C++ Class

##### Step 4.1.1: Create C++ Class Files

- [ ] **Analysis**

  - [ ] Review AFCPlayerController.h/.cpp structure from Week 1
  - [ ] Check how InputManager component is created in AFCPlayerController constructor
  - [ ] Confirm naming: AFCOverworldPlayerController (A prefix for Actor-derived)
  - [ ] Confirm location: `/Source/FC/Core/` (same module as FCPlayerController)

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Create file: `W:\GameDev\FallenCompass\FC\Source\FC\Core\FCOverworldPlayerController.h`
  - [ ] Add header guards and includes:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/PlayerController.h"
    #include "FCOverworldPlayerController.generated.h"

    class UFCInputManager;
    class ACameraActor;

    DECLARE_LOG_CATEGORY_EXTERN(LogFCOverworldController, Log, All);
    ```

  - [ ] Define class:

    ```cpp
    /**
     * Player controller for Overworld level with top-down camera control
     * Uses UFCInputManager to switch to TopDown input mapping context
     * Possesses BP_OverworldCamera for WASD pan and zoom functionality
     */
    UCLASS()
    class FC_API AFCOverworldPlayerController : public APlayerController
    {
        GENERATED_BODY()

    public:
        AFCOverworldPlayerController();

    protected:
        virtual void BeginPlay() override;
        virtual void SetupInputComponent() override;

    private:
        /** Input manager component (handles TopDown input context) */
        UPROPERTY()
        TObjectPtr<UFCInputManager> InputManager;

        /** Reference to possessed camera actor */
        UPROPERTY()
        TObjectPtr<ACameraActor> OverworldCamera;
    };
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Create file: `W:\GameDev\FallenCompass\FC\Source\FC\Core\FCOverworldPlayerController.cpp`
  - [ ] Add includes:

    ```cpp
    #include "FCOverworldPlayerController.h"
    #include "FC/Components/FCInputManager.h"
    #include "Camera/CameraActor.h"
    #include "Kismet/GameplayStatics.h"
    #include "EnhancedInputSubsystems.h"
    #include "EnhancedInputComponent.h"

    DEFINE_LOG_CATEGORY(LogFCOverworldController);
    ```

  - [ ] Implement constructor:

    ```cpp
    AFCOverworldPlayerController::AFCOverworldPlayerController()
    {
        // Create Input Manager component
        InputManager = CreateDefaultSubobject<UFCInputManager>(TEXT("InputManager"));

        UE_LOG(LogFCOverworldController, Log, TEXT("AFCOverworldPlayerController: Constructor executed"));
    }
    ```

  - [ ] Implement BeginPlay:

    ```cpp
    void AFCOverworldPlayerController::BeginPlay()
    {
        Super::BeginPlay();

        // Switch to TopDown input mode
        if (InputManager)
        {
            InputManager->SetInputMappingMode(EFCInputMappingMode::TopDown);
            UE_LOG(LogFCOverworldController, Log, TEXT("BeginPlay: Switched to TopDown input mode"));
        }
        else
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("BeginPlay: InputManager is null!"));
        }

        // Find and possess BP_OverworldCamera in level
        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundCameras);

        for (AActor* Actor : FoundCameras)
        {
            if (Actor->GetName().Contains(TEXT("OverworldCamera")))
            {
                OverworldCamera = Cast<ACameraActor>(Actor);
                SetViewTarget(OverworldCamera);
                UE_LOG(LogFCOverworldController, Log, TEXT("BeginPlay: Possessed OverworldCamera: %s"), *Actor->GetName());
                break;
            }
        }

        if (!OverworldCamera)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("BeginPlay: No OverworldCamera found in level!"));
        }
    }
    ```

  - [ ] Implement SetupInputComponent:

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        // Input binding will be handled by BP_OverworldCamera responding to IA_OverworldPan/Zoom
        // Controller just needs to ensure TopDown IMC is active (done in BeginPlay)

        UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Input component initialized"));
    }
    ```

  - [ ] Save file

- [ ] **Testing After Step 4.1.1** ✅ CHECKPOINT
  - [ ] Files created at correct paths
  - [ ] No syntax errors in IDE
  - [ ] Ready to compile (will compile in next step)

**COMMIT POINT 4.1.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(overworld): Create AFCOverworldPlayerController C++ class"`

---

##### Step 4.1.2: Add to Build Configuration and Compile

- [ ] **Analysis**

  - [ ] Verify FC.Build.cs includes necessary modules (EnhancedInput, AIModule for NavMesh)
  - [ ] Check if Live Coding is enabled for hot reload

- [ ] **Implementation (FC.Build.cs)**

  - [ ] Open `W:\GameDev\FallenCompass\FC\Source\FC\FC.Build.cs`
  - [ ] Verify PublicDependencyModuleNames includes:
    ```csharp
    PublicDependencyModuleNames.AddRange(new string[] {
        "Core",
        "CoreUObject",
        "Engine",
        "InputCore",
        "EnhancedInput",
        "AIModule",  // For NavMesh pathfinding
        "NavigationSystem",  // For navigation queries
        "UMG"  // For UI widgets
    });
    ```
  - [ ] If AIModule or NavigationSystem missing, add them
  - [ ] Save FC.Build.cs

- [ ] **Compilation**

  - [ ] Close Unreal Editor (if open)
  - [ ] Open Visual Studio solution (`FC.sln`)
  - [ ] Build Solution (Ctrl+Shift+B or Build → Build Solution)
  - [ ] Check Output window for compilation success
  - [ ] Verify no errors or warnings for FCOverworldPlayerController

- [ ] **Testing After Step 4.1.2** ✅ CHECKPOINT
  - [ ] Compilation succeeds without errors
  - [ ] No linker errors
  - [ ] FC.Build.cs includes AIModule and NavigationSystem
  - [ ] Can open Unreal Editor without crashes

**COMMIT POINT 4.1.2**: `git add Source/FC/FC.Build.cs && git commit -m "build(overworld): Add AIModule and NavigationSystem to FC.Build.cs"`

---

#### Step 4.2: Create BP_FCOverworldPlayerController Blueprint

##### Step 4.2.1: Create Blueprint Derived from C++ Class

- [ ] **Analysis**

  - [ ] Check AFCPlayerController has BP_FC_PlayerController Blueprint counterpart
  - [ ] Follow same pattern: C++ base class with Blueprint for editor configuration
  - [ ] Confirm folder: `/Content/FC/Core/` (same as other controller Blueprints)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Core/`
  - [ ] Right-click → Blueprint Class → All Classes → search "FCOverworldPlayerController"
  - [ ] Select AFCOverworldPlayerController as parent class
  - [ ] Name: `BP_FCOverworldPlayerController`
  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults panel:
    - [ ] Verify InputManager component visible in Components list
  - [ ] Compile and save Blueprint

- [ ] **Testing After Step 4.2.1** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] InputManager component visible in hierarchy
  - [ ] Asset saved at correct path
  - [ ] Can place in level (test, then remove)

**COMMIT POINT 4.2.1**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(overworld): Create BP_FCOverworldPlayerController Blueprint"`

---

##### Step 4.2.2: Configure InputManager Component in Blueprint

- [ ] **Analysis**

  - [ ] Review BP_FC_PlayerController InputManager configuration from Week 2
  - [ ] InputManager needs IMC_FC_TopDown assigned to TopDownMappingContext property

- [ ] **Implementation (BP_FCOverworldPlayerController)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Select InputManager component in Components panel
  - [ ] Details panel → FC | Input | Contexts:
    - [ ] Set TopDownMappingContext: `/Game/FC/Input/Contexts/IMC_FC_TopDown`
    - [ ] Leave other contexts (FirstPerson, Fight, StaticScene) as None (not used in Overworld)
  - [ ] Details panel → FC | Input | Settings:
    - [ ] DefaultMappingPriority: 0 (default)
  - [ ] Compile and save

- [ ] **Testing After Step 4.2.2** ✅ CHECKPOINT
  - [ ] TopDownMappingContext assigned to IMC_FC_TopDown
  - [ ] Blueprint compiles without errors
  - [ ] No "None" warnings for TopDownMappingContext

**COMMIT POINT 4.2.2**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(overworld): Configure InputManager TopDown context in BP_FCOverworldPlayerController"`

---

#### Step 4.3: Assign BP_FCOverworldPlayerController to L_Overworld

##### Step 4.3.1: Set PlayerController Override in World Settings

- [ ] **Analysis**

  - [ ] Check L_Office World Settings for PlayerController configuration pattern
  - [ ] Confirm BP_FCOverworldPlayerController will be used only in L_Overworld

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] Window → World Settings
  - [ ] Game Mode section → Find "Player Controller Class"
  - [ ] Set Player Controller Class: BP_FCOverworldPlayerController
  - [ ] Save level

- [ ] **Testing After Step 4.3.1** ✅ CHECKPOINT
  - [ ] World Settings shows BP_FCOverworldPlayerController
  - [ ] Level saves without errors
  - [ ] PIE in L_Overworld spawns correct controller

**COMMIT POINT 4.3.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Assign BP_FCOverworldPlayerController to L_Overworld World Settings"`

---

#### Step 4.4: Place BP_OverworldCamera in L_Overworld

##### Step 4.4.1: Add Camera to Level and Position

- [ ] **Analysis**

  - [ ] Camera should be placed at CameraStart_Overworld marker location (from Task 2.4)
  - [ ] Camera will be auto-possessed by AFCOverworldPlayerController::BeginPlay()

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] Drag BP_OverworldCamera from Content Browser into viewport
  - [ ] Position at CameraStart_Overworld location (or manually set):
    - [ ] Location: X=-1000, Y=0, Z=1500
    - [ ] Rotation: Pitch=-70, Yaw=0, Roll=0
  - [ ] Rename instance to: `BP_OverworldCamera_Instance` (or leave as default "BP_OverworldCamera")
  - [ ] Details panel:
    - [ ] Auto Activate: True (ensure camera is active)
  - [ ] Save level

- [ ] **Testing After Step 4.4.1** ✅ CHECKPOINT
  - [ ] Camera visible in viewport at correct position
  - [ ] Camera looking down at PlayerStart location
  - [ ] Level saves without errors

**COMMIT POINT 4.4.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Place BP_OverworldCamera in L_Overworld level"`

---

#### Step 4.5: Test Input Context Switching

##### Step 4.5.1: Full Overworld Input Test

- [ ] **Analysis**

  - [ ] Test that TopDown input context activates correctly
  - [ ] Verify WASD pans camera and mouse wheel zooms
  - [ ] Check Output Log for InputManager and controller logs

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Check Output Log for:
    - [ ] "AFCOverworldPlayerController: Constructor executed"
    - [ ] "BeginPlay: Switched to TopDown input mode"
    - [ ] "BeginPlay: Possessed OverworldCamera: [camera name]"
    - [ ] UFCInputManager log: "SetInputMappingMode: TopDown"
  - [ ] Test WASD keys:
    - [ ] Press W → Camera pans forward (Y positive)
    - [ ] Press S → Camera pans backward (Y negative)
    - [ ] Press A → Camera pans left (X negative)
    - [ ] Press D → Camera pans right (X positive)
  - [ ] Test Mouse Wheel:
    - [ ] Scroll up → Camera zooms in (height decreases or spring arm shortens)
    - [ ] Scroll down → Camera zooms out (height increases or spring arm lengthens)
    - [ ] Verify zoom stops at min/max limits
  - [ ] Test ESC key (should open pause menu from Week 2):
    - [ ] Press ESC → Pause menu appears
    - [ ] Resume → Returns to Overworld
  - [ ] Document any issues in "Known Issues & Backlog"

- [ ] **Testing After Step 4.5.1** ✅ CHECKPOINT
  - [ ] TopDown input context active (logs confirm)
  - [ ] WASD pans camera smoothly
  - [ ] Mouse wheel zooms correctly with limits
  - [ ] ESC opens pause menu (if integrated)
  - [ ] No "Accessed None" errors
  - [ ] No input binding warnings

**COMMIT POINT 4.5.1**: `git add -A && git commit -m "test(overworld): Verify TopDown input context and camera controls in L_Overworld"`

---

### Task 4 Acceptance Criteria

- [ ] AFCOverworldPlayerController C++ class created and compiles successfully
- [ ] BP_FCOverworldPlayerController Blueprint created with InputManager component
- [ ] InputManager configured with IMC_FC_TopDown in TopDownMappingContext slot
- [ ] BP_FCOverworldPlayerController assigned to L_Overworld World Settings
- [ ] BP_OverworldCamera placed in L_Overworld and auto-possessed
- [ ] PIE in L_Overworld switches to TopDown input mode (confirmed in logs)
- [ ] WASD pans camera correctly
- [ ] Mouse wheel zooms camera with min/max limits
- [ ] No compilation errors or Blueprint errors
- [ ] All assets saved in correct folders

**Task 4 complete. Ready for Task 5 sub-tasks (Convoy Pawn & Click-to-Move)? Respond with 'Go' to continue.**

---

### Task 5: Convoy Pawn & Click-to-Move Pathfinding

**Purpose**: Create BP_OverworldConvoy pawn with left-click movement using NavMesh pathfinding via SimpleMoveToLocation().

---

#### Step 5.1: Create Input Action for Click-to-Move

##### Step 5.1.1: Create IA_OverworldClickMove Input Action

- [ ] **Analysis**

  - [ ] Review existing input actions (IA_Interact from Week 1 uses Boolean type)
  - [ ] Click-to-move needs Boolean trigger (press detection, not hold)
  - [ ] Will be bound to Left Mouse Button in IMC_FC_TopDown

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_OverworldClickMove`
  - [ ] Open IA_OverworldClickMove
  - [ ] Set Value Type: Digital (bool)
  - [ ] Save asset

- [ ] **Testing After Step 5.1.1** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Digital (bool)
  - [ ] Asset saves without errors

**COMMIT POINT 5.1.1**: `git add Content/FC/Input/Actions/IA_OverworldClickMove.uasset && git commit -m "feat(overworld): Create IA_OverworldClickMove input action"`

---

##### Step 5.1.2: Add Left Mouse Button Binding to IMC_FC_TopDown

- [ ] **Analysis**

  - [ ] Left Mouse Button key name in Enhanced Input: "LeftMouseButton"
  - [ ] No modifiers needed for simple click detection

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [ ] Add Mapping: IA_OverworldClickMove
  - [ ] Add Key: **Left Mouse Button**
    - [ ] No modifiers needed
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 5.1.2** ✅ CHECKPOINT
  - [ ] Left Mouse Button bound to IA_OverworldClickMove
  - [ ] IMC_FC_TopDown now has 3 mappings (Pan, Zoom, ClickMove)
  - [ ] Asset saves without errors

**COMMIT POINT 5.1.2**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Add left mouse button binding for click-to-move in IMC_FC_TopDown"`

---

#### Step 5.2: Create BP_OverworldConvoy Pawn Blueprint

##### Step 5.2.1: Create Pawn Blueprint with Components

- [ ] **Analysis**

  - [ ] Convoy needs collision (CapsuleComponent), visual mesh, and movement component
  - [ ] FloatingPawnMovement allows NavMesh pathfinding with SimpleMoveToLocation()
  - [ ] Confirm folder: `/Content/FC/World/Blueprints/Pawns/`

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/World/Blueprints/` (create Pawns subfolder if needed)
  - [ ] Right-click → Blueprint Class → Pawn
  - [ ] Name: `BP_OverworldConvoy`
  - [ ] Open BP_OverworldConvoy
  - [ ] Components Panel:
    - [ ] Root: CapsuleComponent (rename to "ConvoyCollision")
      - [ ] Set Capsule Half Height: 100
      - [ ] Set Capsule Radius: 50
    - [ ] Add Child: Static Mesh Component (rename to "ConvoyMesh")
      - [ ] Set Static Mesh: Choose placeholder (Cube, Cylinder, or starter content mesh)
      - [ ] Set Scale: X=1, Y=1, Z=2 (vertical orientation)
      - [ ] Set Material: Bright color for visibility (e.g., red/blue starter material)
    - [ ] Add: Floating Pawn Movement Component
      - [ ] Max Speed: 300.0 (walking speed)
      - [ ] Acceleration: 500.0
      - [ ] Deceleration: 1000.0
  - [ ] Class Defaults:
    - [ ] Auto Possess Player: Disabled (controller will possess manually)
  - [ ] Compile and save

- [ ] **Testing After Step 5.2.1** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] Components hierarchy correct (CapsuleComponent root, mesh child)
  - [ ] FloatingPawnMovement component added
  - [ ] Can place in level viewport (test, then remove)

**COMMIT POINT 5.2.1**: `git add Content/FC/World/Blueprints/Pawns/BP_OverworldConvoy.uasset && git commit -m "feat(overworld): Create BP_OverworldConvoy pawn with collision and movement components"`

---

#### Step 5.3: Implement Click-to-Move Logic in Controller

##### Step 5.3.1: Add Click-to-Move Handler to AFCOverworldPlayerController

- [ ] **Analysis**

  - [ ] Controller needs to handle IA_OverworldClickMove input
  - [ ] On click: Raycast from mouse position to world, move pawn to hit location
  - [ ] Use AI MoveTo functions: SimpleMoveToLocation() or AIController methods

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Open `FCOverworldPlayerController.h`
  - [ ] Add forward declarations:
    ```cpp
    class UInputAction;
    ```
  - [ ] Add private members:

    ```cpp
    private:
        /** Input action for click-to-move */
        UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
        TObjectPtr<UInputAction> ClickMoveAction;

        /** Handle click-to-move input */
        void HandleClickMove();
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Add includes:
    ```cpp
    #include "InputAction.h"
    #include "AI/NavigationSystemBase.h"
    #include "NavigationSystem.h"
    ```
  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        // Bind click-to-move action
        UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
        if (EnhancedInput && ClickMoveAction)
        {
            EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &AFCOverworldPlayerController::HandleClickMove);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound ClickMoveAction"));
        }
        else
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("SetupInputComponent: Failed to bind ClickMoveAction"));
        }
    }
    ```

  - [ ] Implement HandleClickMove():

    ```cpp
    void AFCOverworldPlayerController::HandleClickMove()
    {
        // Get mouse cursor hit result
        FHitResult HitResult;
        bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

        if (!bHit)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: No hit under cursor"));
            return;
        }

        // Get controlled pawn
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: No controlled pawn"));
            return;
        }

        // Use Simple Move To Location (requires FloatingPawnMovement or CharacterMovement)
        FVector TargetLocation = HitResult.Location;

        // Use Navigation System for pathfinding
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLocation;
            bool bFoundPath = NavSys->ProjectPointToNavigation(TargetLocation, NavLocation);

            if (bFoundPath)
            {
                // Move pawn using AI pathfinding
                UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NavLocation.Location);
                UE_LOG(LogFCOverworldController, Log, TEXT("HandleClickMove: Moving to %s"), *NavLocation.Location.ToString());
            }
            else
            {
                UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: Failed to project point to NavMesh"));
            }
        }
        else
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: Navigation System not found"));
        }
    }
    ```

  - [ ] Save file

- [ ] **Compilation**

  - [ ] Add missing include if needed:
    ```cpp
    #include "Blueprint/AIBlueprintHelperLibrary.h"
    ```
  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors

- [ ] **Testing After Step 5.3.1** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] No linker errors
  - [ ] HandleClickMove() method added correctly
  - [ ] Can open Unreal Editor

**COMMIT POINT 5.3.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(overworld): Implement click-to-move pathfinding in AFCOverworldPlayerController"`

---

##### Step 5.3.2: Configure ClickMoveAction in BP_FCOverworldPlayerController

- [ ] **Analysis**

  - [ ] ClickMoveAction property needs IA_OverworldClickMove assigned in Blueprint
  - [ ] Follow same pattern as other input actions in BP_FC_PlayerController

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults → FC | Input | Actions:
    - [ ] Set ClickMoveAction: `/Game/FC/Input/Actions/IA_OverworldClickMove`
  - [ ] Compile and save

- [ ] **Testing After Step 5.3.2** ✅ CHECKPOINT
  - [ ] ClickMoveAction assigned to IA_OverworldClickMove
  - [ ] Blueprint compiles without errors
  - [ ] No "None" warnings for ClickMoveAction

**COMMIT POINT 5.3.2**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(overworld): Assign IA_OverworldClickMove to ClickMoveAction in BP_FCOverworldPlayerController"`

---

#### Step 5.4: Place BP_OverworldConvoy in L_Overworld

##### Step 5.4.1: Add Convoy to Level and Set as Default Pawn

- [ ] **Analysis**

  - [ ] Convoy should spawn at PlayerStart location
  - [ ] Can either place manually or set as Default Pawn Class in World Settings

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] **Option A: Manual Placement**
    - [ ] Drag BP_OverworldConvoy from Content Browser into viewport
    - [ ] Position at PlayerStart location (X=0, Y=0, Z=100)
    - [ ] Set Auto Possess Player: Player 0
  - [ ] **Option B: Default Pawn Class (Recommended)**
    - [ ] Window → World Settings
    - [ ] Game Mode → Default Pawn Class: BP_OverworldConvoy
    - [ ] Pawn will spawn at PlayerStart automatically
  - [ ] Save level

- [ ] **Testing After Step 5.4.1** ✅ CHECKPOINT
  - [ ] Convoy visible in level at PlayerStart
  - [ ] PIE spawns convoy correctly
  - [ ] Controller possesses convoy (check with F8 to eject and see possession)
  - [ ] Level saves without errors

**COMMIT POINT 5.4.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Add BP_OverworldConvoy to L_Overworld as default pawn"`

---

#### Step 5.5: Test Click-to-Move Pathfinding

##### Step 5.5.1: Full Click-to-Move Verification

- [ ] **Analysis**

  - [ ] Test that left-click moves convoy to clicked location on ground
  - [ ] Verify pathfinding follows NavMesh (avoids obstacles if placed)
  - [ ] Check Output Log for movement logs

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify convoy spawns at PlayerStart
  - [ ] Verify camera is possessed (top-down view)
  - [ ] Press `P` to visualize NavMesh (green overlay should be visible)
  - [ ] Left-click on ground (green NavMesh area):
    - [ ] Convoy should move to clicked location
    - [ ] Check Output Log for: "HandleClickMove: Moving to [location]"
  - [ ] Test multiple clicks:
    - [ ] Click different locations → Convoy updates path and moves
    - [ ] Click far away → Convoy finds path across map
  - [ ] Test invalid clicks (outside NavMesh if possible):
    - [ ] Click outside NavMesh bounds → Should log warning "Failed to project point to NavMesh"
  - [ ] Test click on convoy itself:
    - [ ] Should either move to convoy location (no movement) or raycast through to ground
  - [ ] Verify movement speed:
    - [ ] Convoy moves at reasonable speed (Max Speed 300 from FloatingPawnMovement)
    - [ ] Movement is smooth (not jittery)

- [ ] **Testing After Step 5.5.1** ✅ CHECKPOINT
  - [ ] Left-click moves convoy successfully
  - [ ] Pathfinding uses NavMesh (follows green overlay)
  - [ ] Movement logs appear in Output Log
  - [ ] No "Accessed None" errors
  - [ ] Convoy reaches clicked destination
  - [ ] Camera remains independent (can pan/zoom while convoy moves)

**COMMIT POINT 5.5.1**: `git add -A && git commit -m "test(overworld): Verify click-to-move pathfinding with BP_OverworldConvoy"`

---

### Task 5 Acceptance Criteria

- [ ] IA_OverworldClickMove input action created (Digital/Boolean)
- [ ] Left Mouse Button bound to IA_OverworldClickMove in IMC_FC_TopDown
- [ ] BP_OverworldConvoy pawn created with CapsuleComponent, mesh, and FloatingPawnMovement
- [ ] AFCOverworldPlayerController implements HandleClickMove() with NavMesh pathfinding
- [ ] ClickMoveAction assigned to IA_OverworldClickMove in BP_FCOverworldPlayerController
- [ ] BP_OverworldConvoy placed in L_Overworld and set as default pawn
- [ ] Left-click on ground moves convoy to location using SimpleMoveToLocation()
- [ ] Convoy follows NavMesh pathfinding (visible with `P` key)
- [ ] Movement logs appear in Output Log
- [ ] No compilation errors or runtime crashes
- [ ] Camera controls (WASD/zoom) still work while convoy moves

**Task 5 complete. Ready for Task 6 sub-tasks (POI Actor & Interaction Stub)? Respond with 'Go' to continue.**

---

### Task 6: POI Actor & Right-Click Interaction Stub

**Purpose**: Create BP_OverworldPOI actor with right-click interaction that logs to console (stub for future implementation).

---

#### Step 6.1: Create Input Action for POI Interaction

##### Step 6.1.1: Create IA_OverworldInteractPOI Input Action

- [ ] **Analysis**

  - [ ] POI interaction uses right-click (separate from left-click move)
  - [ ] Boolean trigger (press detection)
  - [ ] Will be bound to Right Mouse Button in IMC_FC_TopDown

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_OverworldInteractPOI`
  - [ ] Open IA_OverworldInteractPOI
  - [ ] Set Value Type: Digital (bool)
  - [ ] Save asset

- [ ] **Testing After Step 6.1.1** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Digital (bool)
  - [ ] Asset saves without errors

**COMMIT POINT 6.1.1**: `git add Content/FC/Input/Actions/IA_OverworldInteractPOI.uasset && git commit -m "feat(overworld): Create IA_OverworldInteractPOI input action"`

---

##### Step 6.1.2: Add Right Mouse Button Binding to IMC_FC_TopDown

- [ ] **Analysis**

  - [ ] Right Mouse Button key name in Enhanced Input: "RightMouseButton"
  - [ ] No modifiers needed

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [ ] Add Mapping: IA_OverworldInteractPOI
  - [ ] Add Key: **Right Mouse Button**
    - [ ] No modifiers needed
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 6.1.2** ✅ CHECKPOINT
  - [ ] Right Mouse Button bound to IA_OverworldInteractPOI
  - [ ] IMC_FC_TopDown now has 4 mappings (Pan, Zoom, ClickMove, InteractPOI)
  - [ ] Asset saves without errors

**COMMIT POINT 6.1.2**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Add right mouse button binding for POI interaction in IMC_FC_TopDown"`

---

#### Step 6.2: Create BP_OverworldPOI Actor Blueprint

##### Step 6.2.1: Create Actor Blueprint with Mesh and Collision

- [ ] **Analysis**

  - [ ] POI needs collision for mouse raycast detection
  - [ ] Static mesh for visual representation
  - [ ] Box or sphere collision component
  - [ ] Confirm folder: `/Content/FC/World/Blueprints/Actors/`

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/World/Blueprints/` (create Actors subfolder if needed)
  - [ ] Right-click → Blueprint Class → Actor
  - [ ] Name: `BP_OverworldPOI`
  - [ ] Open BP_OverworldPOI
  - [ ] Components Panel:
    - [ ] Root: Scene Component (rename to "POIRoot")
    - [ ] Add Child: Static Mesh Component (rename to "POIMesh")
      - [ ] Set Static Mesh: Choose placeholder (Cone, Sphere, or starter content mesh)
      - [ ] Set Scale: X=2, Y=2, Z=2 (visible from camera)
      - [ ] Set Material: Distinctive color (e.g., yellow/orange for POI)
    - [ ] Add Child: Box Component (rename to "InteractionBox")
      - [ ] Set Box Extent: X=150, Y=150, Z=100 (larger than mesh for easier clicking)
      - [ ] Enable **Generate Overlap Events**: True
      - [ ] Set Collision Preset: **Custom**
        - [ ] Collision Enabled: Query Only (No Physics Collision)
        - [ ] Object Type: WorldDynamic
        - [ ] Collision Responses: Block All except Pawn (Overlap)
        - [ ] Visibility Channel: **Block** (important for raycast)
  - [ ] Compile and save

- [ ] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] Components hierarchy correct (Scene root, mesh and box children)
  - [ ] InteractionBox set to Block Visibility channel
  - [ ] Can place in level viewport (test, then remove)

**COMMIT POINT 6.2.1**: `git add Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Create BP_OverworldPOI actor with mesh and interaction collision"`

---

##### Step 6.2.2: Add Custom POI Name Property

- [ ] **Analysis**

  - [ ] Each POI should have a name for identification in logs and future UI
  - [ ] Editable per-instance property
  - [ ] String type

- [ ] **Implementation (BP_OverworldPOI Event Graph)**

  - [ ] Open BP_OverworldPOI
  - [ ] Variables Panel → Add Variable:
    - [ ] Name: `POIName`
    - [ ] Type: String
    - [ ] Instance Editable: ✅ Checked
    - [ ] Category: "FC|POI"
    - [ ] Default Value: "Unnamed POI"
    - [ ] Tooltip: "Display name for this Point of Interest"
  - [ ] Compile and save

- [ ] **Testing After Step 6.2.2** ✅ CHECKPOINT
  - [ ] POIName variable created
  - [ ] Instance Editable enabled
  - [ ] Default value set
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.2.2**: `git add Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Add POIName property to BP_OverworldPOI"`

---

#### Step 6.3: Implement POI Interaction Handler in Controller

##### Step 6.3.1: Add Interface for POI Actors (Optional but Recommended)

- [ ] **Analysis**

  - [ ] Interface allows controller to interact with any POI-like actor
  - [ ] Provides GetPOIName() method for future extensibility
  - [ ] Not strictly required for Week 3 stub, but good architecture

- [ ] **Implementation (Unreal Editor - Blueprint Interface)**

  - [ ] Content Browser → `/Game/FC/Core/` (or create Interfaces subfolder)
  - [ ] Right-click → Blueprints → Blueprint Interface
  - [ ] Name: `BPI_InteractablePOI`
  - [ ] Open BPI_InteractablePOI
  - [ ] Add Function: `OnPOIInteract`
    - [ ] No inputs/outputs (void function for stub)
  - [ ] Add Function: `GetPOIName`
    - [ ] Output: String (Return Value)
  - [ ] Compile and save

- [ ] **Implementation (BP_OverworldPOI - Implement Interface)**

  - [ ] Open BP_OverworldPOI
  - [ ] Class Settings → Interfaces → Add → BPI_InteractablePOI
  - [ ] Implement GetPOIName function:
    - [ ] Event Graph → Right-click → Add Event → Event GetPOIName
    - [ ] Connect POIName variable to Return Value
  - [ ] Implement OnPOIInteract function:
    - [ ] Event Graph → Right-click → Add Event → Event OnPOIInteract
    - [ ] Add Print String node:
      - [ ] In String: Append "POI Interaction Stub: " + POIName
      - [ ] Text Color: Yellow
      - [ ] Duration: 5.0
    - [ ] Add Log node (optional):
      - [ ] Use Add Custom Event or Blueprint → Print String
  - [ ] Compile and save

- [ ] **Testing After Step 6.3.1** ✅ CHECKPOINT
  - [ ] BPI_InteractablePOI interface created
  - [ ] BP_OverworldPOI implements interface
  - [ ] GetPOIName returns POIName variable
  - [ ] OnPOIInteract has Print String stub
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.3.1**: `git add Content/FC/Core/BPI_InteractablePOI.uasset Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Create BPI_InteractablePOI interface and implement in BP_OverworldPOI"`

---

##### Step 6.3.2: Add POI Interaction Handler to AFCOverworldPlayerController

- [ ] **Analysis**

  - [ ] Controller needs to handle IA_OverworldInteractPOI input
  - [ ] On right-click: Raycast from mouse position, check if hit actor implements interface
  - [ ] If valid POI: Call OnPOIInteract() interface method

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Open `FCOverworldPlayerController.h`
  - [ ] Add private members:

    ```cpp
    private:
        /** Input action for POI interaction */
        UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
        TObjectPtr<UInputAction> InteractPOIAction;

        /** Handle POI interaction input */
        void HandleInteractPOI();
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        // Existing click-to-move binding...

        // Bind POI interaction action
        if (EnhancedInput && InteractPOIAction)
        {
            EnhancedInput->BindAction(InteractPOIAction, ETriggerEvent::Started, this, &AFCOverworldPlayerController::HandleInteractPOI);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound InteractPOIAction"));
        }
        else
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("SetupInputComponent: Failed to bind InteractPOIAction"));
        }
    }
    ```

  - [ ] Implement HandleInteractPOI():

    ```cpp
    void AFCOverworldPlayerController::HandleInteractPOI()
    {
        // Get mouse cursor hit result
        FHitResult HitResult;
        bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

        if (!bHit)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleInteractPOI: No hit under cursor"));
            return;
        }

        // Check if hit actor implements POI interface
        AActor* HitActor = HitResult.GetActor();
        if (!HitActor)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleInteractPOI: No actor hit"));
            return;
        }

        // Check for interface (Blueprint interface check)
        if (HitActor->GetClass()->ImplementsInterface(UBPI_InteractablePOI::StaticClass()))
        {
            // Call interface method via Blueprint
            IBPI_InteractablePOI::Execute_OnPOIInteract(HitActor);

            // Get POI name for logging
            FString POIName = IBPI_InteractablePOI::Execute_GetPOIName(HitActor);
            UE_LOG(LogFCOverworldController, Log, TEXT("HandleInteractPOI: Interacted with POI '%s'"), *POIName);
        }
        else
        {
            UE_LOG(LogFCOverworldController, Log, TEXT("HandleInteractPOI: Hit actor '%s' is not a POI"), *HitActor->GetName());
        }
    }
    ```

  - [ ] Save file

- [ ] **Compilation**

  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors (may need to regenerate project files if BPI_InteractablePOI.h not found)

- [ ] **Testing After Step 6.3.2** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] No linker errors
  - [ ] HandleInteractPOI() method added correctly
  - [ ] Interface call syntax correct
  - [ ] Can open Unreal Editor

**COMMIT POINT 6.3.2**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(overworld): Implement POI interaction handler in AFCOverworldPlayerController"`

---

##### Step 6.3.3: Configure InteractPOIAction in BP_FCOverworldPlayerController

- [ ] **Analysis**

  - [ ] InteractPOIAction property needs IA_OverworldInteractPOI assigned in Blueprint
  - [ ] Follow same pattern as ClickMoveAction

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults → FC | Input | Actions:
    - [ ] Set InteractPOIAction: `/Game/FC/Input/Actions/IA_OverworldInteractPOI`
  - [ ] Compile and save

- [ ] **Testing After Step 6.3.3** ✅ CHECKPOINT
  - [ ] InteractPOIAction assigned to IA_OverworldInteractPOI
  - [ ] Blueprint compiles without errors
  - [ ] No "None" warnings for InteractPOIAction

**COMMIT POINT 6.3.3**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(overworld): Assign IA_OverworldInteractPOI to InteractPOIAction in BP_FCOverworldPlayerController"`

---

#### Step 6.4: Place POI Actors in L_Overworld

##### Step 6.4.1: Add Multiple POI Instances to Level

- [ ] **Analysis**

  - [ ] Place 3-5 POI actors in different locations for testing
  - [ ] Set unique names for each POI instance
  - [ ] Position on ground (Z=0 or on terrain if elevated)

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
  - [ ] (Optional) Add more POIs with unique names and positions
  - [ ] Save level

- [ ] **Testing After Step 6.4.1** ✅ CHECKPOINT
  - [ ] Multiple POI actors visible in level
  - [ ] Each POI has unique POIName value
  - [ ] POIs positioned on ground (not floating)
  - [ ] Level saves without errors

**COMMIT POINT 6.4.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Place multiple BP_OverworldPOI instances in L_Overworld"`

---

#### Step 6.5: Test POI Right-Click Interaction

##### Step 6.5.1: Full POI Interaction Verification

- [ ] **Analysis**

  - [ ] Test that right-click on POI triggers interaction stub
  - [ ] Verify Print String message appears on screen
  - [ ] Check Output Log for interaction logs
  - [ ] Test right-click on ground (should not interact)

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify convoy spawns at PlayerStart
  - [ ] Verify camera is possessed (top-down view)
  - [ ] Verify POI actors are visible (yellow/orange meshes)
  - [ ] **Test Right-Click on POI**:
    - [ ] Right-click directly on POI mesh (e.g., "Northern Village")
    - [ ] On-screen message should appear: "POI Interaction Stub: Northern Village" (yellow text, 5 seconds)
    - [ ] Check Output Log for: "HandleInteractPOI: Interacted with POI 'Northern Village'"
  - [ ] **Test Multiple POIs**:
    - [ ] Right-click on "Eastern Outpost" → Should show "POI Interaction Stub: Eastern Outpost"
    - [ ] Right-click on "Western Ruins" → Should show correct name
    - [ ] Verify each POI displays its unique name
  - [ ] **Test Right-Click on Non-POI**:
    - [ ] Right-click on ground (not on POI)
    - [ ] Should NOT show POI interaction message
    - [ ] Check Output Log for: "HandleInteractPOI: Hit actor '[ActorName]' is not a POI" or "No hit under cursor"
  - [ ] **Test Interaction Box Range**:
    - [ ] Right-click near edge of POI (within InteractionBox range)
    - [ ] Should still trigger interaction (box is larger than mesh)
  - [ ] **Test While Convoy Moving**:
    - [ ] Left-click to move convoy
    - [ ] While convoy is moving, right-click on POI
    - [ ] POI interaction should work independently of movement

- [ ] **Testing After Step 6.5.1** ✅ CHECKPOINT
  - [ ] Right-click on POI shows on-screen message with correct name
  - [ ] Interaction logs appear in Output Log
  - [ ] Each POI instance displays unique name
  - [ ] Right-click on ground does NOT trigger POI interaction
  - [ ] No "Accessed None" errors
  - [ ] Interaction works while convoy is moving
  - [ ] Interface method calls successful

**COMMIT POINT 6.5.1**: `git add -A && git commit -m "test(overworld): Verify POI right-click interaction stub"`

---

### Task 6 Acceptance Criteria

- [ ] IA_OverworldInteractPOI input action created (Digital/Boolean)
- [ ] Right Mouse Button bound to IA_OverworldInteractPOI in IMC_FC_TopDown
- [ ] BP_OverworldPOI actor created with mesh, InteractionBox (Block Visibility), and POIName property
- [ ] BPI_InteractablePOI interface created with OnPOIInteract() and GetPOIName() methods
- [ ] BP_OverworldPOI implements BPI_InteractablePOI interface
- [ ] OnPOIInteract() displays Print String message with POI name (stub)
- [ ] AFCOverworldPlayerController implements HandleInteractPOI() with raycast and interface check
- [ ] InteractPOIAction assigned to IA_OverworldInteractPOI in BP_FCOverworldPlayerController
- [ ] 3-5 POI instances placed in L_Overworld with unique names
- [ ] Right-click on POI shows on-screen message and logs interaction
- [ ] Right-click on non-POI actors/ground does not trigger POI interaction
- [ ] No compilation errors or runtime crashes
- [ ] POI interaction works independently of convoy movement

**Task 6 complete. Ready for Task 7 sub-tasks (Office-to-Overworld Transition)? Respond with 'Go' to continue.**

---

### Task 7: Office-to-Overworld Level Transition

**Purpose**: Integrate L_Overworld into existing level transition system so players can travel from L_Office to L_Overworld and back.

---

#### Step 7.1: Review Existing Transition System

##### Step 7.1.1: Analyze Week 2 Transition Architecture

- [ ] **Analysis**

  - [ ] Review UFCLevelManager transition methods (documentation/code)
  - [ ] Review UFCGameStateManager state changes (Office, Overworld_Travel)
  - [ ] Check if L_Office has existing transition trigger (table object or widget button)
  - [ ] Identify pattern: State Change → Level Transition → Input Context Switch
  - [ ] Review files:
    - [ ] `Source/FC/Managers/FCLevelManager.h/.cpp` (TransitionToLevel method)
    - [ ] `Source/FC/Managers/FCGameStateManager.h/.cpp` (SetGameState method)
    - [ ] `Content/FC/Office/BP_TableObject_Glass.uasset` (example interaction trigger)
    - [ ] `Content/FC/UI/WBP_MapTable.uasset` (widget with potential "Start Journey" button)

- [ ] **Expected Architecture Pattern**

  - [ ] Week 2 established:
    - [ ] Table interaction → OnTableObjectClicked event → Opens WBP_MapTable widget
    - [ ] Widget button → Triggers transition logic
    - [ ] Transition logic needs:
      1. UFCGameStateManager::SetGameState(Overworld_Travel)
      2. UFCLevelManager::TransitionToLevel("L_Overworld")
      3. UFCInputManager switches context to TopDown mode (may be automatic via GameState)

- [ ] **Testing After Step 7.1.1** ✅ CHECKPOINT
  - [ ] Reviewed LevelManager TransitionToLevel() method
  - [ ] Reviewed GameStateManager SetGameState() method
  - [ ] Identified trigger point in L_Office (table/widget)
  - [ ] Confirmed state flow: Office → Overworld_Travel → Level Transition

**COMMIT POINT 7.1.1**: N/A (analysis only, no code changes)

---

#### Step 7.2: Add Transition Trigger in L_Office

##### Step 7.2.1: Add "Start Journey" Button to WBP_MapTable Widget

- [ ] **Analysis**

  - [ ] WBP_MapTable widget opens when interacting with map table in L_Office
  - [ ] Add button to initiate overworld transition
  - [ ] Button should call transition logic

- [ ] **Implementation (Unreal Editor - Widget Blueprint)**

  - [ ] Open `/Game/FC/UI/WBP_MapTable`
  - [ ] Designer Tab:
    - [ ] Add Widget: Button (rename to "Btn_StartJourney")
    - [ ] Position: Bottom center or appropriate location
    - [ ] Add Text Block as child: "Start Journey" or "Begin Expedition"
    - [ ] Set button style (color, hover effects)
  - [ ] Graph Tab:
    - [ ] Select Btn_StartJourney → Add Event → OnClicked
    - [ ] In OnClicked event:
      - [ ] **Option A: Blueprint Implementation (Quick for Week 3)**
        - [ ] Get Game Instance → Cast to BP_FCGameInstance
        - [ ] Get LevelManager from Game Instance
        - [ ] Call TransitionToLevel with Level Name: "L_Overworld"
        - [ ] Get GameStateManager from Game Instance
        - [ ] Call SetGameState with State: Overworld_Travel
        - [ ] Remove Widget from Parent (close map widget)
      - [ ] **Option B: Custom Event (Better Architecture)**
        - [ ] Create Custom Event: "OnStartJourneyClicked"
        - [ ] Call Custom Event in OnClicked
        - [ ] Implement transition logic in Level Blueprint or PlayerController
  - [ ] Compile and save widget

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

- [ ] **Testing After Step 7.2.1** ✅ CHECKPOINT
  - [ ] "Start Journey" button visible in WBP_MapTable widget
  - [ ] Button positioned correctly
  - [ ] Widget compiles without errors
  - [ ] OnClicked event graph connected

**COMMIT POINT 7.2.1**: `git add Content/FC/UI/WBP_MapTable.uasset && git commit -m "feat(transition): Add Start Journey button to WBP_MapTable widget"`

---

##### Step 7.2.2: Implement Transition Logic in Widget or Controller

- [ ] **Analysis**

  - [ ] Decide implementation location:
    - [ ] Widget Blueprint (simpler for Week 3)
    - [ ] PlayerController (better architecture)
  - [ ] For Week 3: Widget Blueprint is acceptable

- [ ] **Implementation (WBP_MapTable Widget Graph)**

  - [ ] Open WBP_MapTable → Event Graph
  - [ ] Locate Btn_StartJourney OnClicked event
  - [ ] **Add Transition Nodes**:
    - [ ] Get Game Instance
    - [ ] Cast to BP_FCGameInstance (or GI_FCGameInstance if Blueprint-based)
    - [ ] From cast result:
      - [ ] Get GameStateManager → Call SetGameState → Input: Overworld_Travel (enum)
      - [ ] Get LevelManager → Call TransitionToLevel → Input: "L_Overworld" (Name)
    - [ ] Remove from Parent (close widget before transition)
  - [ ] Add Print String for debugging:
    - [ ] "Transitioning to Overworld..." (before transition)
  - [ ] Compile and save

- [ ] **Alternative Implementation (if C++ needed)**

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

  - [ ] Call TransitionToOverworld() from widget button

- [ ] **Testing After Step 7.2.2** ✅ CHECKPOINT
  - [ ] Transition logic implemented in widget or controller
  - [ ] GetGameInstance cast nodes correct
  - [ ] SetGameState and TransitionToLevel calls correct
  - [ ] Widget compiles without errors

**COMMIT POINT 7.2.2**: `git add Content/FC/UI/WBP_MapTable.uasset && git commit -m "feat(transition): Implement Office-to-Overworld transition logic in WBP_MapTable"`

---

#### Step 7.3: Add Return Transition from Overworld to Office

##### Step 7.3.1: Add UI Button or Debug Key for Return Transition

- [ ] **Analysis**

  - [ ] Week 3 needs basic return mechanism (not full travel system)
  - [ ] Options:
    - [ ] ESC key opens pause menu with "Return to Office" button
    - [ ] Debug key (Tab or M) for quick return
    - [ ] On-screen UI button
  - [ ] For Week 3: Debug key is fastest (Task 8 will add proper pause menu)

- [ ] **Implementation (AFCOverworldPlayerController)**

  - [ ] Open `FCOverworldPlayerController.h`
  - [ ] Add method declaration:
    ```cpp
    private:
        /** Debug: Return to office level */
        void DebugReturnToOffice();
    ```
  - [ ] Open `FCOverworldPlayerController.cpp`
  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        // Existing bindings...

        // Debug: Return to office (Tab key)
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AFCOverworldPlayerController::DebugReturnToOffice);
        UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound Tab key to DebugReturnToOffice"));
    }
    ```

  - [ ] Implement DebugReturnToOffice():

    ```cpp
    void AFCOverworldPlayerController::DebugReturnToOffice()
    {
        UE_LOG(LogFCOverworldController, Log, TEXT("DebugReturnToOffice: Returning to L_Office"));

        UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
        if (!GameInstance)
        {
            UE_LOG(LogFCOverworldController, Error, TEXT("DebugReturnToOffice: Failed to get GameInstance"));
            return;
        }

        // Set game state back to Office
        UFCGameStateManager* GameStateMgr = GameInstance->GetGameStateManager();
        if (GameStateMgr)
        {
            GameStateMgr->SetGameState(EFCGameState::Office);
            UE_LOG(LogFCOverworldController, Log, TEXT("DebugReturnToOffice: Set game state to Office"));
        }

        // Transition to Office level
        UFCLevelManager* LevelMgr = GameInstance->GetLevelManager();
        if (LevelMgr)
        {
            LevelMgr->TransitionToLevel(FName("L_Office"));
            UE_LOG(LogFCOverworldController, Log, TEXT("DebugReturnToOffice: Transitioning to L_Office"));
        }
    }
    ```

  - [ ] Save files

- [ ] **Compilation**

  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors
  - [ ] Check UFCGameInstance.h has GetGameStateManager() and GetLevelManager() methods

- [ ] **Testing After Step 7.3.1** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] DebugReturnToOffice() method added
  - [ ] Tab key binding in SetupInputComponent
  - [ ] Can open Unreal Editor

**COMMIT POINT 7.3.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(transition): Add Tab key debug transition from Overworld to Office"`

---

#### Step 7.4: Verify Input Context Switches with Level Transitions

##### Step 7.4.1: Ensure UFCInputManager Responds to GameState Changes

- [ ] **Analysis**

  - [ ] Week 2 Priority 6 established UFCInputManager with mode switching
  - [ ] Confirm GameStateManager state changes trigger InputManager context switch
  - [ ] Expected flow:
    - [ ] GameState → Office: InputManager switches to Office context
    - [ ] GameState → Overworld_Travel: InputManager switches to TopDown context

- [ ] **Review Implementation**

  - [ ] Check if UFCInputManager has OnGameStateChanged() delegate binding
  - [ ] If not implemented yet, add in BeginPlay():

    ```cpp
    // FCInputManager.cpp - BeginPlay()
    void UFCInputManager::BeginPlay()
    {
        Super::BeginPlay();

        // Subscribe to game state changes
        UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetWorld()->GetGameInstance());
        if (GameInstance)
        {
            UFCGameStateManager* GameStateMgr = GameInstance->GetGameStateManager();
            if (GameStateMgr)
            {
                GameStateMgr->OnGameStateChanged.AddDynamic(this, &UFCInputManager::OnGameStateChanged);
                UE_LOG(LogFCInputManager, Log, TEXT("BeginPlay: Subscribed to OnGameStateChanged"));
            }
        }
    }

    void UFCInputManager::OnGameStateChanged(EFCGameState NewState)
    {
        switch (NewState)
        {
            case EFCGameState::Office:
                SetMappingMode(EFCMappingMode::Office);
                break;
            case EFCGameState::Overworld_Travel:
                SetMappingMode(EFCMappingMode::TopDown);
                break;
            default:
                UE_LOG(LogFCInputManager, Warning, TEXT("OnGameStateChanged: Unhandled game state"));
                break;
        }
    }
    ```

- [ ] **Implementation (if needed)**

  - [ ] Open `FCInputManager.h` and `FCInputManager.cpp`
  - [ ] Add OnGameStateChanged method if missing
  - [ ] Add delegate binding in BeginPlay if missing
  - [ ] Compile and test

- [ ] **Testing After Step 7.4.1** ✅ CHECKPOINT
  - [ ] UFCInputManager responds to GameState changes
  - [ ] OnGameStateChanged delegate binding exists
  - [ ] SetMappingMode() called on state transitions
  - [ ] Compilation successful

**COMMIT POINT 7.4.1**: `git add Source/FC/Components/FCInputManager.h Source/FC/Components/FCInputManager.cpp && git commit -m "feat(transition): Ensure UFCInputManager responds to GameState changes for context switching"`

---

#### Step 7.5: Test Full Transition Flow

##### Step 7.5.1: Test Office → Overworld Transition

- [ ] **Test Sequence**

  - [ ] Open L_Office in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify Office environment loads
  - [ ] Verify player spawns at PlayerStart
  - [ ] Navigate to map table
  - [ ] Interact with map table (click on BP_TableObject_Glass or similar)
  - [ ] Verify WBP_MapTable widget opens
  - [ ] Click "Start Journey" button
  - [ ] **Expected Results**:
    - [ ] Print String shows "Transitioning to Overworld..." (if added)
    - [ ] Level transitions to L_Overworld
    - [ ] Overworld level loads (terrain, camera, convoy, POIs visible)
    - [ ] Camera is top-down (controlled by BP_OverworldCamera)
    - [ ] Convoy pawn spawns at PlayerStart
    - [ ] Input context switches to TopDown (WASD pan, mouse wheel zoom work)
    - [ ] Check Output Log for:
      - [ ] "SetGameState: State changed to Overworld_Travel"
      - [ ] "TransitionToLevel: Transitioning to L_Overworld"
      - [ ] "SetMappingMode: Switched to TopDown"

- [ ] **Testing After Step 7.5.1** ✅ CHECKPOINT
  - [ ] Office → Overworld transition works
  - [ ] Level loads correctly
  - [ ] Input context switches correctly
  - [ ] No "Accessed None" errors
  - [ ] Camera and convoy spawn correctly

**COMMIT POINT 7.5.1**: N/A (testing only, no code changes)

---

##### Step 7.5.2: Test Overworld → Office Return Transition

- [ ] **Test Sequence**

  - [ ] Continue from previous test (in L_Overworld)
  - [ ] Press **Tab** key
  - [ ] **Expected Results**:
    - [ ] Level transitions to L_Office
    - [ ] Office level loads (desk, table, objects visible)
    - [ ] Player spawns at PlayerStart in Office
    - [ ] Input context switches to Office (first-person camera, WASD movement work)
    - [ ] Check Output Log for:
      - [ ] "DebugReturnToOffice: Returning to L_Office"
      - [ ] "SetGameState: State changed to Office"
      - [ ] "TransitionToLevel: Transitioning to L_Office"
      - [ ] "SetMappingMode: Switched to Office"

- [ ] **Testing After Step 7.5.2** ✅ CHECKPOINT
  - [ ] Overworld → Office transition works
  - [ ] Level loads correctly
  - [ ] Input context switches correctly
  - [ ] Player can interact with Office objects again
  - [ ] No persistent state issues (widgets closed, controls correct)

**COMMIT POINT 7.5.2**: N/A (testing only, no code changes)

---

##### Step 7.5.3: Test Round-Trip Transitions

- [ ] **Test Sequence**

  - [ ] Continue from previous test (in L_Office)
  - [ ] Perform Office → Overworld transition again (via map widget)
  - [ ] Verify overworld loads correctly second time
  - [ ] Perform Overworld → Office transition again (Tab key)
  - [ ] Verify office loads correctly second time
  - [ ] Repeat 2-3 times
  - [ ] **Expected Results**:
    - [ ] Transitions work consistently every time
    - [ ] No memory leaks or performance degradation
    - [ ] Input contexts always switch correctly
    - [ ] No "stale" widgets or input bindings from previous level

- [ ] **Testing After Step 7.5.3** ✅ CHECKPOINT
  - [ ] Multiple round-trip transitions work
  - [ ] No crashes or errors after multiple transitions
  - [ ] Input always correct for current level
  - [ ] Game state always correct for current level

**COMMIT POINT 7.5.3**: `git add -A && git commit -m "test(transition): Verify full Office ↔ Overworld transition flow"`

---

### Task 7 Acceptance Criteria

- [ ] WBP_MapTable widget has "Start Journey" button
- [ ] Button click triggers GameState change to Overworld_Travel and level transition to L_Overworld
- [ ] Tab key in L_Overworld returns player to L_Office (debug functionality)
- [ ] UFCInputManager automatically switches input contexts on GameState changes:
  - [ ] Office state → Office context (first-person)
  - [ ] Overworld_Travel state → TopDown context (camera controls)
- [ ] Office → Overworld transition works correctly:
  - [ ] Level loads with terrain, camera, convoy, POIs
  - [ ] Input context switches to TopDown
  - [ ] Player can pan/zoom camera, click-to-move convoy, interact with POIs
- [ ] Overworld → Office transition works correctly:
  - [ ] Level loads with desk, table, objects
  - [ ] Input context switches to Office
  - [ ] Player can navigate office and interact with objects
- [ ] Round-trip transitions (Office → Overworld → Office → Overworld) work consistently
- [ ] No crashes, memory leaks, or "Accessed None" errors during transitions
- [ ] Output Log shows correct state changes and level transitions
- [ ] No compilation errors

**Task 7 complete. Ready for Task 8 sub-tasks (Conditional Engine Pause)? Respond with 'Go' to continue.**

---

### Task 8: Conditional Engine Pause for Overworld

**Purpose**: Implement ESC key pause/unpause in Overworld only (not in Office), pausing physics and convoy movement but keeping UI interactive.

---

#### Step 8.1: Create Input Action for Pause

##### Step 8.1.1: Create IA_Pause Input Action

- [ ] **Analysis**

  - [ ] Pause action should be available in all contexts (Office and TopDown)
  - [ ] Boolean trigger for toggle behavior
  - [ ] ESC key is standard for pause menus

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_Pause`
  - [ ] Open IA_Pause
  - [ ] Set Value Type: Digital (bool)
  - [ ] Save asset

- [ ] **Testing After Step 8.1.1** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Digital (bool)
  - [ ] Asset saves without errors

**COMMIT POINT 8.1.1**: `git add Content/FC/Input/Actions/IA_Pause.uasset && git commit -m "feat(pause): Create IA_Pause input action"`

---

##### Step 8.1.2: Add ESC Key Binding to IMC_FC_TopDown

- [ ] **Analysis**

  - [ ] ESC key binding in TopDown context for Overworld pause
  - [ ] Will NOT add to IMC_FC_Office (Office doesn't need pause per DRM)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [ ] Add Mapping: IA_Pause
  - [ ] Add Key: **Escape**
    - [ ] No modifiers needed
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 8.1.2** ✅ CHECKPOINT
  - [ ] ESC key bound to IA_Pause in IMC_FC_TopDown
  - [ ] IMC_FC_TopDown now has 5 mappings (Pan, Zoom, ClickMove, InteractPOI, Pause)
  - [ ] Asset saves without errors

**COMMIT POINT 8.1.2**: `git add Content/FC/Input/Contexts/IMC_FC_TopDown.uasset && git commit -m "feat(pause): Add ESC key binding for pause in IMC_FC_TopDown"`

---

#### Step 8.2: Create Pause Menu Widget

##### Step 8.2.1: Create WBP_PauseMenu Widget

- [ ] **Analysis**

  - [ ] Simple pause menu with resume and return to office options
  - [ ] Displayed when game is paused in Overworld
  - [ ] Should be visually distinct from gameplay

- [ ] **Implementation (Unreal Editor - Widget Blueprint)**

  - [ ] Content Browser → `/Game/FC/UI/`
  - [ ] Right-click → User Interface → Widget Blueprint
  - [ ] Name: `WBP_PauseMenu`
  - [ ] Open WBP_PauseMenu
  - [ ] Designer Tab:
    - [ ] Add Canvas Panel (root)
    - [ ] Add Overlay or Vertical Box (centered):
      - [ ] Add Border or Image (semi-transparent black background)
        - [ ] Anchors: Fill Screen
        - [ ] Color: Black with alpha 0.7 (R=0, G=0, B=0, A=0.7)
      - [ ] Add Vertical Box (centered content):
        - [ ] Anchors: Center
        - [ ] Alignment: Center, Middle
        - [ ] Add Text Block: "PAUSED" (large, bold)
        - [ ] Add Spacer (height 20)
        - [ ] Add Button: "Resume" (rename to Btn_Resume)
          - [ ] Add Text child: "Resume (ESC)"
        - [ ] Add Spacer (height 10)
        - [ ] Add Button: "Return to Office" (rename to Btn_ReturnToOffice)
          - [ ] Add Text child: "Return to Office (Tab)"
        - [ ] (Optional) Add Spacer and Exit button for later
  - [ ] Save widget

- [ ] **Testing After Step 8.2.1** ✅ CHECKPOINT
  - [ ] WBP_PauseMenu widget created
  - [ ] Layout includes "PAUSED" text and two buttons
  - [ ] Semi-transparent background covers screen
  - [ ] Widget compiles without errors

**COMMIT POINT 8.2.1**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Create WBP_PauseMenu widget with Resume and Return buttons"`

---

##### Step 8.2.2: Implement Resume Button Logic

- [ ] **Analysis**

  - [ ] Resume button should unpause game and close widget
  - [ ] Call same unpause logic that ESC key will trigger

- [ ] **Implementation (WBP_PauseMenu Event Graph)**

  - [ ] Open WBP_PauseMenu → Event Graph
  - [ ] Select Btn_Resume → Add Event → OnClicked
  - [ ] In OnClicked event:
    - [ ] Create Custom Event: "RequestResume"
    - [ ] Promote to Event Dispatcher (optional for controller communication)
    - [ ] OR: Get Player Controller → Cast to BP_FCOverworldPlayerController → Call UnpauseGame() method (to be created)
    - [ ] Remove from Parent (close widget)
  - [ ] For Week 3: Direct controller call is acceptable
  - [ ] Compile and save

- [ ] **Testing After Step 8.2.2** ✅ CHECKPOINT
  - [ ] Btn_Resume OnClicked event created
  - [ ] Resume logic connected (will implement controller method next)
  - [ ] Widget compiles without errors

**COMMIT POINT 8.2.2**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Implement Resume button logic in WBP_PauseMenu"`

---

##### Step 8.2.3: Implement Return to Office Button Logic

- [ ] **Analysis**

  - [ ] Return button should unpause game and transition to L_Office
  - [ ] Reuse DebugReturnToOffice() logic from Task 7

- [ ] **Implementation (WBP_PauseMenu Event Graph)**

  - [ ] Open WBP_PauseMenu → Event Graph
  - [ ] Select Btn_ReturnToOffice → Add Event → OnClicked
  - [ ] In OnClicked event:
    - [ ] Get Player Controller → Cast to BP_FCOverworldPlayerController
    - [ ] Call DebugReturnToOffice() method (or new method)
    - [ ] Unpause game before transition (SetGamePaused false)
    - [ ] Remove from Parent (close widget)
  - [ ] Compile and save

- [ ] **Testing After Step 8.2.3** ✅ CHECKPOINT
  - [ ] Btn_ReturnToOffice OnClicked event created
  - [ ] Return to Office logic connected
  - [ ] Widget compiles without errors

**COMMIT POINT 8.2.3**: `git add Content/FC/UI/WBP_PauseMenu.uasset && git commit -m "feat(pause): Implement Return to Office button logic in WBP_PauseMenu"`

---

#### Step 8.3: Implement Pause Logic in Overworld Controller

##### Step 8.3.1: Add Pause Handling to AFCOverworldPlayerController

- [ ] **Analysis**

  - [ ] Controller needs pause/unpause toggle method
  - [ ] Pause should:
    - [ ] Call SetGamePaused(true) to stop physics/movement
    - [ ] Show WBP_PauseMenu widget
    - [ ] Set Input Mode to UI Only (for button clicks)
    - [ ] Show mouse cursor
  - [ ] Unpause should reverse these actions

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Open `FCOverworldPlayerController.h`
  - [ ] Add forward declaration:
    ```cpp
    class UUserWidget;
    ```
  - [ ] Add private members:

    ```cpp
    private:
        /** Input action for pause */
        UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
        TObjectPtr<UInputAction> PauseAction;

        /** Pause menu widget class */
        UPROPERTY(EditDefaultsOnly, Category = "FC|UI")
        TSubclassOf<UUserWidget> PauseMenuClass;

        /** Current pause menu widget instance */
        UPROPERTY()
        TObjectPtr<UUserWidget> PauseMenuInstance;

        /** Is game currently paused? */
        bool bIsPaused;

        /** Handle pause input */
        void HandlePause();

        /** Pause the game */
        UFUNCTION(BlueprintCallable, Category = "FC|Pause")
        void PauseGame();

        /** Unpause the game */
        UFUNCTION(BlueprintCallable, Category = "FC|Pause")
        void UnpauseGame();
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Add includes:
    ```cpp
    #include "Blueprint/UserWidget.h"
    ```
  - [ ] Update constructor to initialize bIsPaused:
    ```cpp
    AFCOverworldPlayerController::AFCOverworldPlayerController()
    {
        bIsPaused = false;
        PauseMenuInstance = nullptr;
    }
    ```
  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        // Existing bindings...

        // Bind pause action
        UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
        if (EnhancedInput && PauseAction)
        {
            EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AFCOverworldPlayerController::HandlePause);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound PauseAction"));
        }
    }
    ```

  - [ ] Implement HandlePause():
    ```cpp
    void AFCOverworldPlayerController::HandlePause()
    {
        if (bIsPaused)
        {
            UnpauseGame();
        }
        else
        {
            PauseGame();
        }
    }
    ```
  - [ ] Implement PauseGame():

    ```cpp
    void AFCOverworldPlayerController::PauseGame()
    {
        if (bIsPaused)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("PauseGame: Already paused"));
            return;
        }

        // Pause game (stops physics, AI, timers)
        SetPause(true);
        bIsPaused = true;

        // Show pause menu widget
        if (PauseMenuClass)
        {
            PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
            if (PauseMenuInstance)
            {
                PauseMenuInstance->AddToViewport(100); // High Z-order

                // Set input mode to UI only
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
                SetInputMode(InputMode);

                // Show mouse cursor
                bShowMouseCursor = true;

                UE_LOG(LogFCOverworldController, Log, TEXT("PauseGame: Game paused, menu shown"));
            }
        }
        else
        {
            UE_LOG(LogFCOverworldController, Error, TEXT("PauseGame: PauseMenuClass not set"));
        }
    }
    ```

  - [ ] Implement UnpauseGame():

    ```cpp
    void AFCOverworldPlayerController::UnpauseGame()
    {
        if (!bIsPaused)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("UnpauseGame: Not paused"));
            return;
        }

        // Remove pause menu widget
        if (PauseMenuInstance)
        {
            PauseMenuInstance->RemoveFromParent();
            PauseMenuInstance = nullptr;
        }

        // Unpause game
        SetPause(false);
        bIsPaused = false;

        // Restore game input mode
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);

        // Keep mouse cursor visible (needed for top-down gameplay)
        bShowMouseCursor = true;

        UE_LOG(LogFCOverworldController, Log, TEXT("UnpauseGame: Game unpaused"));
    }
    ```

  - [ ] Save file

- [ ] **Compilation**

  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors

- [ ] **Testing After Step 8.3.1** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] PauseGame() and UnpauseGame() methods added
  - [ ] HandlePause() toggle logic correct
  - [ ] Can open Unreal Editor

**COMMIT POINT 8.3.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(pause): Implement pause/unpause logic in AFCOverworldPlayerController"`

---

##### Step 8.3.2: Configure Pause Action and Menu Class in BP_FCOverworldPlayerController

- [ ] **Analysis**

  - [ ] PauseAction needs IA_Pause assigned
  - [ ] PauseMenuClass needs WBP_PauseMenu assigned

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults → FC | Input | Actions:
    - [ ] Set PauseAction: `/Game/FC/Input/Actions/IA_Pause`
  - [ ] Class Defaults → FC | UI:
    - [ ] Set PauseMenuClass: `/Game/FC/UI/WBP_PauseMenu`
  - [ ] Compile and save

- [ ] **Testing After Step 8.3.2** ✅ CHECKPOINT
  - [ ] PauseAction assigned to IA_Pause
  - [ ] PauseMenuClass assigned to WBP_PauseMenu
  - [ ] Blueprint compiles without errors
  - [ ] No "None" warnings

**COMMIT POINT 8.3.2**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(pause): Assign IA_Pause and WBP_PauseMenu in BP_FCOverworldPlayerController"`

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
    - [ ] Cast to BP_FCOverworldPlayerController
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
    - [ ] Cast to BP_FCOverworldPlayerController
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
    - [ ] ESC key does nothing (IA_Pause not in IMC_FC_Office)
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

- [ ] IA_Pause input action created (Digital/Boolean)
- [ ] ESC key bound to IA_Pause in IMC_FC_TopDown only (not in IMC_FC_Office)
- [ ] WBP_PauseMenu widget created with semi-transparent background, "PAUSED" text, Resume button, and Return to Office button
- [ ] AFCOverworldPlayerController implements PauseGame() and UnpauseGame() methods
- [ ] PauseGame() calls SetPause(true), shows WBP_PauseMenu, sets input mode to UI Only, shows cursor
- [ ] UnpauseGame() calls SetPause(false), removes menu, restores game input mode
- [ ] HandlePause() toggles between paused/unpaused states on ESC press
- [ ] PauseAction and PauseMenuClass configured in BP_FCOverworldPlayerController
- [ ] Resume button calls UnpauseGame() and closes menu
- [ ] Return to Office button unpauses and transitions to L_Office
- [ ] ESC key in Overworld pauses/unpauses game correctly
- [ ] Convoy movement stops when paused, resumes when unpaused
- [ ] Physics and AI pathfinding stop during pause
- [ ] ESC key in Office does nothing (no pause functionality)
- [ ] No crashes or "Accessed None" errors
- [ ] Pause menu UI is interactive (buttons clickable, cursor visible)
- [ ] Round-trip pause/unpause works consistently

**Task 8 complete. Ready for Task 9 sub-tasks (Testing & Documentation)? Respond with 'Go' to continue.**

---

### Task 9: Testing, Polish & Documentation Update

**Purpose**: Comprehensive end-to-end testing of Week 3 features, performance validation, and documentation updates.

---

#### Step 9.1: Comprehensive Integration Testing

##### Step 9.1.1: Full Gameplay Flow Testing

- [ ] **Analysis**

  - [ ] Test complete player journey: Office → Overworld → Interaction → Return
  - [ ] Verify all systems work together without conflicts
  - [ ] Check for edge cases and error conditions

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
    - [ ] **AFCOverworldPlayerController**:
      - [ ] Purpose: Top-down camera control, click-to-move, POI interaction, pause management
      - [ ] Key methods: BeginPlay (InputManager TopDown mode), HandleClickMove, HandleInteractPOI, HandlePause, PauseGame, UnpauseGame, DebugReturnToOffice
      - [ ] Input actions: IA_OverworldClickMove, IA_OverworldInteractPOI, IA_Pause
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
      - [ ] Mappings: IA_OverworldPan (WASD), IA_OverworldZoom (Mouse Wheel), IA_OverworldClickMove (LMB), IA_OverworldInteractPOI (RMB), IA_Pause (ESC)
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
    - [ ] Add `Source/FC/Core/FCOverworldPlayerController.h/.cpp`
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
    - [ ] List key classes: AFCOverworldPlayerController, BP_OverworldCamera, BP_OverworldConvoy, BP_OverworldPOI
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
  - [ ] ✅ Task 4: AFCOverworldPlayerController and BP derivation
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
  - [ ] AFCOverworldPlayerController documentation
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

   - Create AFCOverworldPlayerController C++, Blueprint derivation, InputManager TopDown setup, camera possession

5. **Task 5: Convoy Pawn & Click-to-Move** (5 steps, 11 sub-steps)

   - Create IA_OverworldClickMove, BP_OverworldConvoy with FloatingPawnMovement, implement NavMesh pathfinding in controller

6. **Task 6: POI Actor & Interaction Stub** (5 steps, 13 sub-steps)

   - Create IA_OverworldInteractPOI, BP_OverworldPOI with InteractionBox, BPI_InteractablePOI interface, right-click handler in controller

7. **Task 7: Office-to-Overworld Transition** (5 steps, 9 sub-steps)

   - Add "Start Journey" button to WBP_MapTable, implement transition logic, Tab key return, InputManager GameState integration

8. **Task 8: Conditional Engine Pause** (5 steps, 14 sub-steps)

   - Create IA_Pause (ESC key), WBP_PauseMenu with Resume/Return buttons, implement pause/unpause in controller (Overworld only)

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

**END OF 0003-TASKS.MD DOCUMENT**
