# FALLEN COMPASS - Development Roadmap

Every week aims to leave the game **playable and compiling**. Multi-week features are marked as **Part 1 / Part 2 / Part 3**.

---

## Phase 1 – MVP / Prototype (Weeks 1–8) - 14.11.-09.12.2025

**Goal:** A thin but complete **vertical slice**:

- Start in office → plan simple route → Overworld travel → enter camp → simple combat → finish expedition with a basic report.

### Week 1 – Project Skeleton & First-Person Office Basics ✅ COMPLETE - 14.11.-19.11.2025

- **Feature: Project & Tooling Setup** ✅ **COMPLETE**

  - Create UE 5.7 C++ project (top-down + FPS hybrid).
  - Set up source control (Git) and basic branching strategy.
  - Create base `GameInstance`, core `GameMode` and `PlayerController` classes.

- **Feature: First-Person Controller – Part 1** ✅ **COMPLETE**

  - Implement simple first-person pawn (WASD + mouse look).
  - Create **L_Office level** (greybox room with full collision, lighting, props).
  - Ensure you can walk around with proper collision and scale.

- **Feature: Office Level & Interactables** ✅ **COMPLETE**

  - BP_OfficeDesk with CameraTargetPoint for future camera targeting.
  - SM_Door placeholder for exit interactions.
  - Comprehensive lighting: DirectionalLight, SkyLight, SkyAtmosphere, ExponentialHeightFog.
  - PlayerStart configured for proper spawn location.

- **Feature: In-World Main Menu System – Part 1** ✅ **COMPLETE**
  - Implement main menu **within L_Office** (not a separate level).
  - Add MenuCamera (static/slow-moving view focused on desk).
  - Create WBP_MainMenu widget with "New Legacy", "Continue", "Load Save Game", "Options", "Quit".
  - Implement smooth camera transition from menu state to first-person gameplay.
  - Door interaction returns to menu state (fade + level reload).
  - Add atmospheric effects: dust motes, candle flicker, ambient sounds.
  - See `/Docs/Tasks/0005-MainMenu-Concept.md` for detailed technical specification.

---

### Week 2 – Office Scene & Map Table Interaction (Meta Layer Start) - 19.11.-26.11.2025

- **Feature: Expedition Data Model – Part 1 (Foundation Classes)**

  - Create `UFCExpeditionData` C++ class (UObject):
    - Properties: ExpeditionName, StartDate, TargetRegion (placeholder string).
    - Properties: StartingSupplies (int32), ExpeditionStatus (enum: Planning, InProgress, Completed, Failed).
  - Create `UFCExpeditionManager` subsystem (UGameInstanceSubsystem):
    - Manages current active expedition data.
    - Methods: `StartNewExpedition()`, `GetCurrentExpedition()`, `EndExpedition(bool bSuccess)`.
  - Store expedition data in GameInstance for persistence across level transitions.

- **Feature: Table Object Interaction System – Part 1 (Foundation)**

  - Create `IFCTableInteractable` C++ interface (BlueprintNativeEvent):
    - Functions: `OnTableObjectClicked()`, `GetCameraTargetTransform()`, `CanInteract()`.
  - Create `BP_TableObject` base Blueprint class:
    - Implements `IFCTableInteractable`.
    - Has SceneComponent for camera target position.
    - Clickable via ray trace from first-person view.
  - Create specific table objects in L_Office:
    - `BP_TableObject_Map` (world map, positioned on table).
    - `BP_TableObject_Logbook` (expedition reports book, placeholder).
    - `BP_TableObject_Letters` (message stack, placeholder).
    - `BP_TableObject_Glass` (expedition start trigger, placeholder).

- **Feature: Map Table Widget – Part 1 (Placeholder UI)**

  - Create `WBP_MapTable` widget at `/Game/FC/UI/MapTable/`:
    - Canvas with parchment/map background texture.
    - Placeholder world map image (static, non-interactive).
    - UI elements:
      - Text display: "Expedition Planning" (title).
      - Text display: Current supplies count (reads from GameInstance).
      - Button: "Start Test Expedition" (enabled when supplies > 0).
      - Button: "Back" (returns to first-person view).
  - Implement Blueprint bindings to read/display supplies from GameInstance.

