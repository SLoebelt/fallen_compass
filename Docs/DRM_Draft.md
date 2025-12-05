# Fallen Compass - Development Roadmap (Draft v2.0)

**Date**: November 23, 2025  
**Status**: DRAFT - Updated based on Week 3 completion  
**Sprint Cadence**: 1 Week (15-20 hours/week)  
**Total Duration**: 40 Weeks

---

## Phase 1 – MVP / Prototype (Weeks 1–8)

**Goal**: A thin but complete **vertical slice**: Start in office → plan simple route → Overworld travel → enter camp → simple combat → finish expedition with a basic report.

### ✅ Week 1 – Project Skeleton & First-Person Office Basics

**Status**: COMPLETE (14.11.-19.11.2025)

- Project Setup, Git, Core Classes.
- First-Person Controller (WASD + Look).
- L_Office Greybox & Lighting.
- In-World Main Menu System.

### ✅ Week 2 – Office Scene & Map Table Interaction

**Status**: COMPLETE (19.11.-21.11.2025)

- Expedition Data Model & Persistence.
- Table Object Interaction System.
- Map Table Widget (Placeholder).
- Level Transition Architecture.
- **Refactoring**: Input Manager Component (Priority 6).

### ✅ Week 3 – Overworld Level & Basic Convoy Movement

**Status**: COMPLETE (21.11.-23.11.2025)

- Overworld Level (Terrain, Camera).
- Convoy Movement (NavMesh, Click-to-Move).
- POI Interaction Stubs.
- Conditional Engine Pause System.

---

### Week 4 – Expedition Loop & Bug Fixing - (23.11.25 - 03.12.25)

**Focus**: Closing the loop between Office and Overworld, and fixing critical state bugs.
**Estimated Time**: 18-22 Hours

- **Feature: Expedition Flow – Part 1 (Loop)**

  - Implement the full transition: Office → Overworld → Return to Office.
  - Create a simple "Expedition Ended" summary screen (placeholder data).
  - **Goal**: Player can start, travel, and return without restarting the game.
  - _Est: 6h_

- **Feature: Overworld Map Widget – Part 1 (Minimap)**

  - Simple 2D overlay showing player position relative to world bounds.
  - Toggle with 'M' key.
  - _Note_: Fog-of-War logic deferred to Week 5 to make room for bug fixes.
  - _Est: 6h_

- **Bug Fixes (Critical & Polish)**
  - **Fix Issue #1 (High)**: Office State Reset on Return. Ensure `InitializeMainMenu` respects existing gameplay state. - FIXED
  - **Fix Issue #4 (Medium)**: ESC Key Toggle in Overworld. Implement `OnKeyDown` override in Pause Menu (`WBP_PauseMenu`). - OPEN
  - **Fix Issue #2 (Low)**: Abort Button Visibility. Hide button when in Office (Abort only visible on `L_Overworld`). - OPEN
  - _Est: 6-8h_

**Implementation Summary**
4.0 Analysis & Discovery: Audited game state, UI, map, and interaction code to understand why Office/Overworld transitions and pause behavior were breaking, and produced a concrete implementation plan aligned with code conventions.

4.2 World Map & Expedition Planning System: Built the new WBP_WorldMap planning UI with fog-of-war and route overlays driven by UFCExpeditionManager, configured a CPU-readable land-mask texture and GameInstance-backed world-map settings, and wired the convoy to feed live exploration data into a persistent fog-of-war grid.

4.2.P1 Core Loop & Data Consistency: Centralized world-map configuration in UFCExpeditionManager::Initialize, mapped convoy world positions into 16×16 exploration cells, and extended expedition data so planned routes, costs, and risk are consistently tracked and autosaved between C++ and UMG.

4.2.P2 Resources & Start-Expedition Flow: Added a small money API to UFCGameInstance, surfaced Money/Supplies in the planning UI, validated route affordability, and implemented a C++ “Start Expedition” flow that consumes funds, marks the expedition in progress, and jumps into the Overworld via the state/level managers.

4.2.P3 Overworld View Map & UX Polish: Created a lightweight WBP_OverworldMap HUD wrapper for a view-only world map, bound it to the M key while traveling, and differentiated planning vs. view modes plus improved on-screen feedback and logging for failed expedition actions.

4.3 Expedition Return Flow: Introduced an Overworld extraction point and an UFCExpeditionSummaryWidget-based summary screen, added an ExpeditionSummary game state and loading hop, and wired ESC/close to cleanly return players from summary back into Office exploration.

