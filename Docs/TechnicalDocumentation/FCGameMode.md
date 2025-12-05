## AFCGameMode — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `FCGameMode.h` 
* **Source:** `FCGameMode.cpp` 

---

## Responsibilities (what this “manager” owns)

**`AFCGameMode`** is the project’s **GameModeBase implementation** used as a lightweight orchestration point at runtime (especially at level start). It primarily owns:

1. **Template defaults for pawn + controller**

   * Sets the default pawn class to `AFCFirstPersonCharacter`.
   * Sets player controller class to `AFCPlayerController`. 

2. **Boot-time diagnostics**

   * Logs (and prints on-screen debug) the active pawn/controller and current map name on BeginPlay. 

3. **Level-start transition finalization hook**

   * On BeginPlay, it notifies `UFCLevelTransitionManager` to finalize any pending `TransitionViaLoading(...)` flows via `InitializeLevelTransitionOnLevelStart()`. 

---

## Public API

### Constructor

* `AFCGameMode()`

  * Configures `DefaultPawnClass` and `PlayerControllerClass`.
  * Logs a message confirming configuration. 

### Lifecycle

* `BeginPlay()`

  * Prints debug info to log + screen (temporary “TODO remove” comment).
  * Calls into `UFCLevelTransitionManager` (if available) to complete loading-based transitions. 

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCLevelTransitionManager` (transition completion)

**What is delegated**

* Completion of Loading → TargetState flows, especially cases like “Overworld → Office → ExpeditionSummary”. 

**Why delegated**

* GameMode is present at level start and is a reliable place to “kick” transition finalization, but the actual orchestration logic belongs in the transition subsystem. 

---

### 2) `AFCFirstPersonCharacter` (default pawn)

**What is delegated**

* Player pawn behavior for the Office level (comment indicates it’s intended for Office as part of a task milestone). 

**Why delegated**

* GameMode chooses the pawn class; pawn class owns movement/camera/interactions. 

---

### 3) `AFCPlayerController` (input + gameplay control)

**What is delegated**

* Input/camera/UI interactions are left to the PlayerController rather than GameMode. 

**Why delegated**

* GameMode sets the controller class; controller implements actual runtime control. 

---

### 4) Engine debug facilities (`GEngine`, `UE_LOG`)

**What is delegated**

* On-screen messages (`AddOnScreenDebugMessage`) and log output. 

**Why delegated**

* Keeps this class useful during early prototype iterations; debug output is explicitly marked temporary (“TODO remove once proper UI system is in place”). 