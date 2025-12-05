## Estimated Time & Schedule

- **Start Date**: 2025-12-04
- **Max Time/Day**: 2 hours (7 days/week), solo junior dev with AI-assisted coding
- **Total Target Time for Week 5**: ~16–18 hours

### Day-by-Day Breakdown

- **Day 1 (Dec 4)** – 2h
   - 0005-0 Analysis & Conventions (docs/tech review, existing assets scan)
   - Sketch implementation plan and call flows for Set Up/Break Camp and fog-of-war

- **Day 2 (Dec 5)** – 2h
   - 0005-1.1: Create base `L_Camp` layout, lighting, NavMesh bounds
   - Quick PIE sanity check (no interactions yet)

- **Day 3 (Dec 6)** – 2h
   - 0005-1.2: Implement Camp leader pawn (class setup, mesh, basic click-to-move using reused Overworld logic)
   - 0005-1.3: Ensure Camp pawn possession on `L_Camp` load

- **Day 4 (Dec 7)** – 2h
   - 0005-2.1: Implement "Set Up Camp" HUD/shortcut action wired through transition managers
   - 0005-2.2: Implement Camp exit (bottom-right) + "Break Camp" HUD/shortcut and return to Overworld with position restore

- **Day 5 (Dec 8)** – 2h
   - 0005-3.1/3.2: Implement fog-of-war reveal helper (16×16 subgrid) and apply mask to existing Overworld map/minimap
   - 0005-3.3: In-memory persistence across simple level transitions

- **Day 6 (Dec 9)** – 2h
   - 0005-4.1/4.2: Fix ESC pause toggle (controller + state manager) and Abort button visibility
   - Start 0005-Y verification tests on main expedition + camp loop

- **Day 7 (Dec 10)** – 2–4h (buffer)
   - Finish 0005-Y/Z/V verification (technical checks, log review, Git hygiene)
   - Address small regressions/UX polish, update documentation and DRM status
   - Reserve up to +2h slack if earlier days overrun

---

## Task Metadata

```yaml
Task ID: 0005
Sprint/Phase: Week 5 / Phase 1 – MVP / Prototype
Feature Name: Local Camp Scene & Overworld Fog-of-War (Stub)
Dependencies:
   - 0001 – Project Skeleton & Office Basics
   - 0002 – Office Scene & Map Table Interaction
   - 0003 – Overworld Level & Basic Convoy Movement
   - Week 4 – Expedition Loop & Bug Fixing (Level/State transitions, World Map)
Estimated Complexity: Medium
Primary Files Affected:
   - /Content/FC/World/Levels/L_Camp.umap (new)
   - /Source/FC/Core/FCPlayerController.*
   - /Source/FC/Core/UFCLevelManager.*
   - /Source/FC/Core/UFCTransitionManager.*
   - /Source/FC/Core/UFCGameStateManager.*
   - /Source/FC/Core/UFCExpeditionManager.*
   - /Source/FC/Characters/ (Camp pawn / explorer)
   - /Content/FC/UI/Overworld/ (HUD, minimap widgets)
   - /Content/FC/UI/Menus/WBP_PauseMenu
   - /Content/FC/UI/Overworld/Abort-related widgets
```

---

## Overview & Context

### Purpose

Implement the first playable **Camp** scene and a basic **Overworld fog-of-war stub**, wired into the existing level/state transition architecture, so a player can: travel on the Overworld, set up camp, control a designated explorer character in Camp, break camp to return to the Overworld at the same position, and see explored Overworld areas revealed on the map/minimap.

### Architecture Notes

- Reuse the established core framework from Weeks 1–4:
   - `UFCLevelManager` and `UFCTransitionManager` for all level loads/fades (no direct `OpenLevel` from widgets).
   - `UFCGameStateManager` for game state transitions (Office, Overworld_Travel, Camp in future, Paused).
   - `UFCExpeditionManager` / expedition data as the single source of truth for convoy position, designated explorer character and exploration mask.
- Camp control:
   - Introduce or configure a dedicated Camp pawn (e.g. `AFC_CampLeaderPawn`) that always represents the **designated explorer** (not "first convoy member").
   - Reuse Overworld click-to-move and interaction patterns (LMB move, RMB interact, LMB-on-interactable queues interaction on overlap) for consistency.
- Fog-of-war:
   - Use the existing 16×16 subgrid (~125m×125m cells) as the reveal grid around the convoy.
   - For Week 5, use a **single prototype minimap** for the active Overworld level; defer per-level minimap generation and camera-based reveal refinement to Week 6.