- **Feature: Table Interaction Flow – Part 1 (Widget Integration)**

  - Implement `AFCPlayerController::OnTableObjectClicked(AActor* TableObject)`:
    - Get camera target from table object.
    - Blend camera to focus on object (2s, cubic).
    - Show appropriate widget based on object type.
    - Set input mode to UI-only.
  - Implement `AFCPlayerController::CloseTableWidget()`:
    - Hide current widget.
    - Blend camera back to first-person.
    - Restore gameplay input mode.
  - Wire `BP_TableObject_Map::OnTableObjectClicked()` to show `WBP_MapTable`.

- **Feature: Level Transition Architecture – Part 1 (Loading Framework)**

  - Create `UFCLevelManager` subsystem (UGameInstanceSubsystem) if not exists:
    - Method: `LoadLevel(FName LevelName, bool bShowLoadingScreen)`.
    - Delegates to `UFCTransitionManager` for fade/loading effects.
    - Stores metadata about current level and previous level for back navigation.
  - Implement "Start Test Expedition" button logic:
    - Calls `ExpeditionManager->StartNewExpedition()` (sets status to InProgress).
    - Calls `LevelManager->LoadLevel(TEXT("L_Overworld_Test"), true)`.
    - Shows loading screen during transition.
  - **No Overworld level yet** (Week 3) - button shows "Coming Soon" message for now.

- **Feature: Persistent Game State Foundation – Part 1**

  - Extend `UFCGameInstance`:
    - Add property: `CurrentSupplies` (int32, default: 100 for testing).
    - Add methods: `AddSupplies(int32 Amount)`, `ConsumeSupplies(int32 Amount, bool& bSuccess)`.
  - Create `UFCGameStateData` struct (C++):
    - Fields: Supplies, Money (int32, placeholder), Day (int32, starts at 1).
  - Store `UFCGameStateData` in GameInstance.
  - Display current supplies in `WBP_MapTable`.

---

### Week 3 – Overworld Level & Basic Convoy Movement

- **Feature: Overworld Level – Part 1 (Basic Terrain & Camera)**

  - Create an **Overworld test level** (simple landscape / tiles).
  - Implement top-down camera actor:

    - Rotatable, pannable (WASD or edge scroll).
    - Basic zoom (mouse wheel), no scouts logic yet.

- **Feature: Overworld Movement – Part 1 (Click-to-Move Convoy)**

  - Create a simple **convoy pawn** (capsule/mesh) with NavMesh movement.
  - Implement **Left-click** on ground → move convoy to clicked location.
  - Add **Right-click interaction stub** on a test POI actor (just prints “Interact”).

---

### Week 4 – Overworld Map Widget & Basic Expedition Flow

- **Feature: Overworld Map Widget – Part 1 (Fog-of-War Stub)**

  - Implement a separate **World Map UI widget**:

    - Open via key or button while in Overworld.
    - Show player position on a simple 2D map.
    - Add a simple “discovered area” representation (e.g. circle around path).

- **Feature: Expedition Flow – Part 1 (Office → Overworld → Return Stub)**

  - From office:

    - Interact with map table → Start Expedition → load Overworld.

  - In Overworld:

    - Add a **“Return to Office”** debug option (e.g. button or menu entry).

  - On return, show a simple “Expedition Ended” screen.

---

### Week 5 – Local Camp Scene & Transitions

- **Feature: Local Scene: Camp – Part 1 (Level & Control)**

  - Create a **camp scene level** (top-down view, point & click movement).
  - Implement basic **Point & Click** unit movement (single leader pawn is enough).

- **Feature: Scene Transitions – Part 1 (Overworld ↔ Camp)**

  - In Overworld, add a **camp POI**:

    - Right-click or interaction key to “enter camp”.
    - Load camp level and spawn camp camera/pawn.

  - From camp, provide **“Return to Overworld”** interaction to go back to the same Overworld location.

---

### Week 6 – Combat Framework (Toolkit Integration)

- **Feature: Combat Framework – Part 1 (Advanced Turn-Based Tile Toolkit Integration)**

  - Import and set up the **Advanced Turn Based Tile Toolkit**.
  - Create a simple **combat test level** using the toolkit’s grid.
  - Implement basic turn order and unit placement (player vs. single enemy type).

- **Feature: Combat Encounter – Part 1 (Trigger from Camp/Overworld)**

  - From camp (or Overworld), add a simple “fight” trigger (e.g. interact at a POI).
  - On trigger → load combat level → complete a simple fight → return to camp or Overworld.
  - After win/lose, show a basic **combat result screen** (placeholder).

