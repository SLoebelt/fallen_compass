// Copyright Slomotion Games. All Rights Reserved.
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


#include "FCInteractionComponent.h"
#include "IFCInteractable.h"
#include "IFCInteractablePOI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCUIManager.h"
#include "Core/FCPlayerController.h"
#include "Components/FCCameraManager.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Interaction/FCInteractionProfile.h"

DEFINE_LOG_CATEGORY(LogFCInteraction);

AFCPlayerController* UFCInteractionComponent::GetOwnerPCCheckedOrNull() const
{
	AFCPlayerController* PC = OwnerPC.Get();
	if (!PC && !bLoggedMissingOwnerPC)
	{
		bLoggedMissingOwnerPC = true;
		UE_LOG(LogFCInteraction, Error,
			TEXT("UFCInteractionComponent owner is not AFCPlayerController (Owner=%s). Interaction prompts/traces will not work."),
			*GetNameSafe(GetOwner()));
	}
	return PC;
}

UFCInteractionComponent::UFCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for responsive detection
}

void UFCInteractionComponent::OnRegister()
{
	Super::OnRegister();
	OwnerPC = Cast<AFCPlayerController>(GetOwner());
}

void UFCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPC = Cast<AFCPlayerController>(GetOwner());
	GetOwnerPCCheckedOrNull(); // triggers one-time error if wrong owner

	// Create the interaction prompt widget if a class is specified
	if (InteractionPromptWidgetClass)
	{
		AFCPlayerController* PC = GetOwnerPCCheckedOrNull();
		if (!PC) { return; }

		InteractionPromptWidget = CreateWidget<UUserWidget>(PC, InteractionPromptWidgetClass);
		if (InteractionPromptWidget)
		{
			InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
			InteractionPromptWidget->AddToViewport();
			UE_LOG(LogFCInteraction, Log, TEXT("Interaction prompt widget created and added to viewport"));
		}
		else
		{
			UE_LOG(LogFCInteraction, Error, TEXT("Failed to create interaction prompt widget"));
		}
	}
	else
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("InteractionPromptWidgetClass is not set on InteractionComponent"));
	}
}

void UFCInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwnerPCCheckedOrNull())
	{
		return;
	}

    if (!bFirstPersonFocusEnabled)
    {
        // Setter already cleared focus when disabled; no trace or prompt updates in non-FP modes.
        return;
    }

	// Rate-limit interaction checks for performance
	InteractionCheckTimer += DeltaTime;
	if (InteractionCheckTimer >= InteractionCheckFrequency)
	{
		InteractionCheckTimer = 0.0f;
		DetectInteractables();
		UpdatePromptWidget();
	}
}

void UFCInteractionComponent::DetectInteractables()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		CurrentInteractable = nullptr;
		return;
	}

	AFCPlayerController* PC = GetOwnerPCCheckedOrNull();

	if (!PC)
	{
		CurrentInteractable = nullptr;
		return;
	}

	// If UI (e.g., table/map widgets) is currently blocking interaction,
	// skip world traces so prompts and logs aren't spammed behind UIs.
	if (!PC->CanWorldInteract())
	{
		CurrentInteractable = nullptr;
		return;
	}

	// Resolve interaction profile to drive trace config.
    UFCInteractionProfile* Profile = GetEffectiveProfile();

    // Get camera location and direction
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const float Range = Profile ? Profile->Range : InteractionTraceDistance;
    ECollisionChannel Channel = ECC_Visibility;
	if (Profile)
	{
		Channel = static_cast<ECollisionChannel>(Profile->TraceChannel.GetValue());
	}

    const FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * Range);

    // Perform line trace (probe type = LineTrace for now; extend later if needed).
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerActor);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TraceEnd,
        Channel,
        QueryParams
    );

	// Debug visualization
	if (bShowDebugTrace)
	{
		DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, bHit ? FColor::Green : FColor::Red, false, InteractionCheckFrequency, 0, 2.0f);
		if (bHit)
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 8, FColor::Yellow, false, InteractionCheckFrequency);
		}
	}

	// Check if we hit an interactable
    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();

        // Optional tag filtering from profile
        if (Profile && Profile->AllowedTags.Num() > 0)
        {
            bool bTagOk = false;
            for (const FName Tag : Profile->AllowedTags)
            {
                if (HitActor->ActorHasTag(Tag))
                {
                    bTagOk = true;
                    break;
                }
            }

            if (!bTagOk)
            {
                // Ignore this hit completely if it does not match the profile's tag filter.
                CurrentInteractable = nullptr;
                return;
            }
        }

        // Check if the actor implements the interactable interface
        if (HitActor->GetClass()->ImplementsInterface(UIFCInteractable::StaticClass()))
        {
            // Check if within interaction range (per-object); still using the interface range.
            const float InteractionRange = IIFCInteractable::Execute_GetInteractionRange(HitActor);
            const float Distance = FVector::Dist(CameraLocation, HitResult.ImpactPoint);

            if (Distance <= InteractionRange)
            {
                const bool bCanInteract = IIFCInteractable::Execute_CanInteract(HitActor, OwnerActor);

                if (bCanInteract)
                {
                    if (CurrentInteractable != HitActor)
                    {
                        CurrentInteractable = HitActor;
                        UE_LOG(LogFCInteraction, Log,
                            TEXT("New interactable in focus: %s"),
                            *HitActor->GetName());
                    }
                    return;
                }
            }
        }
    }

	// No valid interactable found
	if (CurrentInteractable.IsValid())
	{
		UE_LOG(LogFCInteraction, Verbose, TEXT("Lost focus on interactable"));
		CurrentInteractable = nullptr;
	}
}

