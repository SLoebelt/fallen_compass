// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IFCInteractablePOI.h"
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
 * Implements IFCInteractablePOI for action-based interaction.
 */
UCLASS()
class FC_API AFCOverworldPOI : public AActor, public IIFCInteractablePOI
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

	/** Available actions for this POI (configured in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI|Actions", meta = (AllowPrivateAccess = "true"))
	TArray<FFCPOIActionData> AvailableActions;

public:
	// IFCInteractablePOI interface implementation
	virtual TArray<FFCPOIActionData> GetAvailableActions_Implementation() const override;
	virtual void ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) override;
	virtual FString GetPOIName_Implementation() const override;
	virtual bool CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const override;

	/** DEPRECATED: Old stub method - replaced by ExecuteAction() */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
	void OnPOIInteract();
};