- Save/load:
   - Week 5 only provides **in-memory persistence** of exploration state within the current session. Full quick-save hooks and SaveGame integration are defined for Week 6.

### Reference Documents

- `/Docs/UE_NamingConventions.md` - Unreal Engine naming convention and folder structure
- `/Docs/UE_CodeConventions.md` - Unreal Engine coding conventions
- `/Docs/Fallen_Compass_GDD_v0.2.md` - Game Design Document (Overworld, Camp, Fog-of-War; esp. 3.2.2, 3.2.3)
- `/Docs/DRM_Draft.md` - Development Roadmap, Weeks 4–6
- `/Docs/Technical_Documentation.md` - Current technical architecture and class overview (Level/State managers, Expedition system, Overworld input)

---

## Pre-Implementation Phase

### 0005-0: Analysis & Conventions

- [x] **Analysis of Existing Implementations**
   - [x] Read `/Docs/Fallen_Compass_GDD_v0.2.md` sections on Camp, local scenes, and Overworld (3.2.2, 3.2.3, 4.9).
   - [x] Read `/Docs/DRM_Draft.md` Week 4–6 descriptions (Expedition Loop, World Map/Exploration, Camp, Fog-of-War, Quick-Save Hooks).
   - [x] Read `/Docs/Technical_Documentation.md` sections on:
      - [x] `UFCLevelManager`, `UFCTransitionManager`, `UFCGameStateManager`, `UFCLevelTransitionManager`.
      - [x] `UFCExpeditionManager` / `FFCWorldMapExploration` (expedition state, exploration mask, Overworld mapping).
      - [x] `AFCPlayerController`, `UFCInputManager`, and Overworld input/interaction logic.
   - [x] Scan project for existing Camp/POI/local-scene levels under `/Content/FC/World/Levels/` and related Blueprint actors.
   - [x] Document what already exists and what needs to be created:

      ```
      Existing:
      - Office ↔ Overworld transitions via UFCLevelTransitionManager and UFCGameStateManager
      - Overworld top-down input and camera patterns planned via UFCInputManager and UFCCameraManager
      - World-map exploration grid and mask handling in UFCExpeditionManager / FFCWorldMapExploration

      To Create/Extend:
      - L_Camp level at /Content/FC/World/Levels/
      - Camp leader pawn C++/Blueprint and associated assets
      - Overworld HUD hooks for "Set Up Camp" (HUD button + shortcut) and fog-of-war reveal
      - Camp HUD hooks for "Break Camp" (HUD button + shortcut)
      - LevelTransitionManager helpers for Overworld ↔ Camp transitions (reusing RequestTransition pattern)
      - Fog-of-war reveal helper integrated into existing exploration grid
      ```

- [x] **Code Conventions Compliance Check**
   - [ ] Review `/Docs/UE_CodeConventions.md` for applicable rules:
      - [ ] Encapsulation (§2.1) – private members with public accessors.
      - [ ] Modular Organization (§2.2) – place new code in appropriate `/Source/FC/[Module]/` folders.
      - [ ] Blueprint Exposure (§2.3) – only expose what Camp and Overworld Blueprints truly need.
      - [ ] Memory Management (§2.5) – all `UObject*` pointers use `UPROPERTY()`.
      - [ ] Event-Driven Design (§2.6) – prefer timers/delegates over Tick where possible.
      - [ ] Separation of Concerns (§4.3) – transitions via managers, not UI widgets.

- [x] **Naming Conventions Compliance Check**
   - [ ] Ensure C++ classes use correct prefixes (A/U/F + FC).
   - [ ] Ensure new Blueprints use `BP_` / `WBP_` prefixes and live under `/Content/FC/...`.
   - [ ] Ensure any new input assets follow `IA_` / `IMC_` naming rules.

-- [x] **Implementation Plan**
   - [x] List the classes/Blueprints to create or extend for Camp pawn, transitions, and fog-of-war.
   - [x] Identify integration points with `UFCLevelManager`, `UFCTransitionManager`, `UFCLevelTransitionManager`, `UFCGameStateManager`, and `UFCExpeditionManager`.
   - [x] Sketch the expected call flows:
      - [x] Overworld HUD/shortcut → "Set Up Camp" → `UFCLevelTransitionManager` helper → `RequestTransition` → `UFCLevelManager::LoadLevel(L_Camp)` → `AFCPlayerController` possesses Camp explorer pawn.
      - [x] Camp exit/HUD/shortcut → "Break Camp" → `UFCLevelTransitionManager` helper → transition back to Overworld → restore convoy position/state from `UFCExpeditionManager`.
      - [x] Overworld tick/position update → fog-of-war helper in/under `UFCExpeditionManager` → update exploration mask (`FFCWorldMapExploration`) → map/minimap material.

