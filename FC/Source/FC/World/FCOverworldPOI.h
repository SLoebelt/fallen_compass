// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FCOverworldPOI.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogFCOverworldPOI, Log, All);

/**
 * AFCOverworldPOI - Base class for overworld Points of Interest
 * 
 * Provides collision for mouse raycast detection and convoy overlap.
 * Blueprint children configure specific mesh, materials, and POI names.
 */
UCLASS()
class FC_API AFCOverworldPOI : public AActor
{
	GENERATED_BODY()

public:
	AFCOverworldPOI();

protected:
	virtual void BeginPlay() override;

private:
	/** Root component for POI hierarchy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
	USceneComponent* POIRoot;

	/** Static mesh for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* POIMesh;

	/** Collision box for mouse raycast and convoy overlap detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* InteractionBox;

	/** Display name for this Point of Interest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
	FString POIName;

public:
	/** Get POI display name */
	UFUNCTION(BlueprintCallable, Category = "FC|POI")
	FString GetPOIName() const { return POIName; }

	/** Handle POI interaction (stub for Task 6 - will log to console) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
	void OnPOIInteract();
};
