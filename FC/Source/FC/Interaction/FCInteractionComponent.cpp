// Copyright Epic Games, Inc. All Rights Reserved.

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
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Characters/Convoy/FCOverworldConvoy.h"

DEFINE_LOG_CATEGORY(LogFCInteraction);

UFCInteractionComponent::UFCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for responsive detection
}

void UFCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create the interaction prompt widget if a class is specified
	if (InteractionPromptWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
		if (PC)
		{
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
			UE_LOG(LogFCInteraction, Warning, TEXT("No PlayerController found for widget creation"));
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

	APlayerController* PC = Cast<APlayerController>(OwnerActor->GetInstigatorController());
	if (!PC)
	{
		CurrentInteractable = nullptr;
		return;
	}

	// Get camera location and direction
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * InteractionTraceDistance);

	// Perform line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
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

		UE_LOG(LogFCInteraction, Log, TEXT("Hit actor: %s (Implements interface: %d)"),
			*HitActor->GetName(),
			HitActor->GetClass()->ImplementsInterface(UIFCInteractable::StaticClass()) ? 1 : 0);

		// Check if the actor implements the interactable interface
		if (HitActor->GetClass()->ImplementsInterface(UIFCInteractable::StaticClass()))
		{
			// Check if within interaction range
			float InteractionRange = IIFCInteractable::Execute_GetInteractionRange(HitActor);
			float Distance = FVector::Dist(CameraLocation, HitResult.ImpactPoint);

			UE_LOG(LogFCInteraction, Log, TEXT("Hit interactable: %s at distance %.2f (range: %.2f)"), *HitActor->GetName(), Distance, InteractionRange);

			if (Distance <= InteractionRange)
			{
				// Check if we can interact with this object
				bool bCanInteract = IIFCInteractable::Execute_CanInteract(HitActor, OwnerActor);

				UE_LOG(LogFCInteraction, Log, TEXT("Can interact: %d"), bCanInteract ? 1 : 0);

				if (bCanInteract)
				{
					// New interactable found
					if (CurrentInteractable != HitActor)
					{
						CurrentInteractable = HitActor;
						UE_LOG(LogFCInteraction, Log, TEXT("New interactable in focus: %s"), *HitActor->GetName());
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
		APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
		if (PC)
		{
			FVector WorldPos = CurrentInteractable->GetActorLocation();
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
			{
				InteractionPromptWidget->SetPositionInViewport(ScreenPos);
			}
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
	if (!POIActor) return;

	// Query available actions via interface
	IIFCInteractablePOI* POIInterface = Cast<IIFCInteractablePOI>(POIActor);
	if (!POIInterface) return;

	TArray<FFCPOIActionData> AvailableActions = POIInterface->Execute_GetAvailableActions(POIActor);
	FString POIName = POIInterface->Execute_GetPOIName(POIActor);

	UE_LOG(LogFCInteraction, Log, TEXT("POI click on '%s', %d actions available"), *POIName, AvailableActions.Num());

	// Reset convoy-at-POI flag for right-click scenarios
	bConvoyAlreadyAtPOI = false;

	// Handle based on action count
	if (AvailableActions.Num() == 0)
	{
		// No actions available - ignore click
		UE_LOG(LogFCInteraction, Warning, TEXT("POI '%s' has no available actions"), *POIName);
		return;
	}
	else if (AvailableActions.Num() == 1)
	{
		// Single action - auto-select and trigger movement
		PendingInteractionPOI = POIActor;
		PendingInteractionAction = AvailableActions[0].ActionType;
		bHasPendingPOIInteraction = true;

		UE_LOG(LogFCInteraction, Log, TEXT("Auto-selected action '%s' for POI '%s'"), 
			*UEnum::GetValueAsString(PendingInteractionAction), *POIName);

		// Trigger convoy movement to POI
		AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC && PendingInteractionPOI)
		{
			FVector POILocation = PendingInteractionPOI->GetActorLocation();
			PC->MoveConvoyToLocation(POILocation);
			UE_LOG(LogFCInteraction, Log, TEXT("Triggered convoy movement to POI at %s (single action)"), *POILocation.ToString());
		}
	}
	else
	{
		// Multiple actions - delegate to UIManager for widget display
		UE_LOG(LogFCInteraction, Log, TEXT("Requesting action selection for POI '%s'"), *POIName);

		PendingInteractionPOI = POIActor;
		bHasPendingPOIInteraction = false; // Wait for selection

		// Get UIManager to show action selection widget
		UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetWorld()->GetGameInstance());
		if (GameInstance)
		{
			UFCUIManager* UIManager = GameInstance->GetSubsystem<UFCUIManager>();
			if (UIManager)
			{
				UIManager->ShowPOIActionSelection(AvailableActions, this);
				UE_LOG(LogFCInteraction, Log, TEXT("HandlePOIClick: Showing POI action selection widget"));
			}
			else
			{
				UE_LOG(LogFCInteraction, Error, TEXT("HandlePOIClick: Failed to get UIManager subsystem"));
			}
		}
		else
		{
			UE_LOG(LogFCInteraction, Error, TEXT("HandlePOIClick: Failed to get game instance"));
		}
	}
}

void UFCInteractionComponent::OnPOIActionSelected(EFCPOIAction SelectedAction)
{
	if (!PendingInteractionPOI)
	{
		UE_LOG(LogFCInteraction, Warning, TEXT("OnPOIActionSelected: No pending POI"));
		return;
	}

	// Store selected action
	PendingInteractionAction = SelectedAction;
	bHasPendingPOIInteraction = true;

	IIFCInteractablePOI* POIInterface = Cast<IIFCInteractablePOI>(PendingInteractionPOI);
	if (POIInterface)
	{
		FString POIName = POIInterface->Execute_GetPOIName(PendingInteractionPOI);
		UE_LOG(LogFCInteraction, Log, TEXT("Selected action '%s' for POI '%s'"), 
			*UEnum::GetValueAsString(SelectedAction), *POIName);
	}

	// Only trigger movement if convoy not already at POI
	if (!bConvoyAlreadyAtPOI)
	{
		// Right-click multiple-action scenario: move convoy to POI
		AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC && PendingInteractionPOI)
		{
			FVector POILocation = PendingInteractionPOI->GetActorLocation();
			PC->MoveConvoyToLocation(POILocation);
			UE_LOG(LogFCInteraction, Log, TEXT("Triggered convoy movement to POI at %s (right-click multiple)"), *POILocation.ToString());
		}
	}
	else
	{
		// Left-click scenario: convoy already at POI, execute immediately
		UE_LOG(LogFCInteraction, Log, TEXT("Convoy already at POI, executing action immediately"));
		bConvoyAlreadyAtPOI = false; // Reset flag
		NotifyPOIOverlap(PendingInteractionPOI);
	}
}

void UFCInteractionComponent::NotifyPOIOverlap(AActor* POIActor)
{
	if (!POIActor) return;

	IIFCInteractablePOI* POIInterface = Cast<IIFCInteractablePOI>(POIActor);
	if (!POIInterface) return;

	FString POIName = POIInterface->Execute_GetPOIName(POIActor);

	// Check if this is the pending interaction POI
	if (bHasPendingPOIInteraction && PendingInteractionPOI == POIActor)
	{
		// Execute pending action
		UE_LOG(LogFCInteraction, Log, TEXT("Executing pending action '%s' on POI '%s'"), 
			*UEnum::GetValueAsString(PendingInteractionAction), *POIName);

		POIInterface->Execute_ExecuteAction(POIActor, PendingInteractionAction, GetOwner());

		// Clear pending interaction
		bHasPendingPOIInteraction = false;
		PendingInteractionPOI = nullptr;

		// Clear convoy interaction flag
		AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			AFCOverworldConvoy* Convoy = PC->GetPossessedConvoy();
			if (Convoy)
			{
				Convoy->SetInteractingWithPOI(false);
				UE_LOG(LogFCInteraction, Log, TEXT("Cleared convoy interaction flag"));
			}
		}
	}
	else
	{
		// Unintentional overlap (exploration, fleeing, enemy chase)
		TArray<FFCPOIActionData> AvailableActions = POIInterface->Execute_GetAvailableActions(POIActor);

		if (AvailableActions.Num() == 0)
		{
			// No actions - ignore overlap
			UE_LOG(LogFCInteraction, Log, TEXT("Unintentional overlap with POI '%s' (no actions)"), *POIName);
		}
		else if (AvailableActions.Num() == 1)
		{
			// Single action - auto-execute
			UE_LOG(LogFCInteraction, Log, TEXT("Unintentional overlap: auto-executing '%s' on POI '%s'"), 
				*UEnum::GetValueAsString(AvailableActions[0].ActionType), *POIName);

			POIInterface->Execute_ExecuteAction(POIActor, AvailableActions[0].ActionType, GetOwner());

			// Clear convoy interaction flag
			AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
			if (PC)
			{
				AFCOverworldConvoy* Convoy = PC->GetPossessedConvoy();
				if (Convoy)
				{
					Convoy->SetInteractingWithPOI(false);
					UE_LOG(LogFCInteraction, Log, TEXT("Cleared convoy interaction flag after auto-execute"));
				}
			}
		}
		else
		{
			// Multiple actions - show selection dialog
			UE_LOG(LogFCInteraction, Log, TEXT("Unintentional overlap: showing action selection for POI '%s'"), *POIName);

			PendingInteractionPOI = POIActor;
			bHasPendingPOIInteraction = false; // Wait for selection
			bConvoyAlreadyAtPOI = true; // Convoy already at POI (left-click scenario)

			// Get UIManager to show action selection widget
			UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetWorld()->GetGameInstance());
			if (GameInstance)
			{
				UFCUIManager* UIManager = GameInstance->GetSubsystem<UFCUIManager>();
				if (UIManager)
				{
					UIManager->ShowPOIActionSelection(AvailableActions, this);
					UE_LOG(LogFCInteraction, Log, TEXT("NotifyPOIOverlap: Showing POI action selection widget"));
				}
				else
				{
					UE_LOG(LogFCInteraction, Error, TEXT("NotifyPOIOverlap: Failed to get UIManager subsystem"));
				}
			}
			else
			{
				UE_LOG(LogFCInteraction, Error, TEXT("NotifyPOIOverlap: Failed to get game instance"));
			}
		}
	}
}

