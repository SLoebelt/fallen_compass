# PRD — Optional Polish (UI Blocking Queries + Interaction Profiles DataAssets)

## 1) Introduction/Overview

Optional milestone to reduce edge-case bugs and improve maintainability by:

* Centralizing UI blocking logic into explicit, **well-defined** queries
* Introducing interaction probe config as a DataAsset (`UInteractionProfile`)
* Eliminating hidden dependencies by defining **ownership, lifecycle, and update responsibility** for UI blocking state

## 2) Goals

* Remove scattered “is UI open?” checks across controller and interaction.
* Make click/interact gating explicit per modality with **stable semantics**.
* Allow probe tuning without recompiling C++ while preventing DataAsset “feature creep”.
* Prevent click-through and inconsistent input capture by aligning with UE input routing (Enhanced Input / CommonUI where applicable).

## 3) User Stories

* As a player, clicking does nothing when a blocking modal is open, consistently.
* As a developer, I can adjust interaction range/trace settings in a DataAsset.
* As a developer, I can reason about *why* input is blocked (debuggable “blocking reasons”).

## 4) Functional Requirements

### 4.1 UI blocking queries (authoritative + cycle-free)

**4.1.1 UIManager must provide:**

* `CanWorldClick()` (TopDown/Camp click-to-move / click-to-command)
* `CanWorldInteract()` (FirstPerson interact / use)
* (Optional but recommended) `GetWorldInputBlockState()` returning a struct:

  * `bBlocksWorldClick`, `bBlocksWorldInteract`
  * `BlockingReasons` (debug-only, stripped/disabled in Shipping)

**4.1.2 Query semantics must be explicitly defined**

* `CanWorldClick()` returns **false** if *any active UI element* that is classified as “BlocksWorldClick” is present/active.
* `CanWorldInteract()` returns **false** if *any active UI element* that is classified as “BlocksWorldInteract” is present/active.
* Click and interact may be blocked by different widget sets.

**4.1.3 Lifecycle / update responsibility (no hidden polling)**

* UI blocking state must be **event-driven** and owned by UI layer:

  * When a widget becomes active/visible (pushed, activated, opened), it **registers** its blocking flags with UIManager.
  * When it closes/deactivates, it **unregisters**.
* UIManager maintains cached booleans/counters so queries are **O(1)** and do not scan widget trees.

**4.1.4 Widget classification must be concrete**
Each UI widget must declare its blocking behavior via one of:

* A base class (e.g., `UBlockableWidgetBase`) with flags:

  * `bBlocksWorldClick`, `bBlocksWorldInteract`
* Or an interface (e.g., `IUIBlocker`) returning those flags
* Or (if using CommonUI) a CommonActivatableWidget-derived base with these flags

**4.1.5 Controllers/interaction must consult queries**

* Controller click handler must early-out if `!UIManager->CanWorldClick()`
* InteractionComponent / interact input handler must early-out if `!UIManager->CanWorldInteract()`

**4.1.6 UIManager must not call back into controller/interaction**

* No controller references inside UIManager
* UIManager is a pure “readable state” service from gameplay perspective

---

### 4.2 Interaction profile DataAsset (config-only, governed)

**4.2.1 Create `UInteractionProfile` with config-only fields:**

* probe type (LineTrace / SphereTrace / CursorHit / OverlapOnly)
* trace channel / object types
* range / radius
* optional prompt widget class
* allowed tags (optional; keep minimal)
* (Optional) “max hits” / “sort mode” if needed, but **avoid piling on**

**4.2.2 Integrate `UInteractionProfile` into `FPlayerModeProfile`**

* Each player mode selects one interaction profile asset.
* Default asset must exist and be referenced in a safe, central place.

**4.2.3 Prevent DataAsset feature creep**

* Explicit rule: `UInteractionProfile` is **configuration only**, not behavior toggles.
* No accumulating “special-case flags” (e.g., “ignore UI”, “allow through menus”, “only in dungeon”, etc.).
* If a new requirement is behavioral, implement in code or split mode/controllers later.

