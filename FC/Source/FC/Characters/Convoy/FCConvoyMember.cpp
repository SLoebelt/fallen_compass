// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCConvoyMember.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Components/CapsuleComponent.h"
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
	if (ParentConvoy)
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Notifying parent convoy of POI overlap"), *GetName());
		ParentConvoy->NotifyPOIOverlap(POIActor);
	}
	else
	{
		UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No parent convoy to notify!"), *GetName());
	}
}
