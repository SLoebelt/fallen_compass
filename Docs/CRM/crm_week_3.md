# Fallen Compass - Cost & Resource Management (CRM)

**Document Version**: 1.0 Draft  
**Created**: November 23, 2025  
**Last Updated**: November 23, 2025  
**Status**: DRAFT - Pending Review

---

## Executive Summary

This document provides time estimations, resource allocation, and risk assessment for the Fallen Compass project based on analysis of the Development Roadmap (DRM), Technical Documentation, Backlog, and Refactoring Analysis.

### Current Status (End of Week 3)

- **Weeks Completed**: 3 of 40 (7.5% of total timeline)
- **Phase Progress**: Week 3 of 8 in Phase 1 (MVP/Prototype) = 37.5% of Phase 1
- **Actual Time Invested**: ~30-35 hours (Weeks 1-3)
- **Ahead of Schedule Items**: Priority 6 (Input Manager) completed in Week 2 instead of Week 3
- **On Track**: Core architecture solid, all Week 1-3 features complete and tested

### Key Findings

1. **Week 3 Completion Rate**: Excellent - all tasks complete with comprehensive testing
2. **Technical Debt**: Well-managed - 4 known issues documented in backlog, none critical
3. **Architecture Quality**: Strong - subsystem pattern working well, components reusable
4. **Remaining Refactorings**: 2 priorities (7-8) deferred to appropriate timeline slots
5. **Risk Level**: **LOW** - project on solid foundation with clear path forward

---

## Table of Contents

