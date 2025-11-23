// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCConvoyMember.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Components/CapsuleComponent.h"
#include "Core/FCPlayerController.h"
#include "Core/FCFirstPersonCharacter.h"
#include "Interaction/FCInteractionComponent.h"
#include "Interaction/IFCInteractablePOI.h"
#include "AIController.h"
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

	// Check if actor implements IFCInteractablePOI interface
	if (OtherActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Detected overlap with POI %s"), 
			*GetName(), *OtherActor->GetName());
		
		// Notify parent convoy to coordinate stop and interaction
		if (ParentConvoy)
		{
			ParentConvoy->HandlePOIOverlap(OtherActor);
		}
		else
		{
			UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No parent convoy reference"), *GetName());
		}
	}
}

void AFCConvoyMember::SetParentConvoy(AFCOverworldConvoy* InConvoy)
{
	ParentConvoy = InConvoy;
	UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Parent convoy set to %s"), 
		*GetName(), InConvoy ? *InConvoy->GetName() : TEXT("null"));
}
