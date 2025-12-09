## UFCUIBlockSubsystem — World input blocking (click vs interact)

### Where to find it

* **Header:** `Core/FCUIBlockSubsystem.h`
* **Source:** `Core/FCUIBlockSubsystem.cpp`

---

## Responsibility

`UFCUIBlockSubsystem` is a `UGameInstanceSubsystem` that centralizes whether world input should be processed right now.

- Widgets register/unregister themselves as blockers:
  - `RegisterBlocker(UUserWidget* Widget, bool bBlocksClick, bool bBlocksInteract)`
  - `UnregisterBlocker(UUserWidget* Widget)`
- Internally tracks active blockers in a map from `UUserWidget` to simple flags and maintains cached booleans:
  - `bBlocksWorldClickCached`
  - `bBlocksWorldInteractCached`
- Exposes fast queries:
  - `bool CanWorldClick() const;`
  - `bool CanWorldInteract() const;`
  - `FFCWorldInputBlockState GetWorldInputBlockState() const;` (includes debug reasons in non-shipping builds).

This makes `CanWorldClick()` / `CanWorldInteract()` effectively O(1) regardless of how many widgets are open.

---

## Collaborators

### `UFCUIManager` (manager)

- Decides which widgets should be considered blocking and, for legacy widgets, drives registration/unregistration in the subsystem.
- Provides blocking-aware base classes (e.g. pause menu, table widgets, POI action selection, overworld map, expedition summary) that auto-register/unregister themselves with `UFCUIBlockSubsystem`.

### `AFCPlayerController` (controller)

- Calls `CanWorldClick()` in click handlers (`HandleClick`, `HandleOverworldClickMove`, `HandleTableObjectClick`).
- Calls `CanWorldInteract()` in interaction handlers (`HandleInteractPressed`) before running FP traces or delegating to `UFCInteractionComponent`.

### `UFCInteractionComponent` (component)

- Calls `CanWorldInteract()` inside `DetectInteractables()` before running FirstPerson focus traces.
- This ensures interaction prompts and logs do not appear behind blocking UI.

---

## Debugging

- In non-shipping builds, the subsystem can keep a small list of human-readable reasons for blocking (widget names and flags) as part of `FFCWorldInputBlockState`.
- These reasons are useful when building a debug HUD for input blocking but are stripped in shipping builds using `#if !UE_BUILD_SHIPPING` around the extra data.