---

### Week 7 – Core Time & Resource Basics

- **Feature: Time System – Part 1 (Simple Day Counter)**

  - Implement a **global day counter** in `GameInstance`.
  - Time advances:

    - When traveling fixed distance in Overworld.
    - When using an “End Day” button in camp.

- **Feature: Simple Resources – Part 1 (Single Supplies Resource)**

  - Add a single **“Supplies”** variable.
  - Supplies are:

    - **Consumed** per travel segment.
    - **Consumed** when ending a day in camp.

  - Display supplies and day number in a simple HUD element.

---

### Week 8 – MVP Vertical Slice & Expedition Start Trigger

- **Feature: Expedition Start Trigger – Part 1 (Glass Object)**

  - Implement `BP_TableObject_Glass::OnTableObjectClicked()`:
    - Show confirmation dialog: `WBP_ExpeditionConfirm`.
    - Display final summary: Selected region, crew count, total supplies, total cost, risk level.
    - Buttons: "Begin Expedition" / "Cancel".
  - On "Begin Expedition":
    - Deduct transport costs and supplies from GameInstance.
    - Call `ExpeditionManager->StartNewExpedition()` with full route data.
    - Trigger Phase A travel event sequence (simple text summary or brief cutscene).
    - Load Overworld level at selected startpoint.
  - **Diegetic Design**: Glass represents "final toast before departure" - clear emotional beat.

- **Feature: Expedition Flow – Part 2 (Thin Vertical Slice)**

  - Complete basic loop:

    - Office → click map object → plan route → click glass → confirm.
    - Overworld: move to a camp POI.
    - Enter camp: optionally trigger a combat.
    - Return to Overworld → return to office → show a simple expedition report.

  - The report should include at least: days passed, supplies left, simple success/failure flag.

- **Feature: MVP Polish & Bugfixing**

  - Fix critical bugs and crashes found during full loop testing.
  - Clean up temporary debug logic that breaks the flow.
  - Tag build as **MVP Prototype**.

---

## Phase 2 – Demo Release (Weeks 9–26)

**Goal:** Turn the prototype into a **solid public demo** with:

- Meaningful **route planning & risk**,
- Basic **crew, resources, time & tasks**,
- Simple **combat depth**,
- **Difficulty modes** and **failure consequences** as in GDD,
- A small but curated region with multiple POIs and at least one village.

---

### Week 9 – Route Planning UI Basics

- **Feature: Route Planning UI – Part 1 (Two-Phase Route System)**

  - Extend `WBP_MapTable` with interactive world map:
    - **Phase A UI**: "Travel to Start" section.
      - Dropdown: Select transport method (Ship, Wagon, River Boat).
      - Display: Estimated cost, travel time, risk level.
    - **Phase B UI**: "Expedition Region" section.
      - Clickable regions on world map (2-3 predefined regions for MVP).
      - Select startpoint within region (coast, river, known settlement).
  - Store route data in `UFCRouteData` struct:
    - TransportMethod (enum), TransportCost (int32), TransportRisk (float).
    - TargetRegion (FName), StartPoint (FVector or FName).
    - Estimated expedition supplies needed (calculated).

- **Feature: Office→RouteData→Overworld Integration – Part 1**

  - Pass `RouteData` from office to Overworld when starting an expedition.
  - Spawn convoy at the defined expedition startpoint, not a hard-coded location.
  - Abstract Phase A (Travel to Start) as event summary before Overworld loads.

---

### Week 10 – Route Risk Model & Calculation Widget

- **Feature: Route Risk Model – Part 1 (Numeric Risk Score)**

  - For each route segment, compute a simple **risk score** (e.g. based on terrain tags).
  - Aggregated risk shown in the route planning UI.
  - Phase A (Travel to Start) has fixed risk values per transport method.

- **Feature: Route Calculation Widget – Part 1 (Integrated Estimator)**

  - Create `WBP_RouteCalculator` sub-widget (displayed in `WBP_MapTable`):
    - **Inputs**: Selected region, transport method, crew size (placeholder: fixed 5).
    - **Outputs**:
      - Estimated Provisions (Food): based on crew size × expected days.
      - Estimated Supplies: tools, medicine, misc.
      - Transport Cost: money cost for Phase A.
      - Total Risk Level: color-coded (Green/Yellow/Orange/Red).
    - **Validation**: "Start Expedition" button disabled if:
      - Insufficient supplies in GameInstance.
      - Insufficient money for transport.
  - Display calculation results clearly with tooltips explaining factors.