4.6 Centralize Level & State Transitions: Implemented UFCLevelTransitionManager as the orchestration hub for “state + level + fade + UI” flows (start expedition, return to main menu, return from Overworld with summary), moved key transitions out of widgets/controller, and added an Office-level startup hook that resolves loading hops like “return with summary” into the correct UI.

4.7 Align Core Managers & Transition Pattern: Documented clear responsibilities across GameState/Level/Transition/UI/LevelTransition managers and the player controller, sketched a reusable transition API and table-view state flow, and planned a unified InitializeOnLevelStart plus migration checklist so future refactors can route all complex transitions through UFCLevelTransitionManager instead of ad-hoc calls.

---

### Week 5 – Local Camp Scene & Fog-of-War (Mechanics Stub) - (03.12.25 - 10.12.25)

**Focus**: The third gameplay perspective (Camp) and a first playable fog-of-war stub; keep scope to one junior week by postponing biomes, advanced minimap generation, and save/load internals to Week 6.
**Estimated Time**: 16-20 Hours

- **Feature: Local Scene: Camp – Part 1 (Basic Scene & Controls)**

  - Create a base `L_Camp` level (small terrain, campfire prop, simple props) that serves as the **generic camp layout** for all biomes.
  - Implement a dedicated **Camp leader pawn** (always the designated explorer character, not “first convoy member”) with **Point & Click** movement, mirroring Overworld input: **LMB = move**, **RMB = interact**, and **LMB directly on interactable = queued interaction on overlap**, reusing the existing Overworld input logic.
  - _Est: 8h_

- **Feature: Scene Transitions – Part 1 (Set Up / Break Camp Hooks)**

  - Implement a **"Set Up Camp"** action callable from **HUD and a keyboard shortcut** while on **Overworld** (no POI interaction requirement) that routes through the existing level/state transition managers and loads `L_Camp`.
  - Implement a **"Break Camp"** interaction in `L_Camp` via an interactable exit area near the **bottom-right** of the camp, plus matching HUD/shortcut option, that transitions back to Overworld and restores convoy position/state via the transition managers.
  - _Est: 4h_

- **Feature: Fog-of-War – Stub (Overworld Reveal Radius)**
  - Implement simple reveal of the **16×16 subgrid cells (~125m×125m each)** around the convoy while moving in Overworld, using the existing exploration grid.
  - Mask out unseen areas on the existing World Map/Minimap using the exploration mask; initially use a **single prototype minimap** for the active Overworld level (full per-level minimaps are deferred to Week 6).
  - Provide **basic in-memory session persistence** (revealed areas survive simple Overworld reloads during the session; full save/load is deferred).
  - _Est: 4-6h_

- **Bug Fixes (Critical & Polish)**
  - **Fix Issue #4 (Medium)**: ESC Key Toggle in Overworld. Revisit input handling so ESC toggles pause reliably; move implementation from widget `OnKeyDown` into the Enhanced Input / PlayerController + `UFCGameStateManager` pattern. - OPEN
  - **Fix Issue #2 (Low)**: Abort Button Visibility. Hide/disable Abort in Office and Camp; ensure it is only visible/active on `L_Overworld` as per design. - OPEN
  
---

### Week 6 – Camp Biomes, Overworld Minimap & Saves

**Focus**: Make camp scenes reflect Overworld biomes, generate per-level minimaps, refine camera-based fog-of-war reveal, and introduce first quick-save hooks for key transitions.
**Estimated Time**: 18-22 Hours

- **Feature: Camp Biome Variants & Landscape Reflection**

  - Extend the base `L_Camp` into a set of **biome variants** (e.g. woods, mountains, plains) or a **parametrized camp scene** that visually reflects the Overworld landscape where camp was started (woods camp if camping in woods, mountain camp if camping in mountains, etc.).
  - Implement the data and transition plumbing so the **"Set Up Camp"** action receives information about the current Overworld biome/region and selects or configures the matching camp variant on load.
  - _Est: 6-8h_

- **Feature: Overworld Minimap – Level-Based Rendering**

  - Decide on a strategy for **per-Overworld-level minimaps** (each of the 256 grid cells ~2km×2km):
    - either an **automated capture workflow** (editor-only render pass or capture blueprint) that renders minimaps based on actual level landscape,
    - or a **convention for authoring static textures** where automation is not feasible.
  - Implement the chosen workflow and hook **per-level minimap textures** into the existing Overworld Map/Minimap widgets.
  - _Est: 6-8h_

