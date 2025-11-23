// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FCOverworldConvoy.generated.h"

class AFCConvoyMember;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConvoyPOIOverlap, AActor*, POIActor);

/**
 * AFCOverworldConvoy - Base class for convoy parent actor
 * 
 * Manages array of AConvoyMember actors, handles spawning, and aggregates POI overlap events.
 * Blueprint children configure spawn point locations and visual properties.
 */
UCLASS()
class FC_API AFCOverworldConvoy : public AActor
{
	GENERATED_BODY()

public:
	AFCOverworldConvoy();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	/** Array of convoy member actors */
	UPROPERTY()
	TArray<AFCConvoyMember*> ConvoyMembers;

	/** Reference to leader member */
	UPROPERTY()
	AFCConvoyMember* LeaderMember;

	/** Blueprint class to spawn for convoy members */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFCConvoyMember> ConvoyMemberClass;

	/** Root component for convoy hierarchy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ConvoyRoot;

	/** Camera attachment point (elevated for top-down view) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	USceneComponent* CameraAttachPoint;

	/** Leader spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	USceneComponent* LeaderSpawnPoint;

	/** Follower 1 spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Follower1SpawnPoint;

	/** Follower 2 spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Follower2SpawnPoint;

	/** Spawn convoy members at spawn points */
	void SpawnConvoyMembers();

public:
	/** Get leader member reference */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	AFCConvoyMember* GetLeaderMember() const { return LeaderMember; }

	/** Get camera attachment point for camera following */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	USceneComponent* GetCameraAttachPoint() const { return CameraAttachPoint; }

	/** Called by convoy members when they overlap a POI */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	void NotifyPOIOverlap(AActor* POIActor);

	/** Event dispatcher for POI overlap */
	UPROPERTY(BlueprintAssignable, Category = "FC|Convoy|Events")
	FOnConvoyPOIOverlap OnConvoyPOIOverlap;
};
