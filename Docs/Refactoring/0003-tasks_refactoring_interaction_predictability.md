# 0003 — Refactoring Interaction Predictability (Pending State + Arrival-Gated Execution)

## Relevant Files

- `/Docs/UE_NamingConventions.md` — Unreal-specific naming and folder structure guidance.
- `/Docs/UE_CodeConventions.md` — Unreal-specific coding conventions.
- `/Docs/Fallen_Compass_GDD_v0.2.md` — Game Design Document.
- `/Docs/DRM_Draft.md` — Development Roadmap.
- `/Docs/TechnicalDocumentation/FCRuntime.md` — Runtime glossary and ownership map.
- `/Docs/TechnicalDocumentation/Components/FCInteractionComponent.md` — Interaction orchestrator docs.
- `/Docs/TechnicalDocumentation/FCPlayerController.md` — Click routing, movement commands, interaction entry points.
- `/Docs/TechnicalDocumentation/Managers/FCUIManager.md` — UI responsibilities, especially POI action selection.
- `/Docs/Refactoring/0003_refactoring_interaction_predictability.md` — PRD for this milestone.

- `/FC/Source/FC/Interaction/FCInteractionComponent.h`
- `/FC/Source/FC/Interaction/FCInteractionComponent.cpp`
- `/FC/Source/FC/Core/FCPlayerController.h`
- `/FC/Source/FC/Core/FCPlayerController.cpp`
- `/FC/Source/FC/World/FCOverworldConvoy.h`
- `/FC/Source/FC/World/FCOverworldConvoy.cpp`
- `/FC/Source/FC/Characters/FC_ExplorerCharacter.h`
- `/FC/Source/FC/Characters/FC_ExplorerCharacter.cpp`
- `/FC/Source/FC/Core/FCUIManager.h`
- `/FC/Source/FC/Core/FCUIManager.cpp`


## Guiding principles (apply everywhere)

* **No tick polling for “mode/state decisions.”** Drive state via events (click, UI selection, arrival).
  *Mitigates:* Tick polling fragility + CPU waste.
* **Pointer validity always checked.** Use `TWeakObjectPtr` and guard with `.IsValid()`.
  *Mitigates:* stale pointer crashes.
* **Idempotent state transitions.** Clear latches immediately after use; ignore unexpected repeats.
  *Mitigates:* double execution / overlap spam.
* **Single movement authority per click.** Exactly one chain issues movement.
  *Mitigates:* duplicated movement / racey arrival triggers.
* **UI is view-only; gameplay decisions live in InteractionComponent.**
  *Mitigates:* mixed responsibility + inconsistent behavior.

---

## Implementation order that stays safe/compiling

1. **InteractionComponent:** add pending state fields + helper enum + reset helper
2. **InteractionComponent:** implement `NotifyArrivedAtPOI()` and “arrival-gated execution”
3. **Convoy + Explorer:** wire “arrival” events to `NotifyArrivedAtPOI()` (no execution in convoy/explorer)
4. **Controller + InteractionComponent:** consolidate click → interaction → movement issuance chain (single authority)
5. **UIManager:** ensure UI only presents actions and forwards selection; no movement/execution
6. **Docs + logs:** update docs and add consistent logging

---

# 1. Introduce explicit interaction state machine + helper in `UFCInteractionComponent`

### 1.1 Analyze current interaction state handling (done)

**What was found (for future readers):**

* “focused target”
    * `TWeakObjectPtr<AActor> CurrentInteractable;` — used by FP `Interact()` and prompt updates.
* “pending interactable”
    * `UPROPERTY() TObjectPtr<AActor> PendingInteractionPOI;` — holds the POI actor for a queued interaction.
* “selected action”
    * `EFCPOIAction PendingInteractionAction;` — stores the selected POI action to run when appropriate.
* “waiting flags”
    * `bool bHasPendingPOIInteraction = false;` — indicates there is a pending POI interaction to execute.
    * `bool bConvoyAlreadyAtPOI = false;` — indicates the convoy reached the POI before action selection (left-click path).
    * `bool bFirstPersonFocusEnabled = false;` — gates FP focus tracing + prompts (not POI-specific, but a state flag).