void UFCInteractionComponent::UpdatePromptWidget()
{
	if (!InteractionPromptWidget)
	{
		return;
	}

	if (CurrentInteractable.IsValid())
	{
		// Get the prompt text from the interactable
		FText PromptText = IIFCInteractable::Execute_GetInteractionPrompt(CurrentInteractable.Get());

		// Call the widget's SetInteractionPrompt function (must exist in blueprint)
		UFunction* SetPromptFunction = InteractionPromptWidget->FindFunction(FName("SetInteractionPrompt"));
		if (SetPromptFunction)
		{
			InteractionPromptWidget->ProcessEvent(SetPromptFunction, &PromptText);
		}
		else
		{
			UE_LOG(LogFCInteraction, Warning, TEXT("WBP_InteractionPrompt is missing SetInteractionPrompt function"));
		}

		// Position the widget at the interactable's screen position
		AFCPlayerController* PC = GetOwnerPCCheckedOrNull();
		if (!PC) { return; }

		FVector WorldPos = CurrentInteractable->GetActorLocation();
		FVector2D ScreenPos;
		if (PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
		{
			InteractionPromptWidget->SetPositionInViewport(ScreenPos);
		}

		InteractionPromptWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFCInteractionComponent::Interact()
{
	if (!CurrentInteractable.IsValid())
	{
		UE_LOG(LogFCInteraction, Verbose, TEXT("Interact called but no valid interactable in focus"));
		return;
	}

	AActor* InteractableActor = CurrentInteractable.Get();

	// Double-check that we can still interact (conditions may have changed)
	bool bCanInteract = IIFCInteractable::Execute_CanInteract(InteractableActor, GetOwner());
	if (!bCanInteract)
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("Cannot interact with %s (conditions not met)"), *InteractableActor->GetName());
		return;
	}

	// Execute the interaction
	UE_LOG(LogFCInteraction, Log, TEXT("Interacting with: %s"), *InteractableActor->GetName());
	IIFCInteractable::Execute_OnInteract(InteractableActor, GetOwner());
}

void UFCInteractionComponent::HandlePOIClick(AActor* POIActor)
{
	// New click always resets stale pending state (prevents ghost actions or stacked moves).
	ResetInteractionState();

	if (!IsValid(POIActor))
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("[Interaction] Click ignored: invalid POI"));
		return;
	}

	// Treat this click as the new intent source of truth.
	FocusedTarget = POIActor;
	PendingPOI = POIActor;
	bPendingPOIAlreadyReached = false;

	// Query available actions via POI interface.
	if (!POIActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("[Interaction] Clicked actor %s is not a POI"), *GetNameSafe(POIActor));
		return;
	}

	const TArray<FFCPOIActionData> AvailableActions = IIFCInteractablePOI::Execute_GetAvailableActions(POIActor);
	const FString POIName = IIFCInteractablePOI::Execute_GetPOIName(POIActor);

	UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] POI click on '%s', %d actions available"), *POIName, AvailableActions.Num());

	if (AvailableActions.Num() == 0)
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("[Interaction] POI '%s' has no available actions"), *POIName);
		return;
	}

	AFCPlayerController* PC = GetOwnerPCCheckedOrNull();
	if (!PC)
	{
		return;
	}

	UFCCameraManager* CameraManager = PC->FindComponentByClass<UFCCameraManager>();

	// Helper lambda: issue exactly one movement command based on current camera mode.
	auto IssueMovementToPOI = [&]()
	{
		const FVector POILocation = POIActor->GetActorLocation();

		if (CameraManager && CameraManager->GetCameraMode() == EFCPlayerCameraMode::POIScene)
		{
			PC->MoveExplorerToLocation(POILocation);
		}
		else
		{
			PC->MoveConvoyToLocation(POILocation);
		}
	};

	// Helper lambda: compute whether movement is actually required.
	auto NeedsMovement = [&]() -> bool
	{
		const AActor* OwnerActor = GetOwner();
		if (!OwnerActor || !IsValid(POIActor))
		{
			return false;
		}

		const FVector CurrentLocation = OwnerActor->GetActorLocation();
		const FVector TargetLocation = POIActor->GetActorLocation();
		const float DistanceToPOI = FVector::Dist(CurrentLocation, TargetLocation);

		// Tunable threshold; kept conservative to avoid jittery re-issues.
		constexpr float ClickAcceptRadius = 100.0f;
		return DistanceToPOI > ClickAcceptRadius;
	};

	// Multi-action path: go into Selecting phase first, movement (if any) is decided on selection.
	if (AvailableActions.Num() > 1)
	{
		bAwaitingSelection = true;
		bAwaitingArrival = false;

		if (UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GameInstance->GetSubsystem<UFCUIManager>())
			{
				UIManager->ShowPOIActionSelection(AvailableActions, this);

				const AActor* PendingPOIActor = PendingPOI.Get();
				UE_LOG(LogFCInteraction, Log,
					TEXT("[Interaction] Phase=Selecting POI=%s"),
					*GetNameSafe(PendingPOIActor));
			}
		}

		return;
	}

	// Single-action path: we know the action immediately and can decide movement now.
	check(AvailableActions.Num() == 1);

	PendingAction = AvailableActions[0].ActionType;

	const bool bNeedsMove = NeedsMovement();
	if (bNeedsMove)
	{
		bAwaitingArrival = true;
		IssueMovementToPOI();

		const AActor* PendingPOIActor = PendingPOI.Get();
		UE_LOG(LogFCInteraction, Log,
			TEXT("[Interaction] Phase=MovingToPOI POI=%s Action=%s"),
			*GetNameSafe(PendingPOIActor),
			*UEnum::GetValueAsString(PendingAction.GetValue()));
	}
	else
	{
		// Already at POI: treat this as an immediate arrival, no movement.
		bAwaitingArrival = false;
		UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Phase=Executing (already at POI) POI=%s Action=%s"),
			*GetNameSafe(PendingPOI.Get()),
			*UEnum::GetValueAsString(PendingAction.GetValue()));

		NotifyArrivedAtPOI(POIActor);
	}
}