**4.2.4 Validation + fallback behavior must be defined**

* On load/use: validate DataAsset refs.
* If missing/invalid:

  * Fall back to a known default profile asset **or**
  * Fall back to safe hardcoded defaults
* “Safe default” must prioritize preventing unintended actions:

  * If unsure, **disable interaction** rather than allowing click-through/use.

---

### 4.3 Input system integration (avoid inconsistent routing)

**4.3.1 Enhanced Input**

* Gating must occur at the action handling layer (Triggered/Started), not scattered in random ticks.
* Avoid relying solely on “UI consumes input” as that can vary by widget setup.

**4.3.2 CommonUI (if used)**

* Prefer aligning blocking state with the activatable stack.
* UIManager registration can be driven by activatable widget lifecycle (OnActivated/OnDeactivated).

---

### 4.4 Performance requirements (explicit)

* UI blocking queries must be **constant time** and allocation-free during gameplay.
* No widget-tree scanning per click/interact.
* Interaction probing must remain within an agreed budget:

  * No more than one probe per input event (unless intentionally continuous).
  * Avoid per-frame traces unless required by design.

---

### 4.5 Staging

* If time is tight:

  * Phase 1: Implement UIManager queries + event-driven blocker registration, keep probe config hardcoded.
  * Phase 2: Add `UInteractionProfile` DataAssets + validation + defaults.
  * Phase 3: Optional debug tooling (blocking reasons overlay, dev-only logs).

## 5) Security Requirements

1. UI blocking must prevent unintended click-through interactions.
2. Validate DataAsset refs before use; fall back to safe defaults if missing.
3. Avoid exposing debug-only widgets/settings in Shipping builds.
4. Ensure blocker registration cannot be spoofed by non-UI gameplay code (keep API access scoped).

## 6) Non-Goals (Out of Scope)

* Full per-pawn profile selection.
* Complex filtering beyond collision + tags.
* UI theme/layout redesign.
* Solving every UI/input edge case globally (this is about predictable gating + maintainable ownership).

## 7) Design Considerations

* Maintain two separate concepts: “BlocksWorldClick” vs “BlocksWorldInteract”.
* Define initial widget policy **up front** (no ambiguity):

  * Examples to classify: Pause menu, inventory, dialog, tooltip, radial menu, map, settings, HUD overlays.
* Add a dev-only view to show current blockers + reasons (optional but valuable).

## 8) Technical Considerations

* UIManager should live in a stable, globally accessible place:

  * e.g., GameInstance subsystem or LocalPlayer subsystem (preferred), not a transient widget.
* Registration should tolerate duplicate calls and ensure cleanup on widget destruction.
* Consider multiplayer/local split-screen: blocking state should be per-local-player.

## 9) Success Metrics

* No “clicked through UI” issues in testing across top 10 UI flows.
* No regressions in click/interact responsiveness.
* Interaction probe behavior adjustable via DataAsset without code changes.
* Performance: no measurable spike from UI gating (queries remain O(1)).

## 10) Open Questions (now narrowed, action-oriented)

1. **Widget policy list:** Which widgets block click vs interact (prototype list to be finalized)?
2. **Fallback rule:** If profile is missing, do we disable interaction or use hardcoded defaults (recommend: disable interact, allow only if explicitly safe)?
3. **CommonUI adoption:** Are we using CommonUI activatable stacks, or custom widget management?
4. **Debug visibility:** Do we want dev-only “BlockingReasons” surfaced in an overlay/log?

## 11) Risks & Mitigations (added)

* **Risk: state mismatch (widget open but not registered).**
  Mitigation: registration is part of base widget lifecycle; add asserts in non-shipping; add automated UI open/close tests.
* **Risk: DataAsset config bloat.**
  Mitigation: enforce “config-only” rule; code review checklist; split profiles rather than adding flags.
* **Risk: inconsistent input routing between UI and gameplay.**
  Mitigation: gate at action handlers; optionally align with Enhanced Input/CommonUI routing.