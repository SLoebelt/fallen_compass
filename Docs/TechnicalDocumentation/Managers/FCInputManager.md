## UFCInputManager — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `Components/FCInputManager.h` 
* **Source:** `Components/FCInputManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCInputManager`** is a **PlayerController-attached ActorComponent** that centralizes **Enhanced Input Mapping Context switching** for different gameplay modes. It was extracted from `AFCPlayerController` to keep input-mode policy reusable and consistent.

It owns:

1. **Input mapping mode state**

   * Tracks the currently active mapping mode (`CurrentMappingMode`).

2. **Config reference + mapping lookup (via `UFCInputConfig`)**

   * Holds a single reference to a `UFCInputConfig` data asset, which contains all `UInputMappingContext*`s and `UInputAction*`s for this player.
   * When `SetInputMappingMode` is called, the manager looks up the appropriate `UInputMappingContext` for the requested mode from this config and applies it.

3. **Switching logic**

   * Clears all existing mappings and applies exactly one context for the requested mode, at `DefaultMappingPriority`, using the mapping context pulled from `InputConfig`.

4. **Owner validation + diagnostics**

   * Ensures it is attached to a `APlayerController` and warns when contexts aren’t configured. 

---

## Public API (Blueprint-facing)

### Mapping control

* `SetInputMappingMode(EFCInputMappingMode NewMode)`

   * Reads the configured `UFCInputConfig`, looks up the context for the mode, clears current mappings, and applies that context.
* `GetCurrentMappingMode() -> EFCInputMappingMode` 

### Modes (`EFCInputMappingMode`)

* `FirstPerson` (office exploration)
* `TopDown` (overworld navigation)
* `Fight` (combat)
* `StaticScene` (cutscenes/dialogue)
* `POIScene` (camp/local click-move + UI) 

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) Enhanced Input subsystem (`UEnhancedInputLocalPlayerSubsystem`)

**Delegated:** actual storage and evaluation of mapping contexts.
**Why:** the subsystem is the engine-native place for per-local-player context stacks; this component just chooses which context should be active. 

### 2) Owning `APlayerController` / `ULocalPlayer`

**Delegated:** access to the correct local player and input subsystem.
**Why:** mapping contexts are per-local-player, and the component is explicitly designed to live on a PlayerController.

---

## Where to configure / extend (practical notes)

* Configure a single `UFCInputConfig` asset with both `UInputAction*` and `UInputMappingContext*` references and assign it on `UFCInputManager` (component defaults on `BP_FC_PlayerController`). `SetInputMappingMode` will then have valid contexts to apply.
* There are no longer per-mode `*MappingContext` UPROPERTYs on the component; the config asset is the single source of truth.
* If you introduce a new gameplay mode, extend `EFCInputMappingMode`, add a new `UInputMappingContext*` property, and add another switch case in `SetInputMappingMode`.