void UFCInteractionComponent::OnPOIActionSelected(EFCPOIAction SelectedAction)
{
	if (!bAwaitingSelection || !PendingPOI.IsValid())
    {
        UE_LOG(LogFCInteraction, Warning, TEXT("[Interaction] Selection ignored: not awaiting selection"));
        return;
    }

    PendingAction = SelectedAction;
    bAwaitingSelection = false;

    UE_LOG(LogFCInteraction, Log, TEXT("[Interaction] Selected action %s for POI %s"),
        *UEnum::GetValueAsString(SelectedAction),
        *GetNameSafe(PendingPOI.Get()));

	if (bPendingPOIAlreadyReached)
	{
		bPendingPOIAlreadyReached = false;

		// Execute immediately at current overlap location (no movement, no overlap call)
		ExecutePOIActionNow(PendingPOI.Get(), PendingAction.GetValue());
		return;
	}

    // Not at POI yet: move now and wait for arrival
    bAwaitingArrival = true;

    AFCPlayerController* PC = GetOwnerPCCheckedOrNull();
    if (!PC) return;

    const FVector POILocation = PendingPOI->GetActorLocation();
    UFCCameraManager* CameraManager = PC->FindComponentByClass<UFCCameraManager>();

    if (CameraManager && CameraManager->GetCameraMode() == EFCPlayerCameraMode::POIScene)
    {
        PC->MoveExplorerToLocation(POILocation);
    }
    else
    {
        PC->MoveConvoyToLocation(POILocation);
    }
}


