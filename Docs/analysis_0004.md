# Task 0004 – World Map & Expedition Loop – Open Gaps Only

Date: 2025-12-03

This document now only tracks **remaining gaps** for Task 0004 and their impacts. Completed work and detailed fit descriptions have been removed to keep the file focused.

---

## Priority 4 – GDD Alignment & Future Hooks

### P4.1 Route Risk → Negative Events

**Gap**

- There is still no system that consumes `PlannedRiskCost` to drive negative events during or after expeditions:
  - No event triggers on arrival at the start point or after route completion that interpret the risk value.
  - No integration yet with resource loss, injuries, or delay mechanics mentioned in the GDD.

**Impact**

- This does **not** block the Week 4 milestone (Task 4.2), but:
  - The currently-computed risk is informational only; it has no gameplay consequences.
  - Future work on the expedition travel loop and summary screen will need to introduce a small event/encounter system to realize the GDD’s intention.
