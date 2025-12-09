Awesome — with the files you uploaded, I can give you a **beginner-friendly step-by-step implementation tasklist** that plugs into your current flow (you already gate via `CanProcessWorldInteraction()` in several places).

Below is the tasklist, including **exact files to create/modify** and **copy-pasteable snippets**.

---

## 0) What we’re building (in your project terms)

* Replace scattered checks like `CanProcessWorldInteraction()` with **two explicit queries**:

  * `CanWorldClick()` for click-to-move / table clicking
  * `CanWorldInteract()` for “E interact” / focus traces
* Make those queries **O(1)** using cached state updated by widget registration.
* Add `UFCInteractionProfile` DataAsset and wire it into your existing `FPlayerModeProfile::InteractionProfile` placeholder. 

---

## 1) Create a new UI blocking subsystem (O(1) queries)

### Create files

* `Source/FC/Core/FCUIBlockSubsystem.h`
* `Source/FC/Core/FCUIBlockSubsystem.cpp`

### FCUIBlockSubsystem.h

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FCUIBlockSubsystem.generated.h"

class UUserWidget;

USTRUCT(BlueprintType)
struct FFCWorldInputBlockState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool bBlocksWorldClick = false;
    UPROPERTY(BlueprintReadOnly) bool bBlocksWorldInteract = false;

#if !UE_BUILD_SHIPPING
    UPROPERTY(BlueprintReadOnly) TArray<FString> BlockingReasons;
#endif
};

UCLASS()
class FC_API UFCUIBlockSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void RegisterBlocker(UUserWidget* Widget, bool bBlocksClick, bool bBlocksInteract);
    void UnregisterBlocker(UUserWidget* Widget);

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    bool CanWorldClick() const { return !bBlocksWorldClickCached; }

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    bool CanWorldInteract() const { return !bBlocksWorldInteractCached; }

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    FFCWorldInputBlockState GetWorldInputBlockState() const;

private:
    struct FBlockerFlags { bool bClick=false; bool bInteract=false; };

    TMap<TWeakObjectPtr<UUserWidget>, FBlockerFlags> ActiveBlockers;

    bool bBlocksWorldClickCached = false;
    bool bBlocksWorldInteractCached = false;

#if !UE_BUILD_SHIPPING
    TArray<FString> BlockingReasonsCached;
#endif

    void UpdateCache();
    void PruneInvalid();
};
```

### FCUIBlockSubsystem.cpp

```cpp
#include "FCUIBlockSubsystem.h"
#include "Blueprint/UserWidget.h"

void UFCUIBlockSubsystem::RegisterBlocker(UUserWidget* Widget, bool bBlocksClick, bool bBlocksInteract)
{
    if (!Widget) return;

    ActiveBlockers.Add(Widget, FBlockerFlags{bBlocksClick, bBlocksInteract});
    UpdateCache();
}

void UFCUIBlockSubsystem::UnregisterBlocker(UUserWidget* Widget)
{
    if (!Widget) return;

    ActiveBlockers.Remove(Widget);
    UpdateCache();
}