void UFCInteractionComponent::NotifyPOIOverlap(AActor* POIActor)
{
	if (!IsValid(POIActor))
	{
		return;
	}

	UE_LOG(LogFCInteraction, Log, TEXT("NotifyPOIOverlap received: %s"), *GetNameSafe(POIActor));

	// Robust interface check (works for BP implementations too)
	if (!POIActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
	{
		return;
	}

	const FString POIName = IIFCInteractablePOI::Execute_GetPOIName(POIActor);

	// Pending execution path: execute ONLY if this is our pending POI and we have a selected action.
	if (PendingPOI.IsValid() && PendingPOI.Get() == POIActor && PendingAction.IsSet() && !bAwaitingSelection)
	{
		ExecutePOIActionNow(POIActor, PendingAction.GetValue());
		return;
	}

	// Unintentional overlap (exploration, fleeing, enemy chase)
	const TArray<FFCPOIActionData> AvailableActions = IIFCInteractablePOI::Execute_GetAvailableActions(POIActor);

	if (AvailableActions.Num() == 0)
	{
		UE_LOG(LogFCInteraction, Log, TEXT("Unintentional overlap with POI '%s' (no actions)"), *POIName);
		return;
	}
	else if (AvailableActions.Num() == 1)
	{
		ExecutePOIActionNow(POIActor, AvailableActions[0].ActionType);
		return;
	}
	else // > 1
	{
		ResetInteractionState();
		FocusedTarget = POIActor;
		PendingPOI = POIActor;
		bAwaitingSelection = true;
		bPendingPOIAlreadyReached = true;

		if (UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GameInstance->GetSubsystem<UFCUIManager>())
			{
				UIManager->ShowPOIActionSelection(AvailableActions, this);
			}
		}
	}
}

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



void UFCInteractionComponent::SetFirstPersonFocusEnabled(bool bEnabled)
{
    if (bFirstPersonFocusEnabled == bEnabled)
    {
        return;
    }

    bFirstPersonFocusEnabled = bEnabled;

    if (!bFirstPersonFocusEnabled)
    {
        ClearFocusAndHidePrompt();
    }
}

void UFCInteractionComponent::ClearFocusAndHidePrompt()
{
    // Use your actual member names for current focus + prompt widget.
    CurrentInteractable = nullptr;

    if (InteractionPromptWidget)
    {
        InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

EFCInteractionPhase UFCInteractionComponent::GetCurrentInteractionPhase() const
{
    if (bAwaitingSelection) return EFCInteractionPhase::Selecting;
    if (bAwaitingArrival)   return EFCInteractionPhase::MovingToPOI;

    // Usually "Executing" is momentary; this is mostly for logs.
    if (PendingPOI.IsValid() && PendingAction.IsSet())
    {
        return EFCInteractionPhase::Executing;
    }

    return EFCInteractionPhase::Idle;
}

void UFCInteractionComponent::ResetInteractionState()
{
    bAwaitingSelection = false;
    bAwaitingArrival = false;
    bPendingPOIAlreadyReached = false;

    PendingPOI.Reset();
    PendingAction.Reset();

    // Keep FocusedTarget if you want POI highlighting to persist; otherwise reset it too:
    // FocusedTarget.Reset();
}

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

	// Preserve your existing convoy gate cleanup where applicable
	if (AFCPlayerController* PC = GetOwnerPCCheckedOrNull())
	{
		if (AFCOverworldConvoy* Convoy = PC->GetActiveConvoy())
		{
			Convoy->SetInteractingWithPOI(false);
		}
	}
}

UFCInteractionProfile* UFCInteractionComponent::GetEffectiveProfile() const
{
    if (ActiveProfile && ActiveProfile->IsValidProfile())
    {
        return ActiveProfile;
    }

    if (DefaultProfile.IsNull())
    {
        return nullptr;
    }

    UFCInteractionProfile* Loaded = DefaultProfile.Get();
    if (!Loaded)
    {
        Loaded = DefaultProfile.LoadSynchronous();
    }

    if (Loaded && Loaded->IsValidProfile())
    {
        return Loaded;
    }

    return nullptr;
}


void UFCInteractionComponent::ApplyInteractionProfile(UFCInteractionProfile* NewProfile)
{
    ActiveProfile = (NewProfile && NewProfile->IsValidProfile()) ? NewProfile : nullptr;

    // Optional: rebuild prompt widget if class differs
    ClearFocusAndHidePrompt();
}