* In `FCInteractionComponent.md`, the current described flow was confirmed to match the pre-refactor code.
* Target timeline (for both Overworld and Camp) is:

    `Idle → Selecting (optional) → MovingToPOI → Arrived → Executing → Idle`

**Pitfall guarded here**

* **Consistent transitions:** the above members were the ad-hoc source of truth; the next steps consolidate them behind a documented state machine.

---

### 1.2 Document the interaction state machine (implemented)

**Do (done in code, keep here for juniors)**

In `FCInteractionComponent.h` (private section), a prominent comment block was added:

```cpp
// Interaction Predictability State Machine (0003)
//
// RMB Interact intent:
//   Idle -> (Selecting) -> MovingToPOI -> Arrived/Overlap -> Executing -> Idle
//
// Overlap intent (enemy ambush / incidental):
//   Idle -> (Selecting) -> Executing -> Idle
//
// Canonical fields:
//   PendingPOI + PendingAction + (bAwaitingSelection / bAwaitingArrival)
// Rules:
//   - ResetInteractionState() is called ONCE per logical transition.
//   - Movement issuance (when needed) should originate from UFCInteractionComponent (single movement authority).
//   - Overlap may occur at any time; pending action execution must be idempotent (never double fire).
```

This makes the intended phases and canonical fields explicit for any future maintainer.

---

### 1.3 Add an execution helper to guarantee “reset only once” (implemented)

**Do (code completed):**

* Added private helper declaration in `FCInteractionComponent.h`:

    ```cpp
    void ExecutePOIActionNow(AActor* POIActor, EFCPOIAction Action);
    ```

* Implemented helper in `FCInteractionComponent.cpp`:

    ```cpp
    void UFCInteractionComponent::ExecutePOIActionNow(AActor* POIActor, EFCPOIAction Action)
    {
            if (!IsValid(POIActor) || !POIActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
            {
                    UE_LOG(LogFCInteraction, Warning, TEXT("[Interaction] ExecutePOIActionNow ignored (invalid POI)"));
                    return;
            }

            // Clear first to prevent re-entrant overlap causing double fire
            ResetInteractionState();

            UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Executing action %s on POI %s"),
                    *UEnum::GetValueAsString(Action), *GetNameSafe(POIActor));

            IIFCInteractablePOI::Execute_ExecuteAction(POIActor, Action, GetOwner());

            // Preserve convoy gate cleanup where applicable
            if (AFCPlayerController* PC = GetOwnerPCCheckedOrNull())
            {
                    if (AFCOverworldConvoy* Convoy = PC->GetActiveConvoy())
                    {
                            Convoy->SetInteractingWithPOI(false);
                    }
            }
    }
    ```

**Pitfall guarded here**

* **ResetInteractionState only once per logical execution** — the helper clears latches exactly once before firing the POI action, even if overlaps come in re-entrantly.

---

### 1.4 Update overlap and selection flows to use the helper (implemented)

**NotifyPOIOverlap**

* The “pending execution” branch now executes only via `ExecutePOIActionNow`:

    ```cpp
    if (PendingPOI.IsValid() && PendingPOI.Get() == POIActor && PendingAction.IsSet() && !bAwaitingSelection)
    {
            ExecutePOIActionNow(POIActor, PendingAction.GetValue());
            return;
    }
    ```

* When a single available action is auto-executed on incidental overlap, it also goes through the helper:

    ```cpp
    if (AvailableActions.Num() == 1)
    {
            ExecutePOIActionNow(POIActor, AvailableActions[0].ActionType);
            return;
    }
    ```

* When >1 actions are available directly on overlap, the state is reset once and then latched for selection:

    ```cpp
    ResetInteractionState();
    FocusedTarget = POIActor;
    PendingPOI = POIActor;
    bAwaitingSelection = true;
    bPendingPOIAlreadyReached = true;
    // show UI...
    ```

**OnPOIActionSelected**