---

## Week 5 – Local Camp Scene & Fog-of-War (Task 0005)

### Parent Tasks (High-Level)

1. **0005-1 – Implement Local Camp Scene (L_Camp) – Part 1**
   - Create the first playable Camp level with a minimal terrain, campfire prop, and basic lighting that matches the Overworld tone.
   - Implement a single controllable leader pawn with Point & Click movement confined to the camp area.

2. **0005-2 – Implement Overworld↔Camp Scene Transitions – Part 1**
   - Wire up interaction on an Overworld POI to enter the Camp level and a corresponding interaction in Camp to break camp and return to Overworld, preserving convoy position/state.

3. **0005-3 – Implement Overworld Fog-of-War Stub**
   - Implement a simple reveal-radius around the convoy in the Overworld, hook it into the existing world map exploration system, and mask out unseen areas on the Overworld Map / Minimap with basic session persistence.

4. **0005-4 – Bug Fixes & UX Polish (Week 5 Scope)**
   - Address the open roadmap bugs related to Overworld pause/ESC handling and abort button visibility, plus any critical issues discovered while implementing Camp and Fog-of-War.

---

## Detailed Tasks

### 0005-1 – Implement Local Camp Scene (L_Camp) – Part 1

#### 0005-1.0: Analysis & Discovery

- [x] **Review Design & Tech Docs**
   - [x] `/Docs/Fallen_Compass_GDD_v0.2.md` (3.2.2, 3.2.3, 4.9): Camp and local scenes are small, bounded top-down levels with Point & Click control; Overworld is a 3D top-down convoy map with LMB move / RMB interact; Camp is conceptually a local scene sitting on top of Overworld travel.
   - [x] `/Docs/DRM_Draft.md` Week 5: Camp scope is a single generic `L_Camp` layout; Camp control must mirror Overworld input (LMB move, RMB interact, LMB-on-interactable queues interaction on overlap); Set Up/Break Camp are explicit HUD/shortcut actions, not POI-bound; fog-of-war stub is convoy-radius based and uses existing exploration grid.
   - [x] `/Docs/Technical_Documentation.md`: Camp will plug into the existing Office ↔ Overworld stack: `UFCGameInstance` → `UFCLevelManager` / `UFCGameStateManager` / `UFCTransitionManager` / `UFCLevelTransitionManager`, with `AFCPlayerController` + `UFCInputManager` owning input and camera for both Overworld and future Camp.
- [x] **Scan Existing Project Assets**
   - [x] Confirmed existing world levels under `/Content/FC/World/Levels/` include at least `L_Office` and `L_Overworld`; there is **no existing `L_Camp`** or dedicated Camp/local-scene level, so Week 5 will introduce `L_Camp` as a new map.
   - [x] Identified reusable character/convoy meshes under `/Content/FC/Characters/` (e.g. mannequin or convoy leader mesh) that can be used as the visual for the Camp leader pawn; Overworld meshes can be reused for consistency.
   - [x] Overworld control uses `AFCPlayerController` with `UFCInputManager` and Enhanced Input assets (`IA_Click`, `IA_Interact`, `IMC_FC_TopDown` or equivalent) to drive click-to-move and interaction; this pattern will be reused for Camp instead of implementing per-level input in Blueprints.
