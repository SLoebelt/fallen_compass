// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FCExpeditionData.generated.h"

/**
 * EFCExpeditionStatus
 * 
 * Represents the current state of an expedition.
 * Used to track progression from planning through completion/failure.
 */
UENUM(BlueprintType)
enum class EFCExpeditionStatus : uint8
{
	Planning    UMETA(DisplayName = "Planning"),
	InProgress  UMETA(DisplayName = "In Progress"),
	Completed   UMETA(DisplayName = "Completed"),
	Failed      UMETA(DisplayName = "Failed")
};

/**
 * UFCExpeditionData
 *
 * Data object representing a single expedition instance.
 * Stores metadata about the expedition (name, target, supplies, status).
 * Will be expanded in Weeks 9-13 with route planning, crew, and resources.
 */
UCLASS(BlueprintType)
class FC_API UFCExpeditionData : public UObject
{
	GENERATED_BODY()

public:
	/** Display name for this expedition */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition")
	FString ExpeditionName;

	/** Start date/time (placeholder, will be DateTime in future) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition")
	FString StartDate;

	/** Target region name (placeholder string, will be data asset reference in Week 9) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition")
	FString TargetRegion;

	/** Starting supplies allocated to this expedition */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition")
	int32 StartingSupplies;

	/** Current status of the expedition */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition")
	EFCExpeditionStatus ExpeditionStatus;

	/** Selected planning grid area (INDEX_NONE if unset) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 SelectedGridId;

	/** Selected start grid when planning (INDEX_NONE if unset) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 SelectedStartGridId;

	/** Selected start subcell index (INDEX_NONE if unset) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 SelectedStartSubId;

	/** Preview target grid for milestone route (INDEX_NONE if unset) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 PreviewTargetGridId;

	/** Preview target subcell for milestone route (INDEX_NONE if unset) */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 PreviewTargetSubId;

	/** Cached planned route as global subcell ids */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	TArray<int32> PlannedRouteGlobalIds;

	/** Cached money cost for the planned route */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 PlannedMoneyCost;

	/** Cached risk cost for the planned route */
	UPROPERTY(BlueprintReadWrite, Category = "Expedition|Planning")
	int32 PlannedRiskCost;

	/** Constructor with default values */
	UFCExpeditionData();
};