void UFCUIBlockSubsystem::PruneInvalid()
{
    for (auto It = ActiveBlockers.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void UFCUIBlockSubsystem::UpdateCache()
{
    PruneInvalid();

    bBlocksWorldClickCached = false;
    bBlocksWorldInteractCached = false;

#if !UE_BUILD_SHIPPING
    BlockingReasonsCached.Reset();
#endif

    for (const auto& Pair : ActiveBlockers)
    {
        const TWeakObjectPtr<UUserWidget> Widget = Pair.Key;
        const FBlockerFlags Flags = Pair.Value;

        if (!Widget.IsValid()) continue;

        if (Flags.bClick)
        {
            bBlocksWorldClickCached = true;
#if !UE_BUILD_SHIPPING
            BlockingReasonsCached.Add(FString::Printf(TEXT("%s blocks click"), *Widget->GetName()));
#endif
        }

        if (Flags.bInteract)
        {
            bBlocksWorldInteractCached = true;
#if !UE_BUILD_SHIPPING
            BlockingReasonsCached.Add(FString::Printf(TEXT("%s blocks interact"), *Widget->GetName()));
#endif
        }
    }
}

FFCWorldInputBlockState UFCUIBlockSubsystem::GetWorldInputBlockState() const
{
    FFCWorldInputBlockState State;
    State.bBlocksWorldClick = bBlocksWorldClickCached;
    State.bBlocksWorldInteract = bBlocksWorldInteractCached;
#if !UE_BUILD_SHIPPING
    State.BlockingReasons = BlockingReasonsCached;
#endif
    return State;
}
```

---

## 2) Create a widget base that auto-registers itself as a blocker

### Create files

* `Source/FC/UI/FCBlockingWidgetBase.h`
* `Source/FC/UI/FCBlockingWidgetBase.cpp`

### FCBlockingWidgetBase.h

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FCBlockingWidgetBase.generated.h"

/**
 * Base widget that automatically registers itself as a blocking UI widget
 * while it is on screen.
 *
 * New: You can choose whether it blocks World Click and/or World Interact.
 */
UCLASS()
class FC_API UFCBlockingWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** When true, blocks click-to-move / world clicking while this widget is visible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI Blocking")
	bool bBlocksWorldClick = true;

	/** When true, blocks world interaction (e.g. E interact) while this widget is visible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI Blocking")
	bool bBlocksWorldInteract = true;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RegisterAsBlocker();
	void UnregisterAsBlocker();
};

```

### FCBlockingWidgetBase.cpp

```cpp
#include "UI/FCBlockingWidgetBase.h"

#include "Core/UFCGameInstance.h"
#include "Core/FCUIManager.h"               // Keep for backward-compat (optional)
#include "Core/FCUIBlockSubsystem.h"        // Step 1 subsystem

void UFCBlockingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RegisterAsBlocker();

	// Backward-compat: keep your existing focused widget path until all gating is migrated.
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				UIManager->SetFocusedBlockingWidget(this);
			}
		}
	}
}

void UFCBlockingWidgetBase::NativeDestruct()
{
	UnregisterAsBlocker();

	// Backward-compat: clear focused widget only if we're still the focused one.
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				if (UIManager->FocusedBlockingWidget == this)
				{
					UIManager->SetFocusedBlockingWidget(nullptr);
				}
			}
		}
	}

	Super::NativeDestruct();
}

void UFCBlockingWidgetBase::RegisterAsBlocker()
{
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
			{
				Blocker->RegisterBlocker(this, bBlocksWorldClick, bBlocksWorldInteract);
			}
		}
	}
}

void UFCBlockingWidgetBase::UnregisterAsBlocker()
{
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
			{
				Blocker->UnregisterBlocker(this);
			}
		}
	}
}

```

### Beginner step (Blueprint)

* For each blocking widget BP (pause menu, overworld map, table widgets):

  * Set its parent class to `UFCBlockableWidgetBase`
  * Tick:

    * Pause menu: blocks click ✅ blocks interact ✅
    * Overworld map: blocks click ✅ blocks interact ❓ (your choice)
    * A “tooltip” HUD: maybe blocks neither

---

## 3) Replace your current gating with the new explicit queries

Right now you gate using `AFCPlayerController::CanProcessWorldInteraction()` and call it from click handling and interaction tracing.

### 3.1 Modify `FCPlayerController.h`

Add:

```cpp
bool CanWorldClick() const;
bool CanWorldInteract() const;
```

Put them near `CanProcessWorldInteraction()` in `AFCPlayerController`. 

### 3.2 Modify `FCPlayerController.cpp`

Add these implementations:

```cpp
#include "FC/Core/FCUIBlockSubsystem.h"

bool AFCPlayerController::CanWorldClick() const
{
    const UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
    if (!GI) return true;

    if (const UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
    {
        return Blocker->CanWorldClick();
    }

    // Safety fallback: keep old behavior until all widgets migrate
    return CanProcessWorldInteraction();
}

bool AFCPlayerController::CanWorldInteract() const
{
    const UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
    if (!GI) return true;

    if (const UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
    {
        return Blocker->CanWorldInteract();
    }

    return CanProcessWorldInteraction();
}
```

### 3.3 Use `CanWorldClick()` in click paths

Replace checks like:

* `if (!CanProcessWorldInteraction()) return;` in:

  * `HandleOverworldClickMove()` 
  * `HandleTableObjectClick()` 
  * POIScene click branch inside `HandleClick()` 

with:

```cpp
if (!CanWorldClick())
{
    return;
}
```

### 3.4 Use `CanWorldInteract()` in interact paths

At the top of `HandleInteractPressed()` add:

```cpp
if (!CanWorldInteract())
{
    return;
}
```

This ensures “E interact” never leaks through modals.

---

## 4) Update InteractionComponent to use `CanWorldInteract()` (not the old combined check)

You currently skip traces if `!FCPC->CanProcessWorldInteraction()`. 

In `UFCInteractionComponent::DetectInteractables()` change:

```cpp
if (!FCPC->CanProcessWorldInteraction())
```

to:

```cpp
if (!FCPC->CanWorldInteract())
{
    CurrentInteractable = nullptr;
    return;
}
```

That keeps prompts from appearing behind UI, but only for “interact gating” (which is what this trace is really about).

---

## 5) Add `UFCInteractionProfile` DataAsset and wire it into `FPlayerModeProfile`

Right now `FPlayerModeProfile` has a placeholder `TSoftObjectPtr<UObject> InteractionProfile`. 

### 5.1 Create files

* `Source/FC/Interaction/FCInteractionProfile.h`
* `Source/FC/Interaction/FCInteractionProfile.cpp` (can be empty)

### FCInteractionProfile.h

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "FCInteractionProfile.generated.h"

UENUM(BlueprintType)
enum class EFCProbeType : uint8
{
    LineTrace,
    SphereTrace,
    CursorHit,
    OverlapOnly
};

UCLASS(BlueprintType)
class FC_API UFCInteractionProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    EFCProbeType ProbeType = EFCProbeType::LineTrace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    float Range = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    float Radius = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UUserWidget> PromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Filter")
    TArray<FName> AllowedTags;

    bool IsValidProfile() const { return Range > 0.f && Radius >= 0.f; }
};
```

### 5.2 Modify `FCPlayerModeTypes.h`

Change the placeholder:

```cpp
TSoftObjectPtr<UObject> InteractionProfile;
```

to:

```cpp
class UFCInteractionProfile;

UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
TSoftObjectPtr<UFCInteractionProfile> InteractionProfile;
```

This is the cleanest “drop-in” upgrade to your existing DataAsset-driven mode setup. 

---

## 6) Apply the profile to `UFCInteractionComponent`

### 6.1 Modify `FCInteractionComponent.h`

Add:

```cpp
#include "FCInteractionProfile.h" // or forward declare + include in .cpp

UPROPERTY(EditDefaultsOnly, Category="Interaction|Profile")
TSoftObjectPtr<UFCInteractionProfile> DefaultProfile;

void ApplyInteractionProfile(UFCInteractionProfile* NewProfile);

private:
UPROPERTY()
TObjectPtr<UFCInteractionProfile> ActiveProfile;

UFCInteractionProfile* GetEffectiveProfile() const;
```

(Place these near your existing interaction tuning fields like `InteractionTraceDistance`. )

### 6.2 Modify `FCInteractionComponent.cpp`

Implement:

```cpp
#include "FCInteractionProfile.h"

UFCInteractionProfile* UFCInteractionComponent::GetEffectiveProfile() const
{
    if (ActiveProfile && ActiveProfile->IsValidProfile()) return ActiveProfile;
    if (DefaultProfile.IsValid()) return DefaultProfile.Get();

    return nullptr;
}

void UFCInteractionComponent::ApplyInteractionProfile(UFCInteractionProfile* NewProfile)
{
    ActiveProfile = (NewProfile && NewProfile->IsValidProfile()) ? NewProfile : nullptr;

    // Optional: rebuild prompt widget if class differs
    ClearFocusAndHidePrompt();
}
```

Then in `DetectInteractables()` use the profile:

```cpp
UFCInteractionProfile* Profile = GetEffectiveProfile();
const float Range = Profile ? Profile->Range : InteractionTraceDistance;
const ECollisionChannel Channel = Profile ? Profile->TraceChannel : ECC_Visibility;

FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * Range);
```

And add tag filtering:

```cpp
if (Profile && Profile->AllowedTags.Num() > 0)
{
    bool bTagOk = false;
    for (const FName Tag : Profile->AllowedTags)
    {
        if (HitActor->ActorHasTag(Tag)) { bTagOk = true; break; }
    }
    if (!bTagOk) { CurrentInteractable = nullptr; return; }
}
```

You already have a good “disable focus and hide prompt” hook via `SetFirstPersonFocusEnabled()` and `ClearFocusAndHidePrompt()` — keep using that.

---

## 7) Hook profiles into your mode system (real code, your files)

### 7.1 Update the profile type (stop using the placeholder `UObject`)

In `FCPlayerModeTypes.h`, you still have:

```cpp
TSoftObjectPtr<UObject> InteractionProfile;
```

Replace it with a typed soft pointer. 

**Copy-paste patch:**

```cpp
// FCPlayerModeTypes.h