- [x] **Decide Camp Control Architecture**
   - [x] Camp will use a **dedicated pawn class** `AFC_CampLeaderPawn` (C++ in `/Source/FC/Characters/`, with a Blueprint child `BP_CampLeaderPawn` in `/Content/FC/Characters/`) rather than reusing the Overworld convoy controller, to keep Camp-local logic (future tasks, resting, local interactions) separated while still sharing movement helpers.
   - [x] `L_Camp` will always be loaded via the existing **transition pipeline** (no direct `OpenLevel` in widgets): Overworld HUD/shortcut → `AFCPlayerController` → `UFCLevelTransitionManager` helper → `RequestTransition` → `UFCGameStateManager::TransitionTo` (new Camp state, e.g. `Camp_Exploration` when added) → `UFCLevelManager::LoadLevel("L_Camp")` → `UFCTransitionManager` fade flow.
   - [x] **Call flow for entering Camp** (design-level):
      - Overworld: player presses HUD button or shortcut "Set Up Camp".
      - `AFCPlayerController` forwards this to `UFCLevelTransitionManager` (e.g. `RequestSetUpCampFromOverworld`).
      - `UFCLevelTransitionManager` queries `UFCExpeditionManager` for current expedition and convoy position, stores return data, sets game state to Camp, and asks `UFCLevelManager::LoadLevel(L_Camp)` via `UFCTransitionManager` fade.
      - After `L_Camp` loads, `AFCPlayerController` spawns/possesses `AFC_CampLeaderPawn` at a defined Camp entry point.
   - [x] **Designated explorer selection**: For the prototype and demo, the explorer is always one of two predefined Blueprints (e.g. `BP_Explorer_Male`, `BP_Explorer_Female`); skills/traits are not yet dynamic. Expedition data can expose a lightweight flag such as `EFCExplorerType ExplorerType` (Male/Female) or an `FName ActiveExplorerId` so transition helpers and `AFC_CampLeaderPawn` know which BP to spawn. Camp leader pawn will always represent this designated explorer, not implicitly the first convoy member.

#### 0005-1.1: Create Camp Level `L_Camp`

- [x] **Level Creation**
   - [x] Create `L_Camp` level in `/Content/FC/Levels/` following naming conventions.
   - [x] Add simple terrain/ground mesh (flat plane or small sculpted area) sized for a small camp.
   - [x] Place a campfire prop (placeholder mesh is fine) and a few simple props (tents, crates) to suggest layout.
- [x] **Lighting & Atmosphere**
   - [x] Add a primary light source (directional or sky light) consistent with Overworld time-of-day.
   - [x] Add one or two point/spot lights around the campfire for local ambience.
   - [x] Ensure exposure and post-process volumes match basic Overworld look (reuse existing profiles where possible).
- [x] **NavMesh / Movement Area**
   - [x] Add a `NavMeshBoundsVolume` that covers the intended movement area.
   - [x] Bake/verify NavMesh so the leader pawn can move freely within the camp region.

#### 0005-1.2: Implement Camp Leader Pawn (Point & Click)

- [x] **Class Design**
   - [x] Define a C++ character class `AFC_ExplorerCharacter` in `/Source/FC/Characters/` with top-down movement behavior and an `EFCExplorerType` enum (Male/Female) to drive mesh selection in Blueprints.
   - [x] Ensure it is distinct from the Overworld convoy controller (per DRM) but can reuse shared movement helpers if they exist. Comment: We use one Controller, different IMC.
   - [x] Ensure the pawn always represents the **designated explorer character**, not just the first convoy member. For the prototype, this means choosing between fixed Blueprints such as `BP_Explorer_Male` / `BP_Explorer_Female` based on a simple field in expedition data (e.g. `EFCExplorerType` or `ActiveExplorerId`) and spawning/possessing the correct BP.
- [x] **Input & Movement Logic**
   - [x] Mirror Overworld input: Left Mouse Button moves the pawn, Right Mouse Button initiates interaction (reuse Overworld input mapping / helpers where possible).
   - [x] When Left Mouse Button is pressed directly on an interactable object, queue the interaction so it fires once the pawn overlaps the target (matching Overworld behavior; review existing Overworld implementation to mirror edge cases).
   - [x] Confine movement to the camp NavMesh area; prevent leaving the playable space.
   - [x] Ensure camera controls (pan/zoom) in Camp do not conflict with Office/Overworld mappings.
- [x] **Visuals & Feedback**
   - [x] Assign a temporary mesh/skeletal mesh to the leader pawn.
   - [x] Optionally add a simple click decal or cursor feedback at target locations.

#### 0005-1.3: Hook Camp Into Transition Framework (Stub)

- [x] **Spawn/Assign Pawn in `L_Camp`**
   - [x] Ensure that when `L_Camp` loads, the correct pawn class is possessed by the player controller.
   - [x] Spawn pawn at a sensible camp entry location (near campfire or entry point).
- [x] **Return Path Awareness (Design Only)**
   - [x] Document how the Camp scene will later return to Overworld (to be implemented in 0005-2) and what state needs to be carried back (position, party composition, etc.).