* For the “already reached” path, selection now executes immediately without routing back through overlap:

    ```cpp
    if (bPendingPOIAlreadyReached)
    {
            bPendingPOIAlreadyReached = false;

            // Execute immediately at current overlap location (no movement, no overlap call)
            ExecutePOIActionNow(PendingPOI.Get(), PendingAction.GetValue());
            return;
    }
    ```

* For the “not yet reached” path, the code now:

    ```cpp
    bAwaitingArrival = true;
    // movement issuance remains in InteractionComponent (Step 3 will enforce single authority end-to-end)
    ```

**Pitfalls guarded here**

* **No double clears:** both overlap-driven and selection-driven execution routes share the same helper, so `ResetInteractionState()` is invoked in exactly one place.
* **No double execution:** UI selection no longer calls `NotifyPOIOverlap` to “fake” arrival; it either executes now or defers to a real overlap with `bAwaitingArrival`.

---

### 1.5 Audit “single movement authority” (still to do, audit-only)

**Do**

* Run a search for movement issuance related to POIs:
    * `MoveConvoyToLocation(`
    * `MoveExplorerToLocation(`
    * Any AI `MoveTo` or direct Nav pathing on convoy/explorer related to POI interactions.
* Verify that for POI interaction flows, movement is issued **only** from `UFCInteractionComponent` (via the controller click router), not from UI widgets or POI actors.
* If you find additional movement issuance sites, note them inline in this file under Task 3 so they can be consolidated later.

**Pitfall guarded here**

* **Single movement authority per click** — this step documents where we stand today and feeds Task 3, but Step 1’s code changes already respect the rule for the paths they touch.

---

# 2. Implement arrival-gated POI action execution (shared convoy + explorer path)

### 2.1 Add the arrival API (implemented)

**File:** `FCInteractionComponent.h`

**Status**: Implemented in `FCInteractionComponent.h/.cpp`.

* `NotifyPOIOverlap(AActor* POIActor)` remains the **incidental overlap** handler for ambushes, random collisions, and LMB-only movement.
* A dedicated arrival API was added directly under it:

```cpp
// Called by convoy/explorer when they arrive at a POI (overlap or path-complete).
// If bAwaitingArrival is true and PendingPOI matches POIActor, executes PendingAction.
// Otherwise falls back to incidental overlap handling (enemy ambush, LMB move).
// Always clears latches on a valid pending execution (idempotent).
UFUNCTION()
void NotifyArrivedAtPOI(AActor* POIActor);
```

The implementation of `NotifyArrivedAtPOI` enforces:

* If `bAwaitingArrival` is true and `PendingPOI`/`PendingAction` match the arriving actor, it logs and executes via `ExecutePOIActionNow`, which clears state and resets the convoy interaction gate.
* If arrival does not match the pending POI, it cancels the pending intent via `ResetInteractionState()` and then treats the event as incidental by calling `NotifyPOIOverlap`.

**Pitfall guarded**

* Keeps a clear separation between **planned arrival** (`NotifyArrivedAtPOI`) and **incidental collision** (`NotifyPOIOverlap`) while remaining idempotent.

---

### 2.2 Wire convoy and explorer into `NotifyArrivedAtPOI` (implemented)

**Status**: Implemented and verified in code + docs.

* `AFC_ExplorerCharacter` reports POI arrival from its capsule overlap:

    ```cpp
    void AFC_ExplorerCharacter::OnCapsuleBeginOverlap(..., AActor* OtherActor, ...)
    {
            if (!OtherActor || OtherActor == this)
            {
                    return;
            }

            if (OtherActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
            {
                    if (UFCInteractionComponent* Interaction = FindComponentByClass<UFCInteractionComponent>())
                    {
                            Interaction->NotifyArrivedAtPOI(OtherActor);
                    }
            }
    }
    ```

* `AFCOverworldConvoy` aggregates member overlaps in `HandlePOIOverlap` and fires a delegate that is bound by `AFCPlayerController` to `UFCInteractionComponent::NotifyArrivedAtPOI`.
* `NotifyPOIOverlap` is **not** called from convoy/explorer anymore for intentional POI interactions; it is reserved for incidental overlaps.

