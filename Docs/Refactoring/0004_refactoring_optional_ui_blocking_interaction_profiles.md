# PRD — Optional Polish (UI Blocking Queries + Interaction Profiles DataAssets)

## 1) Introduction/Overview
Optional milestone to reduce edge-case bugs and improve maintainability by:
- Centralizing UI blocking logic into explicit queries
- Introducing interaction probe config as a DataAsset (`UInteractionProfile`)

## 2) Goals
- Remove scattered “is UI open?” checks across controller and interaction.
- Make click/interact gating explicit per modality.
- Allow probe tuning without recompiling C++.

## 3) User Stories
- As a player, clicking does nothing when a blocking modal is open, consistently.
- As a developer, I can adjust interaction range/trace settings in a DataAsset.

## 4) Functional Requirements
1. **UI blocking queries**
   1.1 UIManager must provide:
   - `CanWorldClick()` (TopDown/Camp)
   - `CanWorldInteract()` (FirstPerson interact)
   1.2 Controller and InteractionComponent must consult these queries.  
   1.3 UIManager must not call back into controller/interaction (avoid cycles).
2. **Interaction profile DataAsset**
   2.1 Create `UInteractionProfile` with config-only fields:
   - probe type (LineTrace / SphereTrace / CursorHit / OverlapOnly)
   - trace channel/object types
   - range/radius
   - optional prompt widget class
   - allowed tags (optional)
   2.2 Integrate `UInteractionProfile` into `FPlayerModeProfile`.
   2.3 Profiles must not accumulate special-case flags; if needed, move behavior to code or split controllers later.
3. **Staging**
   3.1 If time is tight, keep probe config hardcoded and migrate later.

## 5) Security Requirements
1. UI blocking must prevent unintended click-through interactions.
2. Validate DataAsset refs before use; fall back to safe defaults if missing.
3. Avoid exposing debug-only widgets/settings in Shipping builds.

## 6) Non-Goals (Out of Scope)
- Full per-pawn profile selection.
- Complex filtering beyond collision + tags.
- UI theme/layout redesign.

## 7) Design Considerations (Optional)
- Define which widgets block click vs interact (may differ).

## 8) Technical Considerations (Optional)
- Some HUD may block click but not interact (or vice versa); keep two queries.

## 9) Success Metrics
- No “clicked through UI” issues in testing.
- Probe behavior adjustable via DataAsset.

## 10) Open Questions
- Which widgets count as blocking click vs blocking interact for prototype?
