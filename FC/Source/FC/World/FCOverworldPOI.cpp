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

	// Default: no actions (must be configured in Blueprint)
	AvailableActions.Empty();
}

void AFCOverworldPOI::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s' spawned at %s"), 
		*POIName, *GetActorLocation().ToString());
}

// IFCInteractablePOI interface implementation
TArray<FFCPOIActionData> AFCOverworldPOI::GetAvailableActions_Implementation() const
{
	return AvailableActions;
}

void AFCOverworldPOI::ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor)
{
	// Stub implementation - log action
	UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s': Executing action %s"), 
		*POIName, *UEnum::GetValueAsString(Action));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
			FString::Printf(TEXT("POI '%s': Action '%s' executed (STUB)"), 
				*POIName, *UEnum::GetValueAsString(Action)));
	}

	// Future: Implement actual action logic (open dialog, start trade, etc.)
}

FString AFCOverworldPOI::GetPOIName_Implementation() const
{
	return POIName;
}

bool AFCOverworldPOI::CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const
{
	// Default: all actions allowed
	// Override in Blueprint for quest requirements, locked doors, etc.
	return true;
}

// Keep old OnPOIInteract for backward compatibility
void AFCOverworldPOI::OnPOIInteract_Implementation()
{
	UE_LOG(LogFCOverworldPOI, Warning, TEXT("POI '%s': OnPOIInteract is deprecated, use ExecuteAction()"), *POIName);
}