---

### Week 11 – Resource System Expansion

- **Feature: Resource System – Part 2 (Food + Generic Supplies)**

  - Split resources into at least:

    - **Food**
    - **Generic Supplies** (tools, fuel etc.)

  - Move all existing “Supplies” logic into Food + a generic travel cost in Supplies.

- **Feature: Travel Consumption – Part 1 (Resource Drain by Distance/Time)**

  - When the convoy travels segments in Overworld:

    - Reduce Food and Supplies based on **distance** or **segments**.

  - Update HUD to show separated resources.

---

### Week 12 – Camp Tasks & End-of-Day Logic

- **Feature: Camp Tasks – Part 1 (Hunt & Chop Wood)**

  - In camp scene, allow assigning crew members to:

    - **Hunting task** → returns some Food.
    - **Woodcutting task** → returns some Supplies or “Fuel”.

  - UI for assigning 1–2 crew members per task.

- **Feature: Time & Task Resolution – Part 2 (End Day)**

  - Implement “End Day” button:

    - Time +1 day.
    - Resolve tasks:

      - Roll simple outcomes, generate resources.

    - Consume extra Food for camp rest.

---

### Week 13 – Crew Model & Messages Hub (Part 1)

- **Feature: Crew Data Model – Part 1**

  - Define a `CrewMember` data structure:

    - Name, role, Health, base combat stats, a simple Skill stat.

  - Store crew in `GameInstance` / persistent manager.

- **Feature: Messages Hub Widget – Part 1 (Crew Applications)**

  - Create `WBP_MessagesHub` widget:
    - Tab 1: "Applications" (crew recruitment).
      - List of available crew members for hire.
      - Each entry: Name, Role, Skills summary, Hiring cost.
      - Button: "Hire" → add to crew roster, deduct money.
    - Tabs 2-3 placeholders for contracts and orders (Week 21+).
  - Wire `BP_TableObject_Letters::OnTableObjectClicked()` to show `WBP_MessagesHub`.

- **Feature: Office Crew Management – Part 1 (Select Crew for Expedition)**

  - In `WBP_MapTable`, add crew selection section:

    - Show available crew roster.
    - Checkboxes to select which crew members join next expedition.
    - Display total crew size in route calculator.

  - Pass selected crew into Overworld/camp/combat.

---

### Week 14 – Injuries & Fatigue

- **Feature: Injury & Fatigue System – Part 1**

  - Add:

    - **Health** (HP)
    - **Fatigue** (0–100)

  - After combat:

    - Apply HP loss & increase fatigue to participating crew.

  - After travel:

    - Slight fatigue gain.

- **Feature: Camp Rest Effects – Part 2**

  - When ending the day in camp:

    - Reduce fatigue.
    - Optionally heal a small amount of HP.
    - Consume extra Food.

---

### Week 15 – Scouts & Overworld Vision

- **Feature: Overworld Scouts & Vision – Part 1**

  - Introduce a **“scout strength”** value derived from:

    - Assigned scout crew members and/or equipment.

  - Use scout strength to:

    - Adjust camera **max zoom out**.
    - Adjust forward **vision radius** (how far ahead fog-of-war is revealed).

- **Feature: Fog-of-War – Part 2 (Session Persistence)**

  - Ensure fog-of-war revealed areas are stored for the **current expedition session**.
  - On returning to Overworld from camp/combat, previously revealed areas remain visible.

---

### Week 16 – Route-Based Events

- **Feature: Route Events – Part 1 (Negative Events from Risk)**

  - At end of each route segment or travel batch:

    - Roll for **negative events** based on route risk score.
    - Examples: lose Food, crew injury, Supplies loss.

- **Feature: Event Presentation UI – Part 1**

  - Create a simple **event popup**:

    - Title, description text.
    - Outcome summary (resources lost, injuries).

  - Events are initially **non-interactive** (no choices yet).

---

### Week 17 – Combat Abilities & Basic AI

- **Feature: Combat System – Part 2 (Player Actions)**

  - Extend combat gameplay:

    - Per unit: Move + Basic Attack.
    - Add a simple **Guard/Overwatch** action if the toolkit supports it easily.

