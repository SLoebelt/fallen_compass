// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "UFCWorldMapManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCWorldMapManager, Log, All);

/**
 * FFCMapAreaData
 * 
 * Data-driven metadata for a map area (2km × 2km grid cell).
 * Used as a row in the DT_MapAreas DataTable.
 * 
 * Properties:
 * - AreaID: Unique identifier for this area
 * - AreaName: Display name (localized)
 * - Description: Lore/flavor text
 * - LevelName: Corresponding Overworld level (e.g., "L_Overworld_Forest")
 * - GridPosition: Position on world map grid
 * - StartPoints: List of 3 fixed start point IDs for this area
 * - ExploredSubCells: 12×12 sub-grid (144 booleans) for gradual fog of war unveiling (~166m per cell)
 */
USTRUCT(BlueprintType)
struct FFCMapAreaData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this area */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	FName AreaID;

	/** Display name (localized) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	FText AreaName;

	/** Lore/flavor text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	FText Description;

	/** Corresponding Overworld level (e.g., "L_Overworld_Forest") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	FName LevelName;

	/** Position on world map grid (2km × 2km grid) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	FIntPoint GridPosition;

	/** List of 3 fixed start point IDs for this area */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Area")
	TArray<FName> StartPoints;

	/** 12×12 sub-grid (144 booleans) for gradual fog of war unveiling (~166m per cell) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Area")
	TArray<bool> ExploredSubCells;

	/** Default constructor - initializes ExploredSubCells to 144 false entries */
	FFCMapAreaData()
		: AreaID(NAME_None)
		, AreaName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, LevelName(NAME_None)
		, GridPosition(FIntPoint::ZeroValue)
	{
		// Initialize 12×12 sub-grid (144 cells) to unexplored
		ExploredSubCells.Init(false, 144);
	}
};

/**
 * FFCStartPointData
 * 
 * Data-driven metadata for a start point within an area.
 * Each area has 3 start points with varying risk/cost profiles.
 * Used as a row in the DT_StartPoints DataTable.
 * 
 * Properties:
 * - StartPointID: Unique identifier
 * - StartPointName: Display name (e.g., "Northern Coast", "Mountain Pass")
 * - AreaID: Parent area reference
 * - RiskLevel: Event probability multiplier (0-100)
 * - MoneyCost: Gold required to reach this point
 * - SupplyCost: Provisions/days required
 * - SplinePoints: Route waypoints from office (bottom center) to start point
 * - Description: Tactical info for player
 */
USTRUCT(BlueprintType)
struct FFCStartPointData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	FName StartPointID;

	/** Display name (e.g., "Northern Coast", "Mountain Pass") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	FText StartPointName;

	/** Parent area reference */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	FName AreaID;

	/** Event probability multiplier (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	int32 RiskLevel;

	/** Gold required to reach this point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	int32 MoneyCost;

	/** Provisions/days required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	int32 SupplyCost;

	/** Route waypoints from office (bottom center) to start point (2D map coordinates) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	TArray<FVector2D> SplinePoints;

	/** Tactical info for player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Start Point")
	FText Description;

	/** Default constructor */
	FFCStartPointData()
		: StartPointID(NAME_None)
		, StartPointName(FText::GetEmpty())
		, AreaID(NAME_None)
		, RiskLevel(0)
		, MoneyCost(0)
		, SupplyCost(0)
		, Description(FText::GetEmpty())
	{
	}
};

/**
 * UFCWorldMapManager
 * 
 * Central subsystem for world map data, exploration state, and route calculation.
 * Manages DataTables for areas and start points, tracks fog of war exploration,
 * and provides accessors for the expedition planning UI.
 * 
 * Following existing subsystem architecture (like UFCLevelManager, UFCUIManager).
 */
UCLASS()
class FC_API UFCWorldMapManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** USubsystem interface */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Lookup area data by ID
	 * @param AreaID - Unique area identifier
	 * @param OutAreaData - Area data (output)
	 * @return True if area found
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	bool GetAreaData(FName AreaID, FFCMapAreaData& OutAreaData) const;

	/**
	 * Lookup start point data by ID
	 * @param StartPointID - Unique start point identifier
	 * @param OutStartPointData - Start point data (output)
	 * @return True if start point found
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	bool GetStartPointData(FName StartPointID, FFCStartPointData& OutStartPointData) const;

	/**
	 * C++ helper: Get area data pointer (not exposed to Blueprint)
	 */
	const FFCMapAreaData* GetAreaDataPtr(FName AreaID) const;

	/**
	 * C++ helper: Get start point data pointer (not exposed to Blueprint)
	 */
	const FFCStartPointData* GetStartPointDataPtr(FName StartPointID) const;

	/**
	 * Get all start points for a given area
	 * @param AreaID - Area to query
	 * @return Array of start point data (empty if area not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	TArray<FFCStartPointData> GetStartPointsForArea(FName AreaID) const;

	/**
	 * Calculate total cost for a given start point
	 * @param StartPointID - Start point to calculate cost for
	 * @param OutMoneyCost - Gold cost (output)
	 * @param OutSupplyCost - Supply cost (output)
	 * @return True if start point found and costs retrieved
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	bool CalculateRouteCost(FName StartPointID, int32& OutMoneyCost, int32& OutSupplyCost) const;

	/**
	 * Get route waypoints for visual route drawing
	 * @param StartPointID - Start point to get route for
	 * @return Array of 2D waypoints (empty if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	TArray<FVector2D> GetRouteSplinePoints(FName StartPointID) const;

	/**
	 * Check if a sub-cell is explored
	 * @param AreaID - Area containing the sub-cell
	 * @param SubCellIndex - Index within 12×12 grid (0-143)
	 * @return True if explored, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	bool IsSubCellExplored(FName AreaID, int32 SubCellIndex) const;

	/**
	 * Set sub-cell exploration state
	 * @param AreaID - Area containing the sub-cell
	 * @param SubCellIndex - Index within 12×12 grid (0-143)
	 * @param bExplored - New exploration state
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	void SetSubCellExplored(FName AreaID, int32 SubCellIndex, bool bExplored);

	/**
	 * Update exploration based on convoy world position (called per frame from convoy Tick)
	 * @param WorldPosition - Current convoy position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	void UpdateExplorationAtWorldPosition(const FVector& WorldPosition);

	/**
	 * Get exploration completion percentage for an area
	 * @param AreaID - Area to query
	 * @return Percentage explored (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	float GetExploredPercentage(FName AreaID) const;

	/**
	 * Validate that current planning state is valid and affordable
	 * @return True if planning state is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "World Map")
	bool ValidatePlanningState() const;

protected:
	/** DataTable for map areas (DT_MapAreas) */
	UPROPERTY(EditDefaultsOnly, Category = "World Map|Config")
	UDataTable* AreaDataTable;

	/** DataTable for start points (DT_StartPoints) */
	UPROPERTY(EditDefaultsOnly, Category = "World Map|Config")
	UDataTable* StartPointDataTable;

	/** Office position on map (bottom center) - used for route calculations */
	UPROPERTY(EditDefaultsOnly, Category = "World Map|Config")
	FVector2D OfficeMapPosition;

private:
	/** Validate DataTables on initialization */
	void ValidateDataTables();
};