##### Acceptance Criteria – 0005-1

- [x] `L_Camp` level exists with basic terrain, campfire prop, and lighting.
- [x] Player can load `L_Camp` in PIE and control a leader pawn via Point & Click within the camp.
- [x] NavMesh is correctly baked and prevents the pawn from leaving the camp area.
- [x] No new critical errors or "Accessed None" warnings appear when entering and moving in `L_Camp`.

---

### 0005-2 – Implement Overworld↔Camp Scene Transitions – Part 1

#### 0005-2.0: Analysis & Discovery

- [ ] **Review Existing Transition Systems**
   - [ ] Read DRM Week 4 implementation summary (4.2, 4.3, 4.6, 4.7) for level/state transition architecture.
   - [ ] Inspect `UFCLevelTransitionManager` (or equivalent) and any existing state/level managers handling Office↔Overworld.
- [ ] **Identify Overworld POI Integration Point**
   - [ ] Locate the Overworld POI actor/blueprint that should trigger Camp entry (e.g. camp marker or generic POI base class).
   - [ ] Verify existing interaction system (click/right-click, context actions) and how it triggers transitions.

#### 0005-2.1: Implement "Set Up Camp" Action (Overworld → Camp)

- [ ] **Action Definition**
   - [ ] Define a "Set Up Camp" action that can be triggered from the Overworld HUD and via a keyboard shortcut (no POI interaction required; **explicitly not bound to a POI interaction**).
   - [ ] Enforce any preconditions (e.g. not already in camp, expedition active, safe location checks if applicable).
- [ ] **Transition Implementation**
   - [ ] Route the action through the level/state transition manager instead of directly opening levels.
   - [ ] Implement a transition path: Overworld → fade → load `L_Camp` → possess Camp leader pawn.
   - [ ] Capture the current Overworld convoy position and any necessary expedition state for restoration on return.

#### 0005-2.2: Implement "Break Camp" Action & Exit (Camp → Overworld)

- [ ] **Camp-Side Interaction**
   - [ ] Place an exit/edge area near the **bottom-right** of the camp level that can be interacted with using the same RMB interaction pattern as Overworld.
   - [ ] Add a "Break Camp" action exposed on the Camp HUD and via a keyboard shortcut, reusing the same underlying transition logic as Overworld → Camp.
- [ ] **Overworld Position Preservation**
   - [ ] Decide and implement how the return position is stored (e.g. `FVector` in an expedition/convoy data struct).
   - [ ] On returning to Overworld, reposition the convoy to its previous location and restore camera if applicable.

##### Acceptance Criteria – 0005-2

- [ ] From Overworld, interacting with the designated POI smoothly transitions into `L_Camp`.
- [ ] In `L_Camp`, interacting with the camp exit ("Break Camp") returns to Overworld without losing convoy position or expedition state.
- [ ] No duplicate pawns or controllers are left behind after multiple enter/exit cycles.

---

### 0005-3 – Implement Overworld Fog-of-War Stub

#### 0005-3.0: Analysis & Discovery

- [ ] **Review Existing Fog/WorldMap Systems**
   - [ ] Read the Week 4 implementation summary sections on fog-of-war and world map exploration in `/Docs/DRM_Draft.md`.
   - [ ] Inspect `UFCExpeditionManager` and `FFCWorldMapExploration` for reveal mask handling and update hooks.
   - [ ] Identify where convoy world position is already mapped into exploration cells (if implemented in Week 4 code).

#### 0005-3.1: Implement Reveal Radius Around Convoy

- [ ] **Reveal Logic**
   - [ ] Implement a helper (C++ or Blueprint) that converts the convoy’s world position into the appropriate global cell index on the **16×16 subgrid** (each cell ~125m×125m).
   - [ ] Around that index, reveal a configurable radius of cells each tick or at a fixed time interval.
   - [ ] Ensure reveal only happens while an expedition is active in Overworld.

#### 0005-3.2: Mask Out Unseen Areas on Map/Minimap

- [ ] **Minimap / Map Integration**
   - [ ] Ensure the RevealMask (or equivalent texture) is applied to the Overworld Map / Minimap widget material.
   - [ ] For Week 5, use the existing/prototype minimap texture for the active Overworld level; defer **per-level minimap generation for all 256 tiles** to Week 6.
   - [ ] Verify that unrevealed cells are visually masked (darkened or hidden) while revealed cells show the underlying map.
   - [ ] Add a simple configuration option for reveal radius in `UFCGameInstance` or a similar central config.

