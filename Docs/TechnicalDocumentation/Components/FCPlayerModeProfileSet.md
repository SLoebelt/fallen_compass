## UFCPlayerModeProfileSet — Technical Documentation

### Where to find it (paths)

- Header: `Components/Data/FCPlayerModeProfileSet.h`
- Source: `Components/Data/FCPlayerModeProfileSet.cpp`
- Related types:
  - `Core/FCPlayerModeTypes.h` — defines `EFCPlayerMode` and `FPlayerModeProfile`.
  - `Components/FCPlayerModeCoordinator.h/.cpp` — consumes this data asset.

---

## Responsibilities (what this data asset owns)

`UFCPlayerModeProfileSet` is a `UDataAsset` that groups together editor-configurable `FPlayerModeProfile` entries keyed by `EFCPlayerMode`. It is referenced by `UFCPlayerModeCoordinator` so that player presentation (camera mode, input mapping mode, cursor/mouse lock, and future click/interaction behavior) is driven by data rather than hard-coded controller branches.

### 1. Profile map

- Exposes a single property:
  
  ```cpp
  UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
  TMap<EFCPlayerMode, FPlayerModeProfile> Profiles;
  ```

- Each key is an `EFCPlayerMode` (e.g., Office, Overworld, Camp, Static).
- Each value is an `FPlayerModeProfile` struct defined in `FCPlayerModeTypes.h`, typically containing:
  - A soft reference to a `UFCInputConfig` asset to use for this mode.
  - A camera mode enum value (`EFCPlayerCameraMode`).
  - A soft reference to an interaction profile object (placeholder for a more concrete type later).
  - A high-level click policy enum (`EFCClickPolicy`) describing how clicks should be interpreted.
  - Cursor visibility (`bShowMouseCursor`).
  - Mouse lock behavior (`EMouseLockMode`).

### 2. Designer-facing usage

- Designers create one or more instances of this asset (e.g., `/Game/FC/Config/DA_PlayerModeProfiles`).
- For each entry in the `Profiles` map, they configure:
  - Office profile (Office_Exploration / Office_TableView).
  - Overworld profile (Overworld_Travel / combat states).
  - Camp profile (Camp_Local / POIScene).
  - Static profile (MainMenu, Paused, Loading, ExpeditionSummary).
- The coordinator then looks up the appropriate `FPlayerModeProfile` based on the current `EFCPlayerMode` and applies it via `ApplyMode`, which:
  - Derives and applies the correct input mapping mode through `UFCInputManager`.
  - Applies cursor visibility and input mode.
  - Requests the appropriate camera mode via `SetCameraModeLocal` on the controller.

---

## Public API

The current implementation is intentionally minimal:

- `UFCPlayerModeProfileSet` is defined as:
  
  ```cpp
  UCLASS(BlueprintType)
  class FC_API UFCPlayerModeProfileSet : public UDataAsset
  {
      GENERATED_BODY()
  
  public:
      UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
      TMap<EFCPlayerMode, FPlayerModeProfile> Profiles;
  };
  ```

// `FCPlayerModeProfileSet.cpp` currently just includes the header; lookups, validation, and application logic live in `UFCPlayerModeCoordinator`.

---

## Connected systems (who uses this and why)

### `UFCPlayerModeCoordinator` — consumer of profiles

- **What is delegated**
  - Looking up, validating, and applying `FPlayerModeProfile` for the current `EFCPlayerMode`.
- **Why**
  - Keeps data definition (profiles) separate from behavior (how to apply them). The coordinator becomes the single place that understands how to translate profiles into calls to `UFCInputManager`, `UFCCameraManager`, `UFCInteractionComponent`, and `AFCPlayerController`.

### Editor / designers — configuration surface

- **What is delegated**
  - Choosing which input config, camera mode, interaction profile, click policy, cursor visibility, and mouse lock behavior belong to each player mode.
- **Why**
  - Allows non-programmers to tweak player experience per mode without touching code, once the coordinator’s application logic is in place.

---

## Notes and future extensions

- Validation helpers and profile-lookup functions are expected to live in `UFCPlayerModeCoordinator` rather than on the data asset itself (per current plan), to keep this asset as a simple container.
- If needed later, lightweight helper methods (e.g., `bool HasProfile(EFCPlayerMode Mode) const`) can be added without changing its core responsibility: to provide designer-authored mode profiles keyed by `EFCPlayerMode`.