- **Feature: Enemy AI – Part 1**

  - Implement basic AI behavior:

    - Move towards nearest player unit.
    - Attack if in range, otherwise advance.

  - Ensure enemy turn resolution is stable and readable (turn indicators).

---

### Week 18 – Combat Feedback & Integration with Crew System

- **Feature: Combat Feedback – Part 1**

  - Add:

    - Hit/miss feedback (floaty damage numbers, hit VFX).
    - Simple health bars for units.
    - Turn indicator UI.

- **Feature: Combat & Injury Integration – Part 2**

  - On end of combat:

    - Update **Crew HP**, fatigue, and check for **deaths**.

  - Remove dead crew from crew list and update camp/office UI accordingly.

---

### Week 19 – Failure States & Return Flow

- **Feature: Failure States – Part 1**

  - Define simple failure conditions:

    - All crew dead.
    - Food ≤ 0 for X days.

  - When a failure condition is met during expedition:

    - Trigger expedition failure flow.

- **Feature: Return to Office Flow – Part 2**

  - On success or failure:

    - Transition back to office.
    - Show a basic **“Expedition Result”** screen, including:

      - Success/failure flag.
      - Days spent.
      - Crew alive/dead.
      - Final resources.

---

### Week 20 – Difficulty Modes & Death Consequences

- **Feature: Difficulty Modes – Part 1 (Casual vs Hardcore Basics)**

  - Add simple difficulty settings:

    - Casual: more starting resources, slightly lower risk, no permadeath.
    - Hardcore: fewer resources, higher risk, permadeath enabled (to be finished later).

- **Feature: Death Consequences – Part 2 (Casual Logic per GDD)**

  - Implement **Casual-mode expedition failure**:

    - Expedition fails, but **explorer survives**.
    - Apply harsh consequences:

      - Loss of gear taken on expedition.
      - No reward/payout.
      - Reputation penalty (placeholder).

    - Crew may **quit** (remove some crew members from roster).

---

### Week 21 – Reputation & Finances

- **Feature: Reputation & Finances – Part 1**

  - Add:

    - **Reputation** value with generic “Patrons/Factions”.
    - **Money/Payment** value.

  - Successful expeditions:

    - Pay money reward.
    - Adjust reputation.

- **Feature: Expedition Outcome Report – Part 2**

  - Extend report UI to show:

    - Money gained/lost.
    - Reputation change.
    - List of surviving crew and their state.

---

### Week 22 – Persistent World Map & Expedition History

- **Feature: Persistent World Map – Part 1**

  - When an expedition ends:

    - Save discovered Overworld areas and visited POIs.

  - On new expedition:

    - Show previously explored areas as **known** from office map and Overworld.
    - `WBP_MapTable` displays fog-of-war: known regions detailed, unknown regions grayed/abstract.

- **Feature: Expedition Logbook Widget – Part 1**

  - Create `WBP_ExpeditionLog` widget:
    - List view: all past expeditions (scrollable).
    - Each entry shows: Date, Region, Outcome (Success/Failure), Duration (days).
    - Click entry → detail view:
      - Full expedition summary: crew losses, resources consumed, map coverage, narrative notes.
  - Wire `BP_TableObject_Logbook::OnTableObjectClicked()` to show `WBP_ExpeditionLog`.
  - Populate from `UFCExpeditionManager::GetExpeditionHistory()` (array of archived `UFCExpeditionData`).

---

### Week 23 – UI/UX Polish (HUD and Office)

- **Feature: UI Polish – Part 1 (Exploration HUD)**

  - Improve Overworld and camp HUD:

    - Clear displays for Food, Supplies, day, crew summary.
    - Better layout and icons.

- **Feature: Office UI Polish – Part 1**

  - Improve:

    - Map table UI (route planning clarity).
    - Crew selection UI.
    - Simple main menu and pause menu.

---

### Week 24 – Audio & Visual First Pass

- **Feature: Audio – Part 1**

  - Add placeholder:

    - Footstep SFX.
    - Simple UI click sounds.
    - Basic combat SFX.
    - One ambient track per main mode (office, Overworld, camp/combat).

- **Feature: Visual Polish – Part 1**

  - Improve:

    - Lighting in office, Overworld and camp.
    - Camera smoothing (Overworld, camp, combat).
    - Very simple VFX for hits, resting, events.

---

### Week 25 – Demo Content & Onboarding

