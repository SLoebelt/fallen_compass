// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCConvoyMember.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Components/CapsuleComponent.h"
#include "Core/FCPlayerController.h"
#include "Core/FCFirstPersonCharacter.h"
#include "Interaction/FCInteractionComponent.h"
#include "FC.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCConvoyMember, Log, All);

AFCConvoyMember::AFCConvoyMember()
{
	PrimaryActorTick.bCanEverTick = false;

	// Configure capsule component
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->SetCapsuleHalfHeight(100.0f);
		CapsuleComp->SetCapsuleRadius(50.0f);
		CapsuleComp->SetGenerateOverlapEvents(true);
	}

	// Enable AI controller auto-possession
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AFCConvoyMember::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event to capsule component
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AFCConvoyMember::OnCapsuleBeginOverlap);
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Bound capsule overlap event"), *GetName());
	}
	else
	{
		UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No capsule component found!"), *GetName());
	}
}

void AFCConvoyMember::OnCapsuleBeginOverlap(
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

	// Check if actor implements BPI_InteractablePOI interface
	// Note: Interface check will work once BPI_InteractablePOI is created
	// For now, we'll use a simple class name check as fallback
	if (OtherActor->GetClass()->GetName().Contains(TEXT("POI")))
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Detected overlap with potential POI %s"), 
			*GetName(), *OtherActor->GetName());
		NotifyPOIOverlap(OtherActor);
	}
}

void AFCConvoyMember::SetParentConvoy(AFCOverworldConvoy* InConvoy)
{
	ParentConvoy = InConvoy;
	UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Parent convoy set to %s"), 
		*GetName(), InConvoy ? *InConvoy->GetName() : TEXT("null"));
}

void AFCConvoyMember::NotifyPOIOverlap(AActor* POIActor)
{
	// Check if parent convoy is already interacting
	if (ParentConvoy && ParentConvoy->IsInteractingWithPOI())
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Parent convoy already interacting, skipping notification"), *GetName());
		return;
	}

	// Get player controller's InteractionComponent
	AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		// Get the InteractionComponent from the player's character
		AFCFirstPersonCharacter* FPCharacter = Cast<AFCFirstPersonCharacter>(PC->GetPawn());
		if (FPCharacter)
		{
			UFCInteractionComponent* InteractionComp = FPCharacter->GetInteractionComponent();
			if (InteractionComp)
			{
				UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Notifying InteractionComponent of POI overlap"), *GetName());
				InteractionComp->NotifyPOIOverlap(POIActor);
				return;
			}
			else
			{
				UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No InteractionComponent on character"), *GetName());
			}
		}
		else
		{
			UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: Player pawn is not AFCFirstPersonCharacter"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No player controller found"), *GetName());
	}

	// Fallback: notify parent convoy (old behavior for backwards compatibility)
	if (ParentConvoy)
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Falling back to parent convoy notification"), *GetName());
		ParentConvoy->NotifyPOIOverlap(POIActor);
	}
}
