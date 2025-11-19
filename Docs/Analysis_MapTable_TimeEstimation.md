# Map Table System - Time Estimation Analysis

**Date**: November 19, 2025  
**Scope**: Analysis of time estimates for implementing the complete Map Table interaction system

---

## Feature Breakdown & Sprint Allocation

### Week 2: Foundation (6-7 days)

**Systems Implemented:**

- Table object interaction system (C++ interface + base BP)
- 4 table objects created (Map, Logbook, Letters, Glass)
- Camera focus system for objects
- Basic map widget with placeholder UI
- Expedition data model & manager subsystem
- Level manager subsystem
- Game state foundation

**Complexity**: **Medium**  
**Estimate Confidence**: **High** (builds on Week 1 camera system)

**Why This Week:**

- Establishes architectural patterns used throughout project
- No dependencies on unbuilt systems (Overworld, crew, resources)
- Table object pattern is reusable for all 4 objects

---

### Week 8: Expedition Start Trigger (1-2 days)

**Systems Implemented:**

- Glass object interaction (confirmation dialog)
- Expedition summary display
- Final cost deduction logic
- Phase A travel event sequence (text summary)

**Complexity**: **Low**  
**Estimate Confidence**: **High** (uses existing systems)

**Why This Week:**

- Completes MVP vertical slice
- Minimal new code (dialog widget + flow logic)
- Integrates existing route data from Weeks 9-10

---

### Week 9: Route Planning UI (4-5 days)

**Systems Implemented:**

- Two-phase route system (Travel to Start + Expedition Region)
- Interactive world map in WBP_MapTable
- Transport method selection
- Region/startpoint selection
- RouteData struct & integration

**Complexity**: **Medium-High**  
**Estimate Confidence**: **Medium** (UI layout complexity)

**Why This Week:**

- First major UI feature after MVP complete
- Needs resource system from Week 7 & 11
- No crew dependency yet (uses placeholder crew size)

---

### Week 10: Calculation Widget (3-4 days)

**Systems Implemented:**

- Route risk calculation model
- WBP_RouteCalculator sub-widget
- Provision/supply estimation logic
- Risk level color-coding
- Start button validation

**Complexity**: **Medium**  
**Estimate Confidence**: **High** (pure calculation logic)

**Why This Week:**

- Natural extension of Week 9 route planning
- Needs resource types from Week 11 (Food + Supplies)
- Mathematical calculations are predictable work

---

### Week 13: Messages Hub Part 1 (3-4 days)

**Systems Implemented:**

- WBP_MessagesHub widget (3 tabs)
- Crew applications tab (full implementation)
- Crew hiring system
- Letters object integration
- Crew selection in WBP_MapTable

**Complexity**: **Medium**  
**Estimate Confidence**: **High** (standard UI + data binding)

**Why This Week:**

- Timing: After crew data model created (Week 13 first item)
- Contracts/orders tabs are placeholders (completed Week 21)
- Crew recruitment needed before Week 14 (injuries/fatigue)

---

### Week 21: Messages Hub Part 2 (4-5 days)

**Systems Implemented:**

- Contracts tab (job offers from patrons)
- Orders tab (supply procurement)
- Contract objective integration
- Delivery scheduling system
- Extended expedition report

**Complexity**: **Medium-High**  
**Estimate Confidence**: **Medium** (depends on contract system complexity)

**Why This Week:**

- Timing: After reputation/finances system (Week 21 first item)
- Contracts need reputation system to work
- Orders need full resource/economy system
- Phase 2 polish/content expansion phase

---

### Week 22: Expedition Logbook (2-3 days)

**Systems Implemented:**

- WBP_ExpeditionLog widget
- List view of past expeditions
- Detail view with full summary
- Logbook object integration
- Expedition history retrieval

**Complexity**: **Low-Medium**  
**Estimate Confidence**: **High** (UI + data display)

**Why This Week:**

- Timing: After persistent world map (Week 22 first item)
- Needs archived expedition data (Week 19 report system)
- Simple UI - mostly read-only data display
- Natural fit with persistence features