- **Feature: Demo Content – Part 1 (Curated Region)**

  - Build **one small but complete region**:

    - 2–3 POIs.
    - 1 village or settlement scene.
    - 1–2 combat encounters placed logically.

  - Make sure region uses all core systems (route risk, events, camp, combat).

- **Feature: Onboarding – Part 1 (Basic Hints/Tutorial)**

  - Add simple onboarding:

    - Short text prompts/hints for first expedition (what to click, why).
    - Optional help UI from pause menu.

---

### Week 26 – Demo Release Preparation

- **Feature: Demo Stability & Performance – Part 1**

  - Play through entire demo multiple times.
  - Fix critical bugs and obvious performance issues (long loads, big hitches).

- **Feature: Demo Release Packaging – Part 1**

  - Set up:

    - Build configuration for **Demo build**.
    - Scripts or manual checklist for packaging and testing the build.
    - If desired: upload a private test build (itch/Steam) for your own QA.

---

## Phase 3 – Early Access Release (Weeks 27–40)

**Goal:** From “good demo” to **Early Access**:

- More **content** (regions, enemies, events),
- Deeper **camp management & resources**,
- Fully realized **Hardcore mode** with permadeath,
- Better **UX, performance, packaging**, and EA-ready onboarding.

---

### Week 27 – Content Expansion & New Enemies

- **Feature: Content Expansion – Region 2 Part 1**

  - Block out a **second Overworld region** with new topography.
  - Place placeholder POIs and potential camp spots.

- **Feature: Enemy Types – Part 1**

  - Add at least **one new enemy archetype** (e.g. ranged unit or durable tank).
  - Give it a slightly different AI behavior (e.g. prefers cover, stays at distance).

---

### Week 28 – Region 2 Local Scenes & Equipment Basics

- **Feature: Content Expansion – Region 2 Part 2 (Local Scenes)**

  - Create local scenes (camp, special POI, small settlement) tied to Region 2.
  - Add a couple of simple events tied to these scenes.

- **Feature: Equipment System – Part 1**

  - Implement a simple **equipment system**:

    - Weapons/armor affecting combat stats.

  - Add office UI to assign equipment to crew before expedition.

---

### Week 29 – Event System Depth (Branching Events)

- **Feature: Event System – Part 2 (Branching Choices)**

  - Extend event system to support:

    - Player choices with different outcomes.
    - Different resource/crew consequences based on choice.

- **Feature: Positive & Mixed Events – Part 1**

  - Add travel events that are:

    - Purely positive (discover resources, helpful NPCs).
    - Mixed (gain something at a risk or cost).

---

### Week 30 – Narrative Flavor & Log

- **Feature: Narrative Flavor – Part 1**

  - Write and hook up short descriptive texts for:

    - POIs.
    - Common events (travel & camp).
    - Expedition reports (intro/outro lines).

- **Feature: Expedition Log UI – Part 1**

  - Add an **in-game event log**:

    - Scrollable list of events and combat summaries during current expedition.
    - Accessible from HUD.

---

### Week 31 – Advanced Camp Management

- **Feature: Camp Management – Part 2 (More Tasks)**

  - Add new task types:

    - **Crafting** basic items.
    - **Medicine** preparation.
    - Temporary **fortifications** improving event outcomes or combat positions.

- **Feature: Injury & Healing – Part 2**

  - Implement longer-term injuries:

    - Injuries that persist across days and require treatment tasks.
    - Allow partially healed but weakened crew (e.g. reduced stats).

---

### Week 32 – Resource Types & Simple Economy

- **Feature: Resource System – Part 3 (More Types)**

  - Introduce additional resource types:

    - **Medicine**
    - **Ammo** (if relevant) or a second specialized resource.
    - Generic **Crafting materials**.

  - Update HUD, UI and relevant systems (tasks, events).

- **Feature: Economy & Trading – Part 1**

  - In village scenes:

    - Implement simple **buy/sell** interactions for key resources.

  - Use money from reputation/expeditions as currency.

---

### Week 33 – Hardcore Mode & Meta-Progression Hooks

- **Feature: Hardcore Mode – Part 2 (Permadeath)**

  - Implement **permadeath** logic for explorer in Hardcore:

    - If explorer dies: game treats this explorer as permanently dead.
    - Handle save files (e.g. mark profile as dead; cannot continue same run).

  - Ensure UI clearly communicates stakes.

- **Feature: Meta-Progression Hooks – Part 1**

  - Add basic infrastructure for future meta systems:

    - Flags for achievements/unlocks.
    - Persistent meta data (number of expeditions, total deaths etc.).