**Pitfall guarded**

* Ensures convoy and explorer only report **arrival** into the interaction state machine; they do not execute POI actions or issue movement.

---

### 2.3 Make `HandlePOIClick` the single movement authority (implemented)

**Status**: Implemented in `FCInteractionComponent.cpp`.

**Canonical behavior (Step 3.2 realized)**

* Every RMB POI click:
    * Calls `ResetInteractionState()` once at the beginning (no stacked or ghost interactions).
    * Validates the actor and `UIFCInteractablePOI` interface.
    * Queries `GetAvailableActions` and logs the count.
* For **0 actions**: logs and returns.
* For **>1 actions**: enters `Selecting` phase only, sets `bAwaitingSelection = true`, and shows the UI via `UFCUIManager::ShowPOIActionSelection`.
    * No movement is issued yet; movement is decided later in `OnPOIActionSelected`.
* For **exactly 1 action**: sets `PendingAction` and decides whether to move using a distance check:

    ```cpp
    // Simplified core logic
    const bool bNeedsMove = NeedsMovement();
    if (bNeedsMove)
    {
            bAwaitingArrival = true;
            IssueMovementToPOI();
    }
    else
    {
            bAwaitingArrival = false;
            NotifyArrivedAtPOI(POIActor); // immediate arrival-gated execution
    }
    ```

* `IssueMovementToPOI` is a local lambda that **only** calls either `MoveExplorerToLocation` or `MoveConvoyToLocation` based on `UFCCameraManager::GetCameraMode()`.
* `NeedsMovement` is a local lambda that compares owner location vs. POI location with a conservative `ClickAcceptRadius` to avoid redundant movement when already at the POI.

**Pitfalls guarded**

* **Single movement authority per click:** Only `HandlePOIClick` (and `OnPOIActionSelected` for the multi-action path) issue movement, both via the same controller helpers.
* **Arrival-gated execution:** Even in the “already at POI” case, execution flows through `NotifyArrivedAtPOI` and `ExecutePOIActionNow`, preserving idempotent state resets.
* **UI purity:** `UFCUIManager` only presents actions and forwards selection; it does not issue movement or execute POI actions.

---

### 2.2 Implement arrival-gated execution logic (TODO)

**File:** `FCInteractionComponent.cpp`

**Do**

* Add `NotifyArrivedAtPOI` next to `NotifyPOIOverlap` using the updated concept:

```cpp
void UFCInteractionComponent::NotifyArrivedAtPOI(AActor* POIActor)
{
    if (!IsValid(POIActor))
    {
        UE_LOG(LogFCInteraction, Verbose, TEXT("[Interaction] Arrival ignored: POIActor invalid"));
        return;
    }

    // 1) Intentional, arrival-gated execution:
    //    We previously chose a POI + action and issued movement for it.
    if (bAwaitingArrival && PendingPOI.IsValid())
    {
        if (PendingPOI.Get() == POIActor && PendingAction.IsSet())
        {
            UE_LOG(LogFCInteraction, Log,
                TEXT("[Interaction] Arrival at pending POI %s, executing action %s"),
                *GetNameSafe(POIActor),
                *UEnum::GetValueAsString(PendingAction.GetValue()));

            // This clears state and (for convoy) clears the interaction gate.
            ExecutePOIActionNow(POIActor, PendingAction.GetValue());

            // bAwaitingArrival and PendingXXX are cleared inside ExecutePOIActionNow/ResetInteractionState.
            return;
        }

        // We were waiting for some other POI; treat this as an interrupt to that intent.
        UE_LOG(LogFCInteraction, Log,
            TEXT("[Interaction] Arrival mismatch: expected %s but overlapped %s. Cancelling pending interaction."),
            *GetNameSafe(PendingPOI.Get()), *GetNameSafe(POIActor));

        ResetInteractionState();
        // fall through to incidental handling for POIActor
    }

    // 2) No pending arrival for this POI: treat as incidental overlap.
    //    This covers:
    //       - pure LMB move collisions
    //       - enemy ambushes
    //       - exploratory walks that happen to hit POIs
    NotifyPOIOverlap(POIActor);
}
```