- **Feature: Fog-of-War – Camera-Based Reveal Refinement (Stub Extension)**
  - Extend the Week 5 reveal logic so that **everything currently visible in the Overworld camera viewport** (based on camera pan/zoom on `AFCPlayerController` / camera manager) is revealed, effectively tying reveal radius to sight/camera settings.
  - Ensure the reveal grid (16×16×16×16) still maps correctly to ~125m×125m world cells and that camera zoom changes adjust the reveal footprint coherently.
  - _Est: 4-6h_

- **Feature: Quick-Save Hooks for Transitions (Initial Save System)**
  - Add a **lightweight quick-save call** on key level transitions and actions, starting with **"Set Up Camp"**, **"Break Camp"**, and the existing Office↔Overworld transitions (through `UFCTransitionManager` / `UFCGameInstance`).
  - Introduce a **minimal SaveGame format** (building on `FCSaveGame` / `UFCGameInstance`) that can reliably capture at least: expedition state, Overworld convoy position, and current exploration mask.
  - _Est: 4-6h_

---

### Week 7 – Combat Refinement & Core Systems

**Focus**: Finishing combat loop and adding time/resource pressure.
**Estimated Time**: 18-20 Hours

- **Feature: Combat Encounter – Part 2 (Flow)**

  - Trigger combat from Overworld/Camp.
  - Win/Loss condition detection.
  - Return to previous level after combat.
  - _Est: 8h_

- **Feature: Time & Resources – Part 1**

  - **Time**: Global Day Counter in GameInstance. Advance time on travel distance.
  - **Resources**: Single "Supplies" integer. Consume per distance traveled.
  - Update HUD to show Day/Supplies.
  - _Est: 6h_

- **Refactoring: Priority 7 (SaveGame Versioning)**
  - Implement version tracking in SaveGame object.
  - Essential before MVP testing to prevent save corruption during iteration.
  - _Est: 4h_

---

### Week 8 – MVP Vertical Slice & Polish

**Focus**: Tying it all together into a playable build.
**Estimated Time**: 15-18 Hours

- **Feature: Expedition Start Trigger (The Glass)**

  - Implement `BP_TableObject_Glass`.
  - "Point of No Return" confirmation UI.
  - Diegetic transition to Overworld.
  - _Est: 5h_

- **Feature: Expedition Report – Part 2**

  - Populate the End Screen with real data: Days passed, Supplies consumed, Combat result.
  - _Est: 4h_

- **MVP Polish & Bugfixing**
  - Fix critical blockers.
  - Ensure lighting and transitions are smooth.
  - **Milestone**: Tag build `v0.1-MVP`.
  - _Est: 6-9h_

---

## Phase 2 – Demo Release (Weeks 9–26)

**Goal**: Turn prototype into a public demo. Deepen systems (Crew, Risk, Events).

### Week 9 – Route Planning UI

- **Feature**: Two-Phase Route System (Travel to Start vs. Expedition).
- **UI**: Widget for selecting transport method (Cost vs. Risk).
- _Est: 15h_

### Week 10 – Risk Model & Calculation

- **Feature**: Numeric Risk Score calculation based on route.
- **UI**: Tooltips explaining risk factors.
- **Task**: Wire `PlannedRiskCost` into a minimal negative-event hook (e.g. on arrival/return) without full event content.
- _Est: 15h_

### Week 11 – Resource System Expansion

- **Feature**: Split "Supplies" into Food (Daily) + Supplies (Events/Travel).
- **Feature**: Travel Consumption logic update.
- _Est: 15h_

### Week 12 – Camp Tasks

- **Feature**: Assign crew to tasks (Hunt, Guard, Rest).
- **Feature**: End-of-Day resolution logic.
- _Est: 18h_

### Week 13 – Crew Model & Hiring

- **Feature**: `UFCExpeditionCrew` data structure.
- **UI**: Messages Hub (Letters) for recruiting.
- _Est: 20h_

### Week 14 – Injuries & Fatigue

- **Feature**: Injury system (Light/Heavy wounds).
- **Feature**: Fatigue accumulation and Rest effects.
- _Est: 15h_

### Week 15 – Scouts & Vision

- **Feature**: Send scouts to reveal Fog-of-War.
- **Feature**: Persistent Fog-of-War (Save/Load support).
- _Est: 15h_

### Week 16 – Route Events (System)

