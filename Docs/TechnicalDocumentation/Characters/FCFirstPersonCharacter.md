## AFCFirstPersonCharacter — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `FCFirstPersonCharacter.h` 
* **Source:** `FCFirstPersonCharacter.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFCFirstPersonCharacter`** is the project’s **office exploration** first-person pawn. It owns:

1. **First-person camera setup**

   * Creates `FirstPersonCamera` at eye height and attaches it to the capsule.
   * Uses pawn control rotation for camera rotation.

2. **Character movement tuning (FPS-style)**

   * Configures movement and rotation behavior for controller-yaw-driven first-person movement (walk speed, braking, air control, etc.). 

3. **Look control with pitch clamping**

   * Applies configurable `LookSensitivity` for mouse look.
   * Clamps controller pitch every tick into `[MinPitchAngle, MaxPitchAngle]`.

4. **Interaction trigger**

   * Owns an `UFCInteractionComponent` and routes “Interact” input into `InteractionComponent->Interact()`.

5. **Mesh + animation runtime hookup**

   * Loads `SKM_Manny_Simple` in the constructor, positions it relative to the capsule.
   * Optionally loads and applies a `DefaultAnimBlueprint` soft-class at BeginPlay.

---

## Public API (Blueprint-facing / usable surface)

### Accessors

* `GetFirstPersonCamera() -> UCameraComponent*` 
* `GetInteractionComponent() -> UFCInteractionComponent*` 
* `GetLookSensitivity() -> float` 

### Gameplay lifecycle

* `BeginPlay()`

  * Logs spawn settings.
  * If `DefaultAnimBlueprint` is set, loads synchronously and applies to the mesh’s anim instance class.
* `Tick(float DeltaTime)`

  * Calls `ClampCameraPitch()` continuously.
* `SetupPlayerInputComponent(UInputComponent*)`

  * Binds Enhanced Input actions:

    * `IA_Move` → `HandleMove`
    * `IA_Look` → `HandleLook`
    * `IA_Interact` → `HandleInteract`

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) Enhanced Input (`UEnhancedInputComponent`, `UInputAction`)

**Delegated:** raw input event processing; the character only binds actions and implements handlers.
**Why:** keeps input definitions as assets and enables mapping-context-driven control.

### 2) Character movement (`UCharacterMovementComponent`)

**Delegated:** actual movement physics, acceleration/braking, air control, etc.
**Why:** the character provides intent via `AddMovementInput`; engine movement component does the simulation. 

### 3) Interaction (`UFCInteractionComponent`)

**Delegated:** “what does Interact actually do?” and how interactables are detected/handled.
**Why:** isolates interaction logic from pawn plumbing; the pawn just triggers it.

### 4) Camera system (`UCameraComponent` + controller rotation)

**Delegated:** applying view rotation and its effects.
**Why:** controller rotation is the canonical driver for FPS aim; character only clamps pitch as a safety rule.

### 5) Animation (`UAnimInstance` + soft-class loading)

**Delegated:** actual animation graph execution.
**Why:** lets designers swap the anim BP per character instance via `DefaultAnimBlueprint` without hard references during CDO construction.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Asset paths used directly in code

* **Skeletal mesh:** `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple` 
* **Input Actions:**

  * `/Game/FC/Input/IA_Move`
  * `/Game/FC/Input/IA_Look`
  * `/Game/FC/Input/IA_Interact` 

### Tunables (Editor)

* `LookSensitivity` (default `1.0`, clamp 0.1–5.0) 
* `MinPitchAngle` / `MaxPitchAngle` (defaults `-60` / `60`) 
* `DefaultAnimBlueprint` (soft class; comment suggests assigning something like `ABP_Unarmed`)

### Implementation notes (useful for maintainers)

* Look Y is inverted in `HandleLook` (`AddControllerPitchInput(-LookY * Sensitivity)`), then clamped in Tick. 
* There’s a comment saying “Hide mesh in first-person view”, but the code sets `SetOwnerNoSee(false)` (meaning the owner *can* see the mesh). If you intended invisible body in FP, this is the toggle to revisit. 