#### 0005-3.3: Basic Session Persistence

- [ ] **In-Memory Persistence**
   - [ ] Ensure revealed cells remain revealed for the duration of the current game session (no resetting on simple level reloads within Overworld).
   - [ ] If an existing SaveGame stub is present from Week 4, document minimal integration points but **defer full SaveGame wiring to Week 6** (Quick-Save Hooks feature).

##### Acceptance Criteria – 0005-3

- [ ] Convoy movement in Overworld reveals a circular (or square) area around its position on the map.
- [ ] The Overworld Map / Minimap clearly distinguishes between unseen and revealed terrain.
- [ ] Revealed areas persist for the current session and across basic level transitions as designed for Week 5.

---

### 0005-4 – Bug Fixes & UX Polish (Week 5 Scope)

#### 0005-4.0: Analysis & Bug Triage

- [ ] **Review Open Issues**
   - [ ] Re-read DRM Week 4 and Week 5 bug lists: Issue #4 (ESC Key Toggle in Overworld), Issue #2 (Abort Button Visibility).
   - [ ] Check any additional critical bugs discovered during Weeks 3–4 related to Office/Overworld/Map.
- [ ] **Reproduce & Log**
   - [ ] Reproduce each bug in the current build and capture steps, logs, and any stack traces.

#### 0005-4.1: Fix ESC Key Toggle in Overworld (Issue #4)

- [ ] **Implementation**
   - [ ] Move ESC handling away from widget `OnKeyDown` (tested and not working) and into the Enhanced Input / `AFCPlayerController` handling, using `UFCGameStateManager`’s pause state and state stack.
   - [ ] Ensure ESC toggles the pause menu consistently while in Overworld, without conflicting with Office or Camp scenes.
- [ ] **Testing**
   - [ ] Verify ESC behavior in Office, Overworld, and Camp to ensure each context behaves as intended.

#### 0005-4.2: Fix Abort Button Visibility (Issue #2)

- [ ] **Implementation**
   - [ ] Identify the widget containing the Abort button and the state it should reflect.
   - [ ] Hide or disable the Abort button while in Office and Camp; ensure it is only visible/active on `L_Overworld` as per DRM.
- [ ] **Testing**
   - [ ] Verify Abort button visibility across all relevant levels and transitions.

#### 0005-4.3: Regression & UX Pass

- [ ] **Regression Testing**
   - [ ] Perform a short end-to-end loop: Office → Overworld → Camp → Overworld → Office, using new Camp and fog features.
   - [ ] Watch for new errors, overlaps (e.g. pause menu in wrong level), or inconsistent UI states.

##### Acceptance Criteria – 0005-4

- [ ] ESC key toggles pause/pause menu correctly in Overworld without breaking Office or Camp behavior.
- [ ] Abort button only appears where it should (Overworld), with no stray visibility in Office or Camp.
- [ ] No new critical bugs introduced by Week 5 work; expedition loop remains stable.

---

## Overall Acceptance Criteria for Week 5 (Task 0005)

- [ ] A basic but functional `L_Camp` level exists with Point & Click leader control.
- [ ] Players can enter Camp from an Overworld POI and return to Overworld while preserving expedition state and convoy position.
- [ ] Overworld fog-of-war reveals around the convoy and masks unseen terrain, with basic persistence for the session.
- [ ] Week 4/5 Overworld-related bugs (ESC key toggle, Abort button visibility) are addressed without introducing critical regressions.

---

## 0005-Y: Verification & Testing Phase

### 0005-Y.1: Expedition Loop with Camp Verification

- [ ] **Analysis**
   - [ ] Re-read `/Docs/Technical_Documentation.md` sections on Office↔Overworld transitions, conditional pause, and expedition loop.
   - [ ] List test cases needed to cover Office → Overworld → Camp → Overworld → Office, including ESC and Abort behavior.

- [ ] **Test Sequence – Expedition + Camp Flow**
   - [ ] Start in `L_Office`, plan and start an expedition to Overworld.
   - [ ] On Overworld, use HUD/shortcut to trigger **"Set Up Camp"`; verify smooth transition into `L_Camp` and correct explorer pawn possession.
   - [ ] In Camp, move the explorer via click-to-move and interact with nearby props; verify input parity with Overworld.
   - [ ] Use Camp exit (bottom-right) and Camp HUD/shortcut to trigger **"Break Camp"`; verify return to Overworld at the preserved convoy position.
   - [ ] Use existing flow to finish the expedition and return to Office (if implemented for Week 4); confirm no duplicated controllers or pawns.
   - [ ] Monitor Output Log for errors/warnings, in particular any `Accessed None` messages from Camp and Overworld widgets.