- **Feature**: Event trigger system based on Risk Score.
- **UI**: Event presentation widget (Text + Image + Options).
- **Task**: Elevate the `PlannedRiskCost`-based hook from Week 10 into a reusable encounter system (resource loss, injuries, delays) that runs during or after expeditions.
- _Est: 18h_

### Week 17 – Combat Abilities

- **Feature**: Player active abilities (Skill shots, Buffs).
- **AI**: Basic Enemy AI behavior tree integration.
- _Est: 20h_

### Week 18 – Combat Integration

- **Feature**: Combat results affect Crew (Death/Injury).
- **Feature**: Loot screen.
- _Est: 15h_

### Week 19 – Failure States

- **Feature**: Game Over conditions (Crew dead, Supplies 0).
- **Feature**: Emergency Return option.
- _Est: 15h_

### Week 20 – Difficulty & Content Writing (Start)

- **Feature**: Difficulty Modes (Casual/Hardcore).
- **Content**: **Start writing Route Events (Batch 1).**
- _Est: 15h_

### Week 21 – Reputation & Content Writing

- **Feature**: Reputation & Finances tracking.
- **Content**: **Start writing POI Encounters (Batch 1).**
- _Est: 15h_

### Week 22 – Persistent World Map

- **Feature**: Map remembers visited locations across expeditions.
- **Content**: **Design Region Layout & POI Placement.**
- _Est: 15h_

### Week 23 – UI Polish & Content Writing

- **Feature**: HUD Polish (Exploration & Camp).
- **Content**: **Write Camp Events.**
- _Est: 15h_

### Week 24 – Audio/Visual & Refactoring

- **Feature**: Audio implementation (Ambience, UI SFX).
- **Refactoring**: Priority 8 (Performance & Polish).
- _Est: 18h_

### Week 25 – Demo Content Integration

- **Focus**: Assembling the curated region using content created W20-23.
- **Feature**: Onboarding/Tutorial hints.
- _Est: 20h_

### Week 26 – Demo Release Prep

- **Feature**: Stability fixes.
- **Milestone**: Tag build `v0.2-Demo`.
- _Est: 20h_

---

## Phase 3 – Early Access Release (Weeks 27–40)

**Goal**: Content expansion, Hardcore mode, Platform integration.

### Week 27 – Content Expansion (Region 2)

- **Feature**: New Biome/Region setup.
- **Feature**: New Enemy Type AI.
- _Est: 18h_

### Week 28 – Equipment System

- **Feature**: Assign gear to crew slots.
- **Feature**: Region 2 Local Scenes.
- _Est: 18h_

### Week 29 – Advanced Events

- **Feature**: Branching Event chains.
- **Feature**: Multi-stage quests.
- _Est: 15h_

### Week 30 – Narrative & Platform Research

- **Feature**: Expedition Log UI.
- **Task**: **Research Steam/Platform SDK requirements.**
- _Est: 15h_

### Week 31 – Advanced Camp

- **Feature**: Upgradeable camp facilities.
- **Feature**: Healing mechanics depth.
- _Est: 18h_

### Week 32 – Economy

- **Feature**: Trading posts in villages.
- **Feature**: Resource exchange rates.
- _Est: 15h_

### Week 33 – Hardcore Mode & Platform Prototype

- **Feature**: Permadeath logic implementation.
- **Task**: **Prototype basic Platform Integration (Achievements/Overlay).**
- _Est: 18h_

### Week 34 – Balancing

- **Feature**: Global balancing pass (Economy, Combat, Risk).
- **Feature**: Debug/Telemetry tools.
- _Est: 15h_

### Week 35 – UI Polish Round 2

- **Feature**: Accessibility options.
- **Feature**: Tooltips and clarity pass.
- _Est: 15h_

### Week 36 – Performance

- **Feature**: Optimization (LODs, Tick rates).
- **Feature**: Save/Load robustness check.
- _Est: 18h_

### Week 37 – Platform Integration

- **Feature**: Full Steamworks (or equivalent) integration.
- **Feature**: Build pipeline automation.
- _Est: 20h_

### Week 38 – Tutorial & Marketing

- **Feature**: Dedicated Tutorial Expedition.
- **Feature**: Credits & Main Menu polish.
- _Est: 15h_

### Week 39 – Localization & QA

- **Feature**: Localization Framework (EN/DE).
- **Focus**: Critical Bug Fixes.
- _Est: 20h_

### Week 40 – Release Candidate

- **Focus**: Final testing.
- **Milestone**: Early Access Release `v1.0-EA`.
- _Est: 20h_