---

## Total Implementation Time

| Feature                                | Sprint Week | Estimated Days | Complexity  |
| -------------------------------------- | ----------- | -------------- | ----------- |
| Table Object System Foundation         | Week 2      | 6-7            | Medium      |
| Expedition Start Trigger               | Week 8      | 1-2            | Low         |
| Route Planning UI                      | Week 9      | 4-5            | Medium-High |
| Calculation Widget                     | Week 10     | 3-4            | Medium      |
| Messages Hub Part 1 (Crew)             | Week 13     | 3-4            | Medium      |
| Messages Hub Part 2 (Contracts/Orders) | Week 21     | 4-5            | Medium-High |
| Expedition Logbook                     | Week 22     | 2-3            | Low-Medium  |
| **TOTAL**                              | **7 weeks** | **23-30 days** | **Medium**  |

---

## Risk Assessment

### Low Risk Items ✅

- **Table object interaction system** (extends Week 1 patterns)
- **Calculation widget** (pure math, no new systems)
- **Expedition logbook** (read-only data display)
- **Expedition start trigger** (simple dialog + flow)

### Medium Risk Items ⚠️

- **Route planning UI** (complex layout, two-phase system)
- **Messages Hub crew tab** (depends on crew system timing)
- **Messages Hub contracts/orders** (depends on economy depth)

### Mitigations

1. **Week 2 prototype** validates camera focus pattern early
2. **Placeholder tabs** in Week 13 reduce scope creep
3. **Incremental UI** - each widget works independently
4. **GDD specification** provides clear requirements

---

## Dependency Chain

```
Week 1: Camera blend system (foundation)
  ↓
Week 2: Table object interaction + ExpeditionManager
  ↓
Week 7: Resource system (Supplies)
  ↓
Week 9: Route planning UI (uses resources for validation)
  ↓
Week 10: Calculation widget (extends Week 9)
  ↓
Week 11: Resource expansion (Food + Supplies split)
  ↓
Week 13: Crew model → Messages Hub Part 1
  ↓
Week 19: Expedition reports
  ↓
Week 21: Reputation/Finances → Messages Hub Part 2
  ↓
Week 22: Persistent map + Expedition history → Logbook

Week 8: Start trigger (depends on Weeks 9-10 data)
```

---

## Confidence Level: **HIGH**

**Reasoning:**

1. **Architectural foundation** established in Week 1 (camera blending, state management)
2. **Modular design** - each table object is independent
3. **Clear specifications** in updated GDD (§3.1.2 - §3.1.3)
4. **Incremental delivery** - each week produces testable, playable features
5. **No major unknowns** - all systems use proven UE5 patterns
6. **Buffer time** - estimates assume 5-day work weeks with 1-2 day buffer per feature

**Timeline Match**: ✅ **YES**

- Original Phase 1 (Weeks 1-8): MVP with basic table → **Still fits**
- Phase 2 (Weeks 9-26): Full route planning + messages + logbook → **All accounted for**
- No schedule slip required

---

## Recommendations

1. **Week 2 Critical**: Nail the table object interaction pattern - it's the foundation for all 4 objects
2. **Week 9 Prototype Early**: Route UI is most complex - consider doing mockup/wireframe in Week 8
3. **Messages Hub Scope Control**: Keep contracts simple (3-5 types max for EA release)
4. **Logbook Deferred Details**: Narrative summaries can be placeholder text initially
5. **Visual Polish Separate**: Camera angles/lighting for each object can be Phase 3 polish

---

## Conclusion

The **Map Table system** as specified in the GDD is **achievable within the planned timeline**. The 7-week spread (Weeks 2, 8, 9, 10, 13, 21, 22) provides:

- **Natural progression** (foundation → planning → recruitment → economy → history)
- **Realistic scope** per week (2-7 days per feature)
- **Clear dependencies** (each week builds on previous systems)
- **Early validation** (Week 2 proves the pattern)

**No timeline adjustment needed** - current DRM sprint allocation is appropriate.
