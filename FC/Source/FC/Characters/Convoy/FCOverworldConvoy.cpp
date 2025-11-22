// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Characters/Convoy/FCConvoyMember.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "FC.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCOverworldConvoy, Log, All);

AFCOverworldConvoy::AFCOverworldConvoy()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create component hierarchy
	ConvoyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ConvoyRoot"));
	SetRootComponent(ConvoyRoot);

	CameraAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAttachPoint"));
	CameraAttachPoint->SetupAttachment(ConvoyRoot);
	CameraAttachPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

	LeaderSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeaderSpawnPoint"));
	LeaderSpawnPoint->SetupAttachment(ConvoyRoot);
	LeaderSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Follower1SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower1SpawnPoint"));
	Follower1SpawnPoint->SetupAttachment(ConvoyRoot);
	Follower1SpawnPoint->SetRelativeLocation(FVector(-150.0f, 0.0f, 0.0f));

	Follower2SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower2SpawnPoint"));
	Follower2SpawnPoint->SetupAttachment(ConvoyRoot);
	Follower2SpawnPoint->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f));
}

void AFCOverworldConvoy::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: BeginPlay"), *GetName());
}

void AFCOverworldConvoy::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SpawnConvoyMembers();
}

void AFCOverworldConvoy::SpawnConvoyMembers()
{
	// Clear existing members
	for (AFCConvoyMember* Member : ConvoyMembers)
	{
		if (Member)
		{
			Member->Destroy();
		}
	}
	ConvoyMembers.Empty();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn leader
	AFCConvoyMember* Leader = World->SpawnActor<AFCConvoyMember>(
		AFCConvoyMember::StaticClass(),
		LeaderSpawnPoint->GetComponentLocation(),
		LeaderSpawnPoint->GetComponentRotation(),
		SpawnParams
	);

	if (Leader)
	{
		Leader->AttachToComponent(LeaderSpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Leader->SetParentConvoy(this);
		LeaderMember = Leader;
		ConvoyMembers.Add(Leader);
		UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned leader"), *GetName());
	}

	// Spawn follower 1
	AFCConvoyMember* Follower1 = World->SpawnActor<AFCConvoyMember>(
		AFCConvoyMember::StaticClass(),
		Follower1SpawnPoint->GetComponentLocation(),
		Follower1SpawnPoint->GetComponentRotation(),
		SpawnParams
	);

	if (Follower1)
	{
		Follower1->AttachToComponent(Follower1SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Follower1->SetParentConvoy(this);
		ConvoyMembers.Add(Follower1);
		UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 1"), *GetName());
	}

	// Spawn follower 2
	AFCConvoyMember* Follower2 = World->SpawnActor<AFCConvoyMember>(
		AFCConvoyMember::StaticClass(),
		Follower2SpawnPoint->GetComponentLocation(),
		Follower2SpawnPoint->GetComponentRotation(),
		SpawnParams
	);

	if (Follower2)
	{
		Follower2->AttachToComponent(Follower2SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Follower2->SetParentConvoy(this);
		ConvoyMembers.Add(Follower2);
		UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 2"), *GetName());
	}

	UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned %d total members"), *GetName(), ConvoyMembers.Num());
}

void AFCOverworldConvoy::NotifyPOIOverlap(AActor* POIActor)
{
	if (!POIActor)
	{
		return;
	}

	// Get POI name (simplified - will use interface later)
	FString POIName = POIActor->GetName();
	
	UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s detected POI: %s"), *GetName(), *POIName);
	
	// Display on-screen message
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
			FString::Printf(TEXT("Convoy detected POI: %s"), *POIName));
	}

	// Broadcast event
	OnConvoyPOIOverlap.Broadcast(POIActor);
}