1. [Time Investment Analysis](#time-investment-analysis)
2. [Phase Breakdown & Estimations](#phase-breakdown--estimations)
3. [Weekly Time Estimates](#weekly-time-estimates)
4. [Critical Path Analysis](#critical-path-analysis)
5. [Resource Requirements](#resource-requirements)
6. [Risk Assessment](#risk-assessment)
7. [Recommendations & DRM Adjustments](#recommendations--drm-adjustments)
8. [Backlog Priority Mapping](#backlog-priority-mapping)

---

## Time Investment Analysis

### Completed Work (Weeks 1-3)

| Week            | Features Completed                                                                                                         | Estimated Hours | Actual Hours | Variance                 |
| --------------- | -------------------------------------------------------------------------------------------------------------------------- | --------------- | ------------ | ------------------------ |
| **Week 1**      | Project setup, First-person controller, Office level, Main menu system, Refactoring Priority 1-2                           | 12-15h          | ~12h         | On target                |
| **Week 2**      | Expedition data model, Table interaction system, Map widget, Level transitions, Persistent state, Refactoring Priority 3-6 | 18-22h          | ~15h         | **Faster than expected** |
| **Week 3**      | Overworld level, Top-down camera, Convoy movement, POI system, Pause mechanics, Testing                                    | 15-18h          | ~10h         | **Ahead of schedule**    |
| **Refactoring** | Priorities 1-6 (subsystems, components, state machine)                                                                     | 25h planned     | ~25h         | On target                |
| **TOTAL**       | Weeks 1-3 + Refactoring                                                                                                    | **70-80h**      | **~62h**     | **8-18h under budget**   |

**Key Observations**:

- Week 2 efficiency gains from good Week 1 architecture
- Week 3 leveraged existing subsystems effectively (UFCInputManager, UFCCameraManager)
- Refactoring investment paying off in reduced implementation time
- Ahead of schedule by ~1 week equivalent of work

---

## Phase Breakdown & Estimations

### Phase 1 – MVP / Prototype (Weeks 1-8)

**Goal**: Thin vertical slice - Office → Route Planning → Overworld → Camp → Combat → Report

**Time Budget**: 8 weeks @ 10-15h/week = **80-120 hours**

| Week          | Features                                               | Estimated Hours | Complexity | Dependencies |
| ------------- | ------------------------------------------------------ | --------------- | ---------- | ------------ |
| ✅ **Week 1** | Project setup, FPS controller, Office, Main menu       | 12-15h          | Medium     | None         |
| ✅ **Week 2** | Table interaction, Expedition data, Level transitions  | 15-18h          | High       | Week 1       |
| ✅ **Week 3** | Overworld level, Convoy, POI system, Pause             | 12-15h          | High       | Week 2       |
| **Week 4**    | Overworld map widget, Fog-of-war stub, Expedition flow | 15-20h          | High       | Week 3       |
| **Week 5**    | Camp scene, Point-and-click, Scene transitions         | 18-22h          | Very High  | Week 4       |
| **Week 6**    | Combat framework (ATBT Toolkit), Encounter triggers    | 20-25h          | Very High  | Week 5       |
| **Week 7**    | Time system, Resource system expansion                 | 12-15h          | Medium     | Week 6       |
| **Week 8**    | Expedition start trigger (Glass), MVP polish, Testing  | 15-18h          | Medium     | Week 7       |
| **SUBTOTAL**  | Phase 1 Complete                                       | **119-148h**    | -          | -            |

**Current Progress**: 62h spent, 57-86h remaining in Phase 1

**Risk Assessment**: **LOW** - On track, architecture solid, clear path forward

---

### Phase 2 – Demo Release (Weeks 9-26)

**Goal**: Public demo with meaningful route planning, crew/resources, combat depth, difficulty modes

**Time Budget**: 18 weeks @ 12-18h/week = **216-324 hours**

| Week Range      | Feature Categories                                                    | Estimated Hours | Complexity |
| --------------- | --------------------------------------------------------------------- | --------------- | ---------- |
| **Weeks 9-10**  | Route planning UI, Risk model, Calculation widget                     | 28-35h          | High       |
| **Weeks 11-12** | Resource expansion (Food/Supplies), Camp tasks, End-of-day            | 30-38h          | High       |
| **Weeks 13-14** | Crew model, Messages hub, Injuries & fatigue                          | 35-42h          | Very High  |
| **Weeks 15-16** | Scouts, Vision, Fog-of-war v2, Route events                           | 28-35h          | High       |
| **Weeks 17-18** | Combat abilities, Enemy AI, Feedback, Crew integration                | 38-45h          | Very High  |
| **Weeks 19-20** | Failure states, Return flow, Difficulty modes, Death consequences     | 32-40h          | High       |
| **Weeks 21-22** | Reputation, Finances, Outcome reports, World map persistence, Logbook | 30-38h          | High       |
| **Weeks 23-24** | UI/UX polish (HUD, Office), Audio, Visual first pass                  | 28-35h          | Medium     |
| **Weeks 25-26** | Demo content (curated region), Onboarding, Stability, Packaging       | 35-42h          | High       |
| **SUBTOTAL**    | Phase 2 Complete                                                      | **284-350h**    | -          |

**Risk Assessment**: **MEDIUM** - Depends heavily on toolkit integration (ATBT), crew system complexity

---

### Phase 3 – Early Access Release (Weeks 27-40)

**Goal**: EA-ready build with expanded content, deeper systems, full Hardcore mode

**Time Budget**: 14 weeks @ 15-20h/week = **210-280 hours**

| Week Range      | Feature Categories                                                  | Estimated Hours | Complexity |
| --------------- | ------------------------------------------------------------------- | --------------- | ---------- |
| **Weeks 27-28** | Region 2 content, New enemies, Equipment system                     | 32-40h          | High       |
| **Weeks 29-30** | Event system depth (branching), Narrative flavor, Log UI            | 28-35h          | Medium     |
| **Weeks 31-32** | Advanced camp management, Resource types, Economy/trading           | 35-42h          | High       |
| **Weeks 33-34** | Hardcore mode v2, Meta-progression, Balancing, Debug tools          | 30-38h          | High       |
| **Weeks 35-36** | UI polish round 2, Accessibility, Performance, Save/load robustness | 32-40h          | Medium     |
| **Weeks 37-38** | EA packaging, Platform integration, Tutorial v2, Marketing support  | 30-38h          | Medium     |
| **Weeks 39-40** | Release candidate prep, Localization, Bug fixing, Launch            | 35-45h          | High       |
| **SUBTOTAL**    | Phase 3 Complete                                                    | **222-278h**    | -          |

**Risk Assessment**: **MEDIUM-HIGH** - Packaging/platform integration can be unpredictable

---

## Total Project Estimation

### Summary Table

| Phase              | Weeks        | Estimated Hours | Status                                 |
| ------------------ | ------------ | --------------- | -------------------------------------- |
| **Phase 1 (MVP)**  | 1-8          | 119-148h        | ✅ Week 3/8 complete (62h spent)       |
| **Phase 2 (Demo)** | 9-26         | 284-350h        | Not started                            |
| **Phase 3 (EA)**   | 27-40        | 222-278h        | Not started                            |
| **Refactoring**    | Ongoing      | 28-32h          | ✅ Priorities 1-6 complete (25h spent) |
| **TOTAL PROJECT**  | **40 weeks** | **653-808h**    | **~10% complete**                      |

### Time Allocation by Work Type

| Work Type                               | Estimated Hours | Percentage |
| --------------------------------------- | --------------- | ---------- |
| Core Feature Development                | 450-550h        | 69%        |
| UI/UX Polish & Iteration                | 80-100h         | 12%        |
| Content Creation (levels, POIs, events) | 60-80h          | 10%        |
| Refactoring & Architecture              | 28-32h          | 4%         |
| Testing & Bug Fixing                    | 35-46h          | 5%         |
| **TOTAL**                               | **653-808h**    | **100%**   |

### Weekly Commitment Analysis

**Average hours per week**: 653-808h ÷ 40 weeks = **16-20 hours/week**

**Current pace (Weeks 1-3)**: 62h ÷ 3 weeks = **~21 hours/week** ✅ Sustainable and on target

**Recommended target**: **15-18 hours/week** average

- Allows for buffer weeks (holidays, unexpected delays)
- Sustainable for solo developer
- Matches current productivity

---

## Critical Path Analysis

### Must-Have for Each Milestone

#### Phase 1 MVP (Week 8) - Critical Features

**Blockers** (Cannot skip):

1. ✅ Office → Overworld transition (Complete)
2. ✅ Convoy movement (Complete)
3. Camp scene with basic units (Week 5)
4. Combat integration (ATBT toolkit) (Week 6)
5. Time/resource systems (Week 7)
6. Expedition report (Week 8)

**Nice-to-Have** (Can defer):

- Fog-of-war (stub acceptable)
- Multiple POI types (1-2 sufficient)
- Complex route planning (basic placeholder OK)

**Dependencies**:

- Week 5 (Camp) blocks Week 6 (Combat)
- Week 6 (Combat) blocks Week 7 (Resources need consumption context)
- Week 7 (Time/Resources) blocks Week 8 (Report needs data)

**Risk Mitigation**:

- ATBT toolkit integration is HIGH RISK - allocate extra buffer in Week 6
- Consider 2-week buffer: Weeks 6-7 for combat if needed
- Have fallback combat system (simple turn-based without toolkit)

---

#### Phase 2 Demo (Week 26) - Critical Features

**Blockers**:

1. Route planning UI (Weeks 9-10)
2. Crew system with hiring (Week 13)
3. Multi-resource economy (Week 11)
4. Combat depth (abilities, AI) (Weeks 17-18)
5. Failure states & consequences (Weeks 19-20)
6. Demo content (curated region) (Week 25)

**Nice-to-Have**:

- Reputation system (can simplify)
- Advanced route events (start with 5-10)
- Meta-progression (defer to Phase 3)

**Dependencies**:

- Crew system (Week 13) blocks Injuries (Week 14), Combat integration (Week 18)
- Resource system (Week 11) blocks Economy (Week 21)
- Combat depth (Weeks 17-18) blocks proper difficulty tuning (Weeks 19-20)

**Risk Mitigation**:

- Content creation (Week 25) is often underestimated - start earlier
- Begin POI/event writing in Week 20-21 parallel to systems work
- Allocate 2 weeks for iteration after "feature complete" (Weeks 25-26)

---

#### Phase 3 EA Release (Week 40) - Critical Features

**Blockers**:

1. Hardcore mode implementation (Week 33)
2. Save/load versioning (Priority 7 refactoring) (Week 27-28)
3. Packaging pipeline (Week 37)
4. Platform integration (Steam/etc) (Week 37)
5. Tutorial experience (Week 38)
6. Localization framework (Week 39)

**Nice-to-Have**:

- Region 2 content (can launch with 1 region + promise more)
- Advanced equipment system (basic version sufficient)
- Extensive meta-progression (minimal acceptable)

**Risk Mitigation**:

- Packaging/platform work is HIGH RISK - start investigating in Week 30
- Localization can be complex - externalize text early (Week 35)
- Build RC (Release Candidate) by Week 38, not Week 39
- Allocate full 2 weeks for bug fixing (Weeks 39-40)

---

## Resource Requirements

### Solo Developer Time Allocation

**Recommended weekly schedule** (15-18h/week):

| Day       | Hours      | Focus Area                              |
| --------- | ---------- | --------------------------------------- |
| Monday    | 2-3h       | Planning, documentation, Git management |
| Tuesday   | 3-4h       | Core C++ development                    |
| Wednesday | 3-4h       | Blueprint implementation, UI work       |
| Thursday  | -          | Rest / catch-up buffer                  |
| Friday    | 3-4h       | Testing, polish, bug fixing             |
| Saturday  | 4-5h       | Feature completion, integration testing |
| Sunday    | -          | Documentation, planning next week       |
| **Total** | **15-20h** | Sustainable solo pace                   |

### External Resources Needed

**Week 6 (Combat Toolkit)**:

- **Advanced Turn-Based Tile Toolkit** (ATBT) - Already purchased?
- Estimated learning curve: 4-6 hours
- Budget extra time for integration issues

**Week 24 (Audio)**:

- Sound effects pack (Overworld ambience, UI clicks, combat)
- Music tracks (Office, Overworld, Combat)
- Estimated cost: $50-150 (asset store)
- Implementation time: 8-12 hours

**Week 24 (Visual Polish)**:

- VFX pack (dust motes, candle flicker, combat effects)
- Possibly custom icon pack for UI
- Estimated cost: $30-80
- Implementation time: 6-10 hours

**Week 25 (Demo Content)**:

- Potentially hire writer for event text (optional)
- Estimated cost: $100-300 for 20-30 events
- Alternative: Write yourself (adds 10-15h)

**Week 37 (Platform Integration)**:

- Steamworks SDK (free)
- Potential dev/testing accounts
- Estimated cost: $0 (plus $100 Steam Direct fee at launch)

**TOTAL ESTIMATED EXTERNAL COSTS**: $180-530 (conservative)

---

## Risk Assessment

### High-Risk Items

| Risk                                 | Impact                      | Probability  | Mitigation                                          | Timeline   |
| ------------------------------------ | --------------------------- | ------------ | --------------------------------------------------- | ---------- |
| **ATBT Combat Integration**          | CRITICAL - blocks Week 6-8  | Medium (40%) | Prototype integration in Week 5, have fallback plan | Week 6     |
| **Crew System Complexity**           | HIGH - affects Weeks 14-18  | Medium (30%) | Start simple (name + 3 stats), iterate later        | Week 13    |
| **Content Creation Underestimation** | HIGH - affects demo quality | High (60%)   | Start event/POI writing in Week 20, not Week 25     | Week 20-25 |
| **Platform Integration Issues**      | HIGH - can delay EA launch  | Medium (40%) | Start research/testing in Week 30, not Week 37      | Week 30    |
| **Save/Load Corruption**             | MEDIUM - player frustration | Medium (30%) | Implement Priority 7 refactoring early (Week 8-9)   | Week 8     |

### Medium-Risk Items

| Risk                               | Impact                     | Probability  | Mitigation                                      |
| ---------------------------------- | -------------------------- | ------------ | ----------------------------------------------- |
| **Performance Issues (Overworld)** | MEDIUM - affects feel      | Low (20%)    | Profile early (Week 10), optimize in Week 36    |
| **UI Iteration Time**              | MEDIUM - polish takes time | High (70%)   | Allocate 2 polish passes (Weeks 23, 35)         |
| **Scope Creep**                    | MEDIUM - timeline slip     | Medium (50%) | Strict "Nice-to-Have" vs "Must-Have" discipline |
| **Balancing Difficulty**           | MEDIUM - gameplay quality  | Medium (40%) | Early playtesting (Week 15+), iteration cycles  |

### Low-Risk Items (Monitor Only)

- Navigation mesh issues (well-understood tech)
- Camera system complexity (already prototyped)
- State management bugs (architecture is solid)
- Input system conflicts (UFCInputManager working well)

---

## Recommendations & DRM Adjustments

### Critical Recommendations

#### 1. ✅ **Week 4 Scope Reduction** (RECOMMENDED)

**Current DRM**: Week 4 includes Overworld Map Widget + Fog-of-War stub + Expedition Flow

**Issue**: This is 3 distinct features - may be overloaded after heavy Week 3

**Recommendation**:

- **Keep**: Overworld Map Widget (simple placeholder), basic Expedition Flow
- **Defer to Week 5**: Fog-of-War system (even stub version)
- **Rationale**: Week 5 is already "Local Scene: Camp" - can be lighter, move fog-of-war there

**Adjusted Week 4** (15-18h):

- Overworld Map Widget - Part 1 (simple minimap overlay) (8-10h)
- Expedition Flow - Part 1 (Office → Overworld → Return stub) (7-8h)

**Adjusted Week 5** (15-18h):

- Local Scene: Camp - Part 1 (level + control) (10-12h)
- Fog-of-War - Stub (simple reveal circle) (5-6h)

**Impact**: Reduces Week 4 stress, balances Week 5 load, maintains timeline

---

#### 2. ✅ **Week 6 Combat Buffer** (STRONGLY RECOMMENDED)

**Current DRM**: Week 6 is "Combat Framework - Part 1 (ATBT Integration)" (single week)

**Issue**: Toolkit integration is HIGH RISK - historically these take longer than expected

**Recommendation**:

- **Extend to 1.5 weeks**: Week 6 + first half of Week 7
- **Compress Week 7**: Time/Resource systems are simpler, can fit in 0.5 week

**Adjusted Timeline**:

- **Week 6 (20h)**: ATBT Integration, basic combat setup
- **Week 7 Part A (10h)**: ATBT refinement, combat encounter trigger
- **Week 7 Part B (8h)**: Time system + Simple resources

**Impact**: Provides buffer for toolkit learning curve, maintains Phase 1 deadline

---

#### 3. ✅ **Priority 7 Refactoring Early** (RECOMMENDED)

**Current Plan**: Priority 7 (SaveGame Versioning) before Week 8

**Issue**: Will need save system for Week 8 expedition report testing

**Recommendation**:

- **Implement in Week 7** (add 3-4h to week)
- **Rationale**: Week 7 is lighter (see adjustment #2), versioning needed for testing

**Impact**: Better testing in Week 8, avoids save issues in Phase 2

---

#### 4. ✅ **Content Pre-Production Start** (CRITICAL)

**Current DRM**: Week 25 is "Demo Content" (single week)

**Issue**: Writing 20-30 events, designing POIs, testing takes much longer than 1 week

**Recommendation**:

- **Start in Week 20**: Begin event writing parallel to system work
- **Allocate 6 weeks total**: Weeks 20-25 (5-8h/week dedicated to content)
- **Method**: Write events during "lighter" system weeks, integrate in Week 25

**Adjusted Content Timeline**:

- Week 20: Write 5-8 route events (4h)
- Week 21: Write 5-8 POI encounters (5h)
- Week 22: Design region layout, POI placement (4h)
- Week 23: Write 5-8 camp events (4h)
- Week 24: Narrative flavor pass (3h)
- Week 25: Integration, testing, polish (15h)

**Impact**: Demo content quality much higher, reduces Week 25 crunch

---

#### 5. ✅ **Platform Integration Early Start** (RECOMMENDED)

**Current DRM**: Week 37 is "Platform Integration - Part 1"

**Issue**: Steamworks/platform SDKs can have unexpected issues

**Recommendation**:

- **Research in Week 30**: Download SDK, read docs (2-3h)
- **Prototype in Week 33**: Basic integration test (3-4h)
- **Full implementation Week 37**: With buffer for issues (8-10h)

**Impact**: Reduces risk of EA launch delay due to technical issues

---

### Optional Recommendations (Nice-to-Have)

#### 6. 📋 **Playtesting Sessions**

**Recommendation**: Schedule 3 external playtesting sessions:

- **Week 15**: Combat + crew system feedback (2-3 testers)
- **Week 22**: Full expedition loop feedback (3-5 testers)
- **Week 35**: EA build quality check (5-10 testers)

**Time Investment**: 2-3h per session (setup, feedback review)

**Impact**: Catch UX issues early, validate difficulty balance

---

#### 7. 📋 **Documentation Maintenance Weeks**

**Recommendation**: Every 8 weeks, allocate 2-3h for documentation update:

- **Week 8**: Update Technical_Documentation.md with Phase 1 systems
- **Week 16**: Document Phase 2 systems (crew, resources, events)
- **Week 24**: Document UI/UX patterns, audio integration
- **Week 32**: Document combat depth, equipment, economy
- **Week 40**: Final documentation pass for EA launch

**Impact**: Easier onboarding for potential collaborators, better maintenance

---

### DRM Adjustment Summary

**Weeks to Modify**:

1. ✅ **Week 4**: Reduce scope (defer fog-of-war to Week 5)
2. ✅ **Week 5**: Add fog-of-war stub (from Week 4)
3. ✅ **Week 6-7**: Extend combat to 1.5 weeks, compress time system
4. ✅ **Week 7**: Add Priority 7 refactoring (SaveGame versioning)
5. ✅ **Weeks 20-25**: Spread content creation across 6 weeks
6. ✅ **Week 30**: Add platform research task
7. ✅ **Week 33**: Add platform prototype task

**No changes needed**: Weeks 1-3 (complete), Weeks 8-19, 26-29, 31-32, 34-40

**Total timeline impact**: NONE - adjustments maintain 40-week target while reducing risk

---

## Backlog Priority Mapping

### Known Issues from Backlog.md

| Issue                                 | Severity | Target Sprint | DRM Integration                          |
| ------------------------------------- | -------- | ------------- | ---------------------------------------- |
| **Issue #1**: Office State Reset      | Medium   | Week 4-5      | ✅ Add to Week 4 tasks (2-3h)            |
| **Issue #2**: Abort Button Visibility | Low      | Week 4        | ✅ Add to Week 4 polish (15min)          |
| **Issue #3**: Level Loading Order     | Low      | Week 5+       | 📋 Add to Week 23 (UI polish)            |
| **Issue #4**: ESC Toggle Pause Menu   | Low      | Week 4-5      | ✅ Add to Week 4 tasks (Option A: 15min) |

**Recommendation**:

- **Week 4**: Fix Issues #1, #2, #4 (total 3-4h added)
- **Week 5+**: Address Issue #3 during natural refactoring
- **Impact**: Week 4 becomes 18-22h (still manageable)

### Refactoring Priorities Mapping

| Priority         | Description          | Target   | DRM Integration          | Status  |
| ---------------- | -------------------- | -------- | ------------------------ | ------- |
| **Priority 1-6** | Core architecture    | Week 1-2 | ✅ Complete              | ✅ DONE |
| **Priority 7**   | SaveGame Versioning  | Week 8   | ✅ Move to Week 7 (3-4h) | Pending |
| **Priority 8**   | Performance & Polish | Week 12  | ✅ Add to Week 24 (4-6h) | Pending |

---

## Adjusted DRM Sections

### Proposed Changes to Fallen_Compass_DRM.md

#### Week 4 (MODIFIED)

```markdown
### Week 4 – Overworld Map Widget & Basic Expedition Flow

- **Feature: Overworld Map Widget – Part 1 (Simple Minimap)**

  - Create simple minimap overlay showing player position
  - Basic zoom levels (no fog-of-war yet)
  - Estimated: 8-10 hours

- **Feature: Expedition Flow – Part 1 (Office → Overworld → Return Stub)**

  - Implement complete transition loop
  - Basic return-to-office flow
  - Expedition end screen placeholder
  - Estimated: 7-8 hours

- **Bugfix: Known Issues from Week 3**
  - Issue #1: Office State Reset (2-3h)
  - Issue #2: Abort Button Visibility (15min)
  - Issue #4: ESC Toggle in Pause Menu (15min)
  - Estimated: 3-4 hours

**Total Week 4**: 18-22 hours
```

#### Week 5 (MODIFIED)

```markdown
### Week 5 – Local Camp Scene & Fog-of-War Stub

- **Feature: Local Scene: Camp – Part 1 (Level & Control)**

  - Create L_Camp level (basic terrain)
  - Implement Point & Click unit movement (single leader pawn)
  - Estimated: 10-12 hours

- **Feature: Fog-of-War – Stub (Moved from Week 4)**
  - Simple reveal circle around convoy
  - Basic persistence (session only)
  - Estimated: 5-6 hours

**Total Week 5**: 15-18 hours
```

#### Week 6-7 (MODIFIED)

```markdown
### Week 6-7 – Combat Framework & Core Systems (1.5 Week Sprint)

**Week 6 (20 hours)**:

- **Feature: Combat Framework – Part 1 (ATBT Integration)**
  - Install and learn Advanced Turn-Based Tile Toolkit
  - Create basic combat level
  - Implement turn order and unit placement
  - Player vs. single enemy type
  - Estimated: 20 hours

**Week 7 Part A (10 hours)**:

- **Feature: Combat Framework – Part 2 (Refinement)**
  - Combat encounter trigger from Camp/Overworld
  - Win/lose result screen
  - Estimated: 10 hours

**Week 7 Part B (8 hours)**:

- **Feature: Time System – Part 1 (Simple Day Counter)**

  - Day tracking in GameInstance
  - UI display
  - Estimated: 3-4 hours

- **Feature: Simple Resources – Part 1 (Single Supplies Resource)**

  - Supplies consumption per day
  - Supplies HUD element
  - Estimated: 3-4 hours

- **Refactoring: Priority 7 – SaveGame Versioning**
  - Version tracking and migration framework
  - Estimated: 2 hours

**Total Week 6-7**: 38 hours (spread across 2 weeks)
```

---

## Conclusion & Next Steps

### Summary

**Project Health**: ✅ **HEALTHY**

- Ahead of schedule by ~1 week
- Architecture solid and extensible
- Technical debt well-managed
- Clear path to MVP

**Time Investment**: ⚠️ **MANAGEABLE**

- 653-808 total hours estimated
- 16-20h/week average needed
- Current pace: 21h/week ✅ Sustainable

**Risk Level**: 🟡 **MEDIUM**

- High-risk items identified and mitigated
- Combat toolkit integration needs buffer
- Content creation needs early start
- Platform work needs early investigation

### Immediate Next Steps (Week 4)

**Priority 1 - Core Features** (15-18h):

1. Implement Overworld Map Widget (simple minimap) (8-10h)
2. Complete Expedition Flow loop (Office → Overworld → Return) (7-8h)

**Priority 2 - Bugfixes** (3-4h): 3. Fix Issue #1: Office State Reset (2-3h) 4. Fix Issue #2: Abort Button Visibility (15min) 5. Fix Issue #4: ESC Toggle Pause Menu (15min)

**Priority 3 - Planning** (2h): 6. Review Week 5 requirements 7. Research ATBT toolkit documentation 8. Plan camp scene layout

**Total Week 4 Estimate**: 20-24 hours

### Long-Term Actions

**Before Week 8** (Phase 1 completion):

- [ ] Implement Priority 7 refactoring (SaveGame versioning)
- [ ] Complete vertical slice testing
- [ ] Update Technical_Documentation.md
- [ ] Tag MVP build

**Before Week 26** (Demo release):

- [ ] Start content writing in Week 20 (not Week 25)
- [ ] Conduct playtesting sessions (Weeks 15, 22)
- [ ] Implement Hardcore mode foundation
- [ ] Curate demo region content

**Before Week 40** (EA release):

- [ ] Research platform integration (Week 30)
- [ ] Prototype Steam integration (Week 33)
- [ ] Implement localization framework
- [ ] Build release candidate (Week 38)
- [ ] Full QA pass (Weeks 39-40)

---

## Appendix: Time Tracking Template

### Weekly Time Log Template

```markdown
## Week X - [Feature Name]

**Planned Hours**: Xh
**Actual Hours**: Xh
**Variance**: +/-Xh

### Monday (Xh)

- Task 1: Description (Xh)
- Task 2: Description (Xh)

### Tuesday (Xh)

- Task 3: Description (Xh)

... (continue for each day)

### Weekly Reflection

- What went well:
- What took longer than expected:
- What to improve next week:
- Blockers encountered:
```

### Monthly Review Template

```markdown
## Month X Review - [Phase Name]

**Weeks Completed**: X-Y
**Total Hours Invested**: Xh
**Planned vs Actual**: X% variance

### Features Completed

- Feature 1 (estimated Xh, actual Xh)
- Feature 2 (estimated Xh, actual Xh)

### Technical Debt

- Items added to backlog:
- Items resolved:

### Risk Updates

- New risks identified:
- Risks mitigated:

### Adjustments for Next Month

- Scope changes:
- Timeline adjustments:
```

---

**END OF DRAFT CRM**

**Document Status**: Ready for review and approval  
**Next Review Date**: End of Week 4 (November 30, 2025)  
**Owner**: Solo Developer  
**Approver**: Solo Developer
