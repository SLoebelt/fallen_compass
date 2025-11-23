// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "World/FCOverworldPOI.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY(LogFCOverworldPOI);

AFCOverworldPOI::AFCOverworldPOI()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create component hierarchy
	POIRoot = CreateDefaultSubobject<USceneComponent>(TEXT("POIRoot"));
	SetRootComponent(POIRoot);

	// Create static mesh component
	POIMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("POIMesh"));
	POIMesh->SetupAttachment(POIRoot);
	POIMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	POIMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create interaction box for raycast and overlap
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(POIRoot);
	InteractionBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	InteractionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	InteractionBox->SetGenerateOverlapEvents(true);

	// Default POI name
	POIName = TEXT("Unnamed POI");
}

void AFCOverworldPOI::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s' spawned at %s"), 
		*POIName, *GetActorLocation().ToString());
}

void AFCOverworldPOI::OnPOIInteract_Implementation()
{
	// Stub implementation - logs to console
	UE_LOG(LogFCOverworldPOI, Log, TEXT("POI Interaction: %s"), *POIName);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			FString::Printf(TEXT("POI Interaction Stub: %s"), *POIName));
	}
}