**Pitfalls guarded here**

* **Explicit arrival-gated behavior:** only executes when `bAwaitingArrival` and `PendingPOI`/`PendingAction` match.
* **Explicit interrupts:** mismatched arrivals cancel the old intent and fall back to incidental handling.
* **Idempotency:** execution still goes through `ExecutePOIActionNow`, which clears latches and resets convoy interaction gate.

---

### 2.3 Convoy wiring: use `NotifyArrivedAtPOI` (TODO)

**File:** `FCPlayerController.cpp`

**Do**

* Wherever you bind the Overworld convoy's `OnConvoyPOIOverlap` delegate, route it to `NotifyArrivedAtPOI` instead of `NotifyPOIOverlap`.

**Before**

```cpp
ActiveConvoy->OnConvoyPOIOverlap.AddDynamic(InteractionComponent, &UFCInteractionComponent::NotifyPOIOverlap);
```

**After**

```cpp
ActiveConvoy->OnConvoyPOIOverlap.AddDynamic(InteractionComponent, &UFCInteractionComponent::NotifyArrivedAtPOI);
```

**Pitfall guarded**

* Convoy overlaps are treated as **arrivals** into the state machine; the interaction component decides whether they complete a pending action or count as incidental encounters.

---

### 2.4 Explorer wiring: add POI arrival and route to `NotifyArrivedAtPOI` (TODO)

The Camp explorer already handles movement and path-follow; this step makes it behave like a "mini convoy member" from the interaction system's perspective.

**File:** `FC_ExplorerCharacter.h`

**Do**

* Add a forward declaration and cached pointer:

```cpp
class UFCInteractionComponent;

UCLASS()
class FC_API AFC_ExplorerCharacter : public ACharacter
{
    GENERATED_BODY()

    // ...

protected:
    // existing movement fields ...

    /** Cached reference to the player's interaction component (for POI arrival notifications). */
    UPROPERTY()
    TWeakObjectPtr<UFCInteractionComponent> InteractionComponent;

    /** Handle capsule overlap with POIs (Camp arrival). */
    UFUNCTION()
    void OnCapsuleBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
};
```

**File:** `FC_ExplorerCharacter.cpp`

**Do**

* In `BeginPlay`, bind the overlap and cache the interaction component:

```cpp
void AFC_ExplorerCharacter::BeginPlay()
{
    Super::BeginPlay();

    AController* CurrentController = GetController();

    UE_LOG(LogFCExplorerCharacter, Log, TEXT("ExplorerCharacter BeginPlay:"));
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("  ExplorerType: %s"), *UEnum::GetValueAsString(ExplorerType));
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("  Controller: %s"),
        CurrentController ? *CurrentController->GetName() : TEXT("NONE"));

    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AFC_ExplorerCharacter::OnCapsuleBeginOverlap);
        UE_LOG(LogFCExplorerCharacter, Log, TEXT("ExplorerCharacter: Bound capsule overlap event"));
    }

    if (AFCPlayerController* PC = Cast<AFCPlayerController>(CurrentController))
    {
        UFCInteractionComponent* IC = PC->FindComponentByClass<UFCInteractionComponent>();
        InteractionComponent = IC;
        if (!InteractionComponent.IsValid())
        {
            UE_LOG(LogFCExplorerCharacter, Warning,
                TEXT("ExplorerCharacter: No UFCInteractionComponent found on controller %s"),
                *GetNameSafe(PC));
        }
    }
}
```

* Implement the overlap handler:

```cpp
void AFC_ExplorerCharacter::OnCapsuleBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    if (!OtherActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
    {
        return;
    }

    if (!InteractionComponent.IsValid())
    {
        UE_LOG(LogFCExplorerCharacter, Warning,
            TEXT("OnCapsuleBeginOverlap: InteractionComponent invalid, cannot notify arrival"));
        return;
    }

    UE_LOG(LogFCExplorerCharacter, Log,
        TEXT("ExplorerCharacter: Detected overlap with POI %s, notifying InteractionComponent"),
        *OtherActor->GetName());

    InteractionComponent->NotifyArrivedAtPOI(OtherActor);
}
```