### 0005-Y.2: Fog-of-War & Minimap Verification

- [ ] **Test Sequence – Exploration & Reveal**
   - [ ] On Overworld, move the convoy into previously unrevealed areas and verify cells around convoy position become marked as revealed in the 16×16 subgrid.
   - [ ] Open the Overworld map/minimap and confirm that unrevealed cells are properly masked while revealed cells show the underlying map.
   - [ ] Trigger a simple level reload or transition (e.g. Overworld → Camp → Overworld) and verify exploration remains revealed for the current session.
   - [ ] Confirm that reveal only happens while an expedition is active.

### 0005-Y.3: ESC, Abort Button & Regression Testing

- [ ] **ESC Behavior**
   - [ ] Verify ESC toggles pause correctly in Overworld using the Enhanced Input/`AFCPlayerController` + `UFCGameStateManager` implementation (no reliance on `OnKeyDown` in widgets).
   - [ ] Verify ESC behavior in Office and Camp is correct and does not conflict with Overworld pause.

- [ ] **Abort Button Visibility**
   - [ ] Confirm Abort is visible and functional only on `L_Overworld` as designed.
   - [ ] Confirm Abort is hidden/disabled in Office and Camp.

- [ ] **Regression & Stability**
   - [ ] Perform multiple expedition loops with several Set Up / Break Camp cycles.
   - [ ] Check for memory issues, duplicated pawns/controllers, or stuck states.
   - [ ] Ensure no new critical errors have been introduced.

---

## 0005-Z: Technical & Conventions Verification

### 0005-Z.1: Core Framework Integration Check

- [ ] **Project Settings**
   - [ ] Verify `UFCGameInstance` and core subsystems (`UFCLevelManager`, `UFCTransitionManager`, `UFCGameStateManager`, `UFCUIManager`) are correctly configured.
   - [ ] Confirm new input actions/mappings for Set Up Camp / Break Camp are correctly registered and active in Overworld/Camp contexts.

- [ ] **Subsystem Initialization**
   - [ ] Launch PIE and check Output Log for expected initialization logs from `LogFCLevelManager`, `LogFCGameStateManager`, `LogFallenCompassGameInstance`, and any new Camp/fog-of-war logs.
   - [ ] Confirm there are no "Failed to initialize" or similar errors.

### 0005-Z.2: Code Conventions Compliance Audit

- [ ] **UE_CodeConventions.md Compliance**
   - [ ] Encapsulation: new data members are private with appropriate accessors; only necessary fields are `BlueprintReadWrite`.
   - [ ] Modular organization: new classes live under appropriate `/Source/FC/[Module]/` folders; forward declarations used in headers, full includes in .cpp.
   - [ ] Blueprint exposure: only necessary functions are `BlueprintCallable` or `BlueprintImplementableEvent`.
   - [ ] Memory management: all `UObject*` pointers on new classes use `UPROPERTY()`; persistent widgets (if any) have correct outers.
   - [ ] Event-driven design: avoid unnecessary Tick; use timers/delegates for fog-of-war and transitions.
   - [ ] Separation of concerns: controllers delegate transitions to managers; widgets do not call `OpenLevel` directly.

### 0005-Z.3: Naming & Folder Structure Verification

- [ ] **Naming Conventions**
   - [ ] All new C++ classes use `FC` prefixes (`AFC*`, `UFC*`, `F*`).
   - [ ] All new Blueprints use `BP_`/`WBP_` prefixes and live under `/Content/FC/...`.
   - [ ] New input assets follow `IA_` (actions) and `IMC_` (mapping contexts) naming.

- [ ] **Folder Structure**
   - [ ] Source files in `/Source/FC/[Module]/` are placed in the correct modules (Core, Characters, etc.).
   - [ ] Content assets are placed in appropriate subfolders (World/Levels, UI/Overworld, Characters, etc.).

### 0005-Z.4: Output Log Review

- [ ] Launch PIE, perform a full Office → Overworld → Camp → Overworld → Office loop.
- [ ] Review Output Log for:
   - [ ] No `Error:` messages except intentional test cases.
   - [ ] No `Warning:` messages for missing assets or null pointers from new Camp/fog-of-war code.
   - [ ] Presence of expected log messages from new Camp/fog-of-war transitions for easier debugging.

