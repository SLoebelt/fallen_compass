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

### Week 4 – Expedition Loop & Bug Fixing

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
  - **Fix Issue #1 (High)**: Office State Reset on Return. Ensure `InitializeMainMenu` respects existing gameplay state.
  - **Fix Issue #4 (Medium)**: ESC Key Toggle in Overworld. Implement `OnKeyDown` override in Pause Menu.
  - **Fix Issue #2 (Low)**: Abort Button Visibility. Hide button when in Office.
  - _Est: 6-8h_

---

### Week 5 – Local Camp Scene & Fog-of-War

**Focus**: The third gameplay perspective (Camp) and map visibility.
**Estimated Time**: 16-20 Hours

- **Feature: Local Scene: Camp – Part 1**

  - Create `L_Camp` level (small terrain, campfire prop).
  - Implement **Point & Click** movement for a single leader pawn (distinct from Convoy controller).
  - _Est: 8h_

- **Feature: Scene Transitions – Part 1**

  - Add "Enter Camp" interaction to Overworld POI.
  - Add "Break Camp" (Return to Overworld) interaction in Camp level.
  - Ensure player position is preserved in Overworld upon return.
  - _Est: 4h_

- **Feature: Fog-of-War – Stub (Moved from Week 4)**
  - Implement simple "reveal radius" around convoy in Overworld.
  - Mask out unseen areas on the Minimap/World Map.
  - Basic session persistence (remember revealed areas until game restart).
  - _Est: 4-6h_

---

### Week 6 – Combat Framework Integration (Start)

**Focus**: Integrating the Advanced Turn-Based Tile Toolkit (ATBT). **High Risk Week.**
**Estimated Time**: 20 Hours

- **Feature: Combat Framework – Part 1 (Setup)**

  - Import ATBT Toolkit.
  - Create a custom `FC_GridManager` extending toolkit functionality.
  - Set up a basic combat map (`L_Combat_Test`).
  - _Est: 8h_

- **Feature: Basic Encounter**
  - Configure Player Unit and Enemy Unit (Placeholder meshes).
  - Implement basic turn order: Player Move/Attack -> Enemy Move/Attack.
  - _Est: 12h_

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