**Pitfalls guarded here**

* Camp explorer now reports arrivals into the same interaction pipeline as the convoy; the orchestrator decides whether that arrival is planned or incidental.

---

### 2.5 FirstPerson carveout (no arrival gating) (already correct)

**Note**

* The Office/FirstPerson flow continues to use `CurrentInteractable` + `Interact()` and does **not** touch POI pending latches or `bAwaitingArrival`, so no changes are required here.

**Pitfall guarded**

* Prevents accidental coupling of FP interactions to movement/arrival.

---

# 3. Enforce a single movement authority per click (Overworld + Camp)

### 3.1 Map current movement call sites

**Do**
Search for:

* `MoveConvoyToLocation`
* `MoveExplorerToLocation`
* pathing calls / AI move requests
* click handlers that issue movement

Create a small dev-note table in a comment (or doc):

| Path                | Who receives click | Who decides action | Who issues movement                  | Who executes action          |
| ------------------- | ------------------ | ------------------ | ------------------------------------ | ---------------------------- |
| Overworld click POI | Controller         | InteractionComp    | InteractionComp (via controller API) | InteractionComp (on arrival) |
| Camp click POI      | Controller         | InteractionComp    | InteractionComp                      | InteractionComp (on arrival) |

**Pitfall guarded**

* prevents “movement issued from two different places.”

---

### 3.2 Consolidate movement issuance into one chain

**Recommended canonical chain**
`PlayerController::HandleClick → InteractionComponent::HandlePOIClick → (issue movement) → NotifyArrivedAtPOI → execute`

**Do**

* In `HandlePOIClick`, decide if movement is needed (distance / already at POI)
* Issue exactly one movement call (through a controller helper, not UIManager)
* Set `bAwaitingArrival=true` only when movement is actually issued

Example:

```cpp
// after PendingPOI and PendingAction set
const bool bNeedsMove = /* distance check or convoy state */;
if (bNeedsMove)
{
    bAwaitingArrival = true;
    // Ask controller to move convoy/explorer (single authority)
    OwningController->MoveConvoyToPOI(PendingPOI.Get()); // example helper
}
else
{
    // Already at POI: execute immediately (no arrival latch)
    NotifyArrivedAtPOI(PendingPOI.Get());
}
```

**Pitfalls guarded**

* **Single movement authority:** movement only comes from InteractionComponent.
* **Idempotency:** repeated clicks reset state first; repeated movement requests don’t stack.
* **Avoid UI-driven movement:** UIManager must not call movement.

---

### 3.3 Verify FP interactions never issue movement

**Status (audit complete, no code changes):**

* Searched FP interaction entry points (`AFCFirstPersonCharacter::HandleInteract`, `AFCPlayerController::HandleInteractPressed` in `FirstPerson` mode, and `HandleTableObjectClick`).
* All of these delegate to `UFCInteractionComponent::Interact` or table interaction logic only; none call `MoveConvoyToLocation`, `MoveExplorerToLocation`, AI `MoveTo`, or other movement helpers.
* Result: FP interaction paths do **not** issue movement today, so no removals were necessary.

---

# 4. Standardize multi-action vs single-action selection behavior

### 4.1 Choose single-action policy (document it)

Pick one and write it as a comment near `HandlePOIClick`:

* **Option A (recommended):** auto-run single action
* Option B: confirm UI

**Pitfall guarded**

* Consistency across Overworld and Camp.

---

### 4.2 Multi-action selection flow

**Do**
In `HandlePOIClick` after querying POI actions:

* If `Actions.Num() > 1`:

  * `bAwaitingSelection=true`
  * `bAwaitingArrival=false`
  * store `PendingPOI`
  * call `UIManager->ShowPOIActionSelection(...)`
  * block world clicks while widget open (existing “UI blocks world” rule)