---

## 0005-V: Git Repository Hygiene

### 0005-V.1: Working Tree Cleanup

- [ ] Run from repo root:

   ```powershell
   cd <RepoPath>
   git status
   ```

- [ ] Verify:
   - [ ] No unintended `.uasset`/`.umap` changes outside expected folders.
   - [ ] No temporary or log files added outside `/Saved`.
   - [ ] No engine or plugin files accidentally modified.

### 0005-V.2: Stage & Commit Week 5 Changes

- [ ] Stage changes:

   ```powershell
   git add Source/ Content/ Docs/
   git status
   ```

- [ ] Commit with a scoped message, e.g.:

   ```powershell
   git commit -m "feat(0005): add camp scene, transitions and fog-of-war stub"
   ```

### 0005-V.3: Documentation Updates

- [ ] Update `/Docs/Technical_Documentation.md` with:
   - [ ] New/updated classes (Camp pawn, fog-of-war helpers, transition hooks).
   - [ ] Updated call flows for Set Up Camp / Break Camp.
- [ ] Update `/Docs/DRM_Draft.md` to reflect Week 5 implementation status.
- [ ] Commit documentation updates if not already included.

---

## Known Issues & Backlog (0005)

> Use this section to capture any limitations, visual polish items, or follow-up work discovered during Week 5 that should be addressed in later sprints (e.g., Week 6+).

- **Camp Biomes & Landscape Detail**
   - **Issue**: Camp currently uses a generic layout and does not yet visually reflect Overworld biomes (woods vs. mountains).
   - **Planned Resolution**: Week 6 – Camp Biome Variants & Landscape Reflection.

- **Per-Level Minimap Textures**
   - **Issue**: Only a prototype minimap is used for the active Overworld level; 256 per-level textures/captures are not yet implemented.
   - **Planned Resolution**: Week 6 – Overworld Minimap – Level-Based Rendering.

- **Camera-Based Reveal Radius**
   - **Issue**: Fog-of-war reveal is still convoy-radius-based; not yet driven by camera viewport/sight.
   - **Planned Resolution**: Week 6 – Fog-of-War – Camera-Based Reveal Refinement.

- **Quick-Save / SaveGame Integration**
   - **Issue**: Exploration state persistence is in-memory only; no quick-save hooks are wired to transitions.
   - **Planned Resolution**: Week 6 – Quick-Save Hooks for Transitions.

- **Camp Explorer Walk Animations Not Playing**
   - **Issue**: Explorer character moves correctly in Camp with AI controller + SimpleMoveToLocation, but walk animations don't play because `Acceleration = 0.00` while `Velocity = 300.00`. The Animation Blueprint's `ShouldMove` condition checks for acceleration > 0, which SimpleMoveToLocation's pathfinding doesn't set the same way as direct movement input (AddMovementInput).
   - **Log Evidence**: `LogFCExplorerCharacter: Velocity: 300.00 | Acceleration: 0.00` and `LogFCExplorerCharacter: ABP ShouldMove: FALSE`
   - **Root Cause**: SimpleMoveToLocation uses UE's nav system pathfinding which calculates movement internally without directly calling AddMovementInput on every frame, resulting in zero reported acceleration despite active movement.
   - **Planned Resolution**: Week 6 – Investigate alternative movement approach (direct AddMovementInput with custom pathfinding) or modify Animation Blueprint to use velocity-based `ShouldMove` condition instead of acceleration-based. May also require custom AI controller movement component settings or manual acceleration calculation.
   - **Workaround**: Character slides correctly to destination; gameplay functional but visual polish missing.

---

## Task 0005 Complete ✅

**Acceptance Criteria Met**

- [ ] Functional `L_Camp` level with Point & Click explorer control.
- [ ] Stable Overworld ↔ Camp transitions preserving expedition/convoy state.
- [ ] Overworld fog-of-war stub revealing around convoy and masking unseen terrain with session persistence.
- [ ] ESC key and Abort button behavior fixed for Overworld without breaking Office or Camp.

**Next Steps**

- Proceed to **Week 6 – Camp Biomes, Overworld Minimap & Saves** to deepen immersion, generate per-level minimaps, connect fog-of-war to camera sight, and implement quick-save hooks and SaveGame integration.