class UFCInputConfig;
class UFCInteractionProfile; // <-- add this

// ...

/** Mode-specific interaction probe settings (DataAsset). */
UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
TSoftObjectPtr<UFCInteractionProfile> InteractionProfile; // <-- replace the UObject version
```

### 7.2 Apply the InteractionProfile in `UFCPlayerModeCoordinator::ApplyMode`

Your coordinator already applies camera + input + cursor and then only toggles FP focus. 
Now we load the interaction profile and pass it to the component.

**In `FCPlayerModeCoordinator.cpp` add the include:**

```cpp
#include "Interaction/FCInteractionProfile.h"
```

**Then replace your interaction block with this (copy-paste):**

```cpp
// 4) Interaction gating + profile apply (no tick polling)
if (UFCInteractionComponent* Interaction = PC->FindComponentByClass<UFCInteractionComponent>())
{
    Interaction->SetFirstPersonFocusEnabled(Profile.CameraMode == EFCPlayerCameraMode::FirstPerson);

    UFCInteractionProfile* LoadedProfile = nullptr;
    if (!Profile.InteractionProfile.IsNull())
    {
        LoadedProfile = Profile.InteractionProfile.LoadSynchronous();
    }

    Interaction->ApplyInteractionProfile(LoadedProfile);
}
```

That’s it for “profiles actually drive behavior when modes change”. (Before, you had the “Later:” comment here.) 

---

## 7.5 UIManager coverage patch (PauseMenu / Table / POI / OverworldMap / ExpeditionSummary)

Why this is good: right now `CanWorldClick/CanWorldInteract` can only be as correct as the UI state you track. You already block via `FocusedBlockingWidget` (and table fallback), but **PauseMenu, Table widgets, and POI selection currently don’t set `FocusedBlockingWidget` when opened**.
So click/interact can leak through unless every blocking widget inherits your base and self-registers.

### A) `UFCUIManager::ShowPauseMenu()` / `HidePauseMenu()`

In `ShowPauseMenu()`, right after `AddToViewport(100);`, add:

```cpp
SetFocusedBlockingWidget(PauseMenuWidget); // NEW: blocks world click + interact while pause is open
```

`ShowPauseMenu()` currently doesn’t set it. 

In `HidePauseMenu()`, before (or after) removing the widget, add:

```cpp
if (FocusedBlockingWidget == PauseMenuWidget)
{
    SetFocusedBlockingWidget(nullptr); // NEW: unblocks world input
}
```

`HidePauseMenu()` currently restores input mode but doesn’t clear the blocker. 

---

### B) `UFCUIManager::ShowTableWidget()` / `CloseTableWidget()`

In `ShowTableWidget()`, right after `CurrentTableWidget->AddToViewport();`, add:

```cpp
SetFocusedBlockingWidget(CurrentTableWidget); // NEW
```

Right now it only adds to viewport. 

In `CloseTableWidget()`, before `CurrentTableWidget = nullptr;`, add:

```cpp
if (FocusedBlockingWidget == CurrentTableWidget)
{
    SetFocusedBlockingWidget(nullptr); // NEW
}
```

Right now it just removes and nulls. 

---

### C) `UFCUIManager::ShowPOIActionSelection()` / `ClosePOIActionSelection()`

In `ShowPOIActionSelection()`, right after `CurrentPOIActionSelectionWidget->AddToViewport();`, add:

```cpp
SetFocusedBlockingWidget(CurrentPOIActionSelectionWidget); // NEW
```

Right now it adds to viewport and returns. 

In `ClosePOIActionSelection()`, before `CurrentPOIActionSelectionWidget = nullptr;`, add:

```cpp
if (FocusedBlockingWidget == CurrentPOIActionSelectionWidget)
{
    SetFocusedBlockingWidget(nullptr); // NEW
}
```

Right now it removes + nulls only. 

---

### D) OverworldMap + ExpeditionSummary

These already set (and summary clears) `FocusedBlockingWidget`.
But one extra safety fix for the map close path (because the **controller** closes it):

In `AFCPlayerController::HandleToggleOverworldMap()`, inside the branch where you close the map widget, add this right before `RemoveFromParent()` (or right after):

```cpp
if (UFCGameInstance* GI = GetGameInstance<UFCGameInstance>())
{
    if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
    {
        if (UIManager->FocusedBlockingWidget == CurrentOverworldMapWidget)
        {
            UIManager->SetFocusedBlockingWidget(nullptr);
        }
    }
}
```

That prevents a stale blocker reference if the map widget doesn’t self-unregister.

---

## 8) Quick test checklist (updated for `CanWorldClick()` + `CanWorldInteract()`)

You want to prove **both queries** behave correctly (and consistently across widgets):

1. **Pause menu**

   * Open pause → `CanWorldClick()` should be false, `CanWorldInteract()` false
   * Close pause → both true again (in a gameplay mode)

2. **Table widget**

   * Click a table object to open its widget → both false
   * ESC closes table widget → both true again (still in table view / desk)

3. **POI action selection**

   * Trigger POI selection widget → both false
   * Choose an action / close widget → both true again

4. **Overworld map HUD**

   * Open map → both false
   * Close map → both true again, and no “stuck blocked input” feeling

5. **Expedition summary**

   * Show summary → both false
   * Close summary → both true again 