```cpp
if (Actions.Num() > 1)
{
    bAwaitingSelection = true;
    bAwaitingArrival   = false;

    UIManager->ShowPOIActionSelection(PendingPOI.Get(), Actions);
    UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Phase=Selecting POI=%s"), *GetNameSafe(PendingPOI.Get()));
    return;
}
```

**Pitfalls guarded**

* **Consistent transitions:** selecting only happens when >1 action.
* **UI separation:** UIManager shows UI only; InteractionComp owns state.

---

### 4.3 Single-action behavior

If `Actions.Num() == 1`:

* set `PendingAction`
* either execute immediately (already at POI) or set awaiting arrival and issue movement

```cpp
if (Actions.Num() == 1)
{
    PendingAction = Actions[0];

    const bool bNeedsMove = /* distance check */;
    if (bNeedsMove)
    {
        bAwaitingArrival = true;
        UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Phase=MovingToPOI POI=%s Action=%d"),
            *GetNameSafe(PendingPOI.Get()), (int32)PendingAction.GetValue());
        OwningController->MoveConvoyToPOI(PendingPOI.Get());
    }
    else
    {
        UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Phase=Executing POI=%s Action=%d"),
            *GetNameSafe(PendingPOI.Get()), (int32)PendingAction.GetValue());
        NotifyArrivedAtPOI(PendingPOI.Get());
    }
}
```

**Pitfalls guarded**

* **Idempotency:** execution always clears state via `ResetInteractionState()`.
* **No implicit “pick first for multi-actions.”**

---

# 5. Clarify UIManager vs InteractionComponent responsibilities

### 5.1 Audit UIManager POI APIs

**Do**
Identify any UIManager code that:

* starts movement
* executes POI actions
* mutates interaction state fields

**Pitfalls guarded**

* **Mixed responsibility:** UI must not be a gameplay brain.

---

### 5.2 Restrict UIManager to view-only

**Do**
UIManager should only:

* show selection widget
* forward selected action back to InteractionComponent

Preferred callback flow:
`UIWidget → UIManager → InteractionComponent::OnPOIActionSelected(...)`

No: `UIManager → Controller → execute`.

---

### 5.3 Confirm controller routing stays clean

**Do**
Controller:

* forwards click → InteractionComponent
* does not decide actions
* does not issue movement (except as a “transport API” called by InteractionComp)

Add a short comment above click handlers:

```cpp
// Note: interaction configuration and decisions are owned by UFCInteractionComponent.
```

---

# 6. Update documentation + logging

### 6.1 Logging

Add logs at phase transitions, using consistent format:

```cpp
UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Phase=%d POI=%s Action=%s"),
    (int32)GetCurrentInteractionPhase(),
    *GetNameSafe(PendingPOI.Get()),
    PendingAction.IsSet() ? TEXT("Set") : TEXT("None"));
```

**Pitfalls guarded**

* helps debug inconsistent transitions quickly.

---

### 6.2 Docs updates

Update:

* `FCRuntime.md`: describe “pending state + arrival-gated execution”
* `FCInteractionComponent.md`: add phases + field definitions + `NotifyArrivedAtPOI` contract
* `FCUIManager.md`: explicitly view-only for POI selection

---

### 6.3 Quick validation scenarios (PIE)

Run:

* Overworld single-action POI: click → move → arrive → execute once
* Overworld multi-action POI: click → selecting → choose → move → arrive → execute once
* Camp same behaviors
* Random overlap without pending: should log “ignored” and do nothing

---

## Pitfalls and mitigations (embedded summary)

* **Pointer Validity:** `TWeakObjectPtr` + `.IsValid()` checks everywhere
* **Idempotency:** `ResetInteractionState()` called after execute; new click resets stale state
* **Delegate Binding:** `RemoveAll(this)` before `Add...` when wiring arrival delegates
* **Avoid Tick Polling:** no camera queries or arrival checks in tick; event-driven only
* **Consistent transitions:** only advance phases when in correct latch state