---

### Week 34 – Balancing & Debug Tools

- **Feature: Balancing Pass – Part 1**

  - Do a first structured balancing pass:

    - Resource consumption vs. gain.
    - Risk vs. event severity.
    - Combat difficulty across regions.

  - Adjust numeric values to make demo + new content feel cohesive.

- **Feature: Telemetry/Debug Tools – Part 1**

  - Implement simple in-game debug panel (for yourself):

    - Inspect current resources, crew stats, risk values.
    - Optionally buttons to jump to scenes or spawn events (for faster testing).

---

### Week 35 – UI Polish Round 2 & Clarity

- **Feature: UI Polish – Part 2**

  - Improve:

    - Tooltips for all main UI elements (resources, stats, tasks).
    - Icons and color coding for risk, injuries, morale (if present).

  - Make sure everything is understandable without reading the GDD.

- **Feature: Accessibility – Part 1**

  - Basic options:

    - Font size scaling.
    - Colorblind-friendly palette for critical indicators.
    - Minimal keybinding customization.

---

### Week 36 – Performance & Save/Load Robustness

- **Feature: Performance Optimization – Part 2**

  - Optimize:

    - LODs for Overworld and scenes.
    - Collision and tick usage for actors.
    - Garbage collection behavior where possible.

- **Feature: Stability & Save/Load – Part 2**

  - Make sure:

    - Saves do not easily corrupt (simple validation).
    - Interesting edge cases are handled (e.g. saving in camp, after combat, during low resources).

---

### Week 37 – Early Access Packaging & Platform Integration

- **Feature: Early Access Packaging – Part 2**

  - Finalize:

    - Automated build pipeline where possible.
    - Versioning scheme (e.g. 0.1.x EA).
    - Build settings for EA (logging levels, crash dumps).

- **Feature: Platform Integration – Part 1**

  - For your chosen platform (e.g. Steam):

    - Basic integration (overlay, app ID).
    - Stub out achievements (even if not fully used yet).
    - Set up crash reporting if feasible.

---

### Week 38 – EA Tutorial & Player Support

- **Feature: EA Tutorial Experience – Part 2**

  - Improve onboarding:

    - Optional “first expedition” tutorial with scripted steps.
    - Clear instructions for map table, route planning, camp and combat.

- **Feature: Marketing Support – Part 1**

  - Add:

    - Visible build number in main menu.
    - Simple **credits** screen.
    - Optional in-game link/info for bug reporting or feedback.

---

### Week 39 – Release Candidate Preparation & Localization Framework

- **Feature: Release Candidate – Part 1**

  - Freeze feature development.
  - Focus on:

    - Critical bug fixes only.
    - Full playthroughs of multiple expeditions.
    - Check stability over long runs.

- **Feature: Localization Framework – Part 1**

  - Externalize text (dialogs, UI labels, events).
  - Support at least **EN + DE**, even if translations are minimal at first.

---

### Week 40 – Early Access Release & Post-Release Backlog

- **Feature: Early Access Release – Part 3**

  - Build final EA version.
  - Configure store page:

    - Description reflecting GDD features.
    - Screenshots & trailer from game.

  - Push live (or prepare as “ready to ship”).

- **Feature: Post-Release Backlog Setup – Part 1**

  - Collect:

    - Personal notes on issues/ideas from development.
    - Plan for first EA patches (hotfixes + QoL).

  - Turn this into a **Phase 4 backlog** (post-EA roadmap).

---

## Phase 4 – Post-EA Backlog & Technical Debt

### Known Issues & Technical Backlog

#### Log Warnings (from Week 1 Log Review)

- [ ] LogTemp: Warning: RestorePlayerPosition: No pending load data (expected if no save exists)
- [ ] LogRenderer: Warning: [VSM] Non-Nanite Marking Job Queue overflow. Performance may be affected. (large shadow map area, not critical for prototype)
- [ ] LogPlayerController: Error: InputMode:UIOnly - Attempting to focus Non-Focusable widget SObjectWidget [Widget.cpp(990)]! (UI focus issue, not blocking)
- [ ] LogFallenCompassPlayerController: Warning: TableView Debug: CameraTargetPoint Rotation/Location/Spawned Camera Rotation (debug output, not critical)

These should be reviewed and addressed in a future sprint as part of technical debt reduction and polish.

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
