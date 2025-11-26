// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "UFCWorldMapManager.h"
#include "UFCGameInstance.h"

DEFINE_LOG_CATEGORY(LogFCWorldMapManager);

void UFCWorldMapManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogFCWorldMapManager, Log, TEXT("UFCWorldMapManager::Initialize - Initializing World Map Manager"));

	ValidateDataTables();
}

void UFCWorldMapManager::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG(LogFCWorldMapManager, Log, TEXT("UFCWorldMapManager::Deinitialize - Shutting down World Map Manager"));
}

void UFCWorldMapManager::ValidateDataTables()
{
	if (!AreaDataTable)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("ValidateDataTables - AreaDataTable is not assigned! Configure in BP_FC_GameInstance."));
	}
	else
	{
		TArray<FName> RowNames = AreaDataTable->GetRowNames();
		UE_LOG(LogFCWorldMapManager, Log, TEXT("ValidateDataTables - AreaDataTable loaded with %d areas"), RowNames.Num());
	}

	if (!StartPointDataTable)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("ValidateDataTables - StartPointDataTable is not assigned! Configure in BP_FC_GameInstance."));
	}
	else
	{
		TArray<FName> RowNames = StartPointDataTable->GetRowNames();
		UE_LOG(LogFCWorldMapManager, Log, TEXT("ValidateDataTables - StartPointDataTable loaded with %d start points"), RowNames.Num());
	}
}

bool UFCWorldMapManager::GetAreaData(FName AreaID, FFCMapAreaData& OutAreaData) const
{
	const FFCMapAreaData* AreaDataPtr = GetAreaDataPtr(AreaID);
	if (AreaDataPtr)
	{
		OutAreaData = *AreaDataPtr;
		return true;
	}
	return false;
}

bool UFCWorldMapManager::GetStartPointData(FName StartPointID, FFCStartPointData& OutStartPointData) const
{
	const FFCStartPointData* StartPointDataPtr = GetStartPointDataPtr(StartPointID);
	if (StartPointDataPtr)
	{
		OutStartPointData = *StartPointDataPtr;
		return true;
	}
	return false;
}

const FFCMapAreaData* UFCWorldMapManager::GetAreaDataPtr(FName AreaID) const
{
	if (!AreaDataTable)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("GetAreaDataPtr - AreaDataTable is null"));
		return nullptr;
	}

	const FFCMapAreaData* AreaData = AreaDataTable->FindRow<FFCMapAreaData>(AreaID, TEXT("GetAreaDataPtr"));
	if (!AreaData)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("GetAreaDataPtr - Area '%s' not found in DataTable"), *AreaID.ToString());
	}

	return AreaData;
}

const FFCStartPointData* UFCWorldMapManager::GetStartPointDataPtr(FName StartPointID) const
{
	if (!StartPointDataTable)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("GetStartPointDataPtr - StartPointDataTable is null"));
		return nullptr;
	}

	const FFCStartPointData* StartPointData = StartPointDataTable->FindRow<FFCStartPointData>(StartPointID, TEXT("GetStartPointDataPtr"));
	if (!StartPointData)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("GetStartPointDataPtr - Start point '%s' not found in DataTable"), *StartPointID.ToString());
	}

	return StartPointData;
}

TArray<FFCStartPointData> UFCWorldMapManager::GetStartPointsForArea(FName AreaID) const
{
	TArray<FFCStartPointData> Result;

	FFCMapAreaData AreaData;
	if (!GetAreaData(AreaID, AreaData))
	{
		return Result;
	}

	// Lookup each start point by ID
	for (const FName& StartPointID : AreaData.StartPoints)
	{
		FFCStartPointData StartPointData;
		if (GetStartPointData(StartPointID, StartPointData))
		{
			Result.Add(StartPointData);
		}
	}

	return Result;
}

bool UFCWorldMapManager::CalculateRouteCost(FName StartPointID, int32& OutMoneyCost, int32& OutSupplyCost) const
{
	FFCStartPointData StartPointData;
	if (!GetStartPointData(StartPointID, StartPointData))
	{
		OutMoneyCost = 0;
		OutSupplyCost = 0;
		return false;
	}

	OutMoneyCost = StartPointData.MoneyCost;
	OutSupplyCost = StartPointData.SupplyCost;
	return true;
}

TArray<FVector2D> UFCWorldMapManager::GetRouteSplinePoints(FName StartPointID) const
{
	FFCStartPointData StartPointData;
	if (!GetStartPointData(StartPointID, StartPointData))
	{
		return TArray<FVector2D>();
	}

	return StartPointData.SplinePoints;
}

bool UFCWorldMapManager::IsSubCellExplored(FName AreaID, int32 SubCellIndex) const
{
	const FFCMapAreaData* AreaData = GetAreaDataPtr(AreaID);
	if (!AreaData || !AreaData->ExploredSubCells.IsValidIndex(SubCellIndex))
	{
		return false;
	}

	return AreaData->ExploredSubCells[SubCellIndex];
}

void UFCWorldMapManager::SetSubCellExplored(FName AreaID, int32 SubCellIndex, bool bExplored)
{
	if (!AreaDataTable)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("SetSubCellExplored - AreaDataTable is null"));
		return;
	}

	FFCMapAreaData* AreaData = AreaDataTable->FindRow<FFCMapAreaData>(AreaID, TEXT("SetSubCellExplored"));
	if (!AreaData)
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("SetSubCellExplored - Area '%s' not found"), *AreaID.ToString());
		return;
	}

	if (!AreaData->ExploredSubCells.IsValidIndex(SubCellIndex))
	{
		UE_LOG(LogFCWorldMapManager, Warning, TEXT("SetSubCellExplored - Invalid SubCellIndex %d (valid range: 0-143)"), SubCellIndex);
		return;
	}

	// Only update if state changed
	if (AreaData->ExploredSubCells[SubCellIndex] != bExplored)
	{
		AreaData->ExploredSubCells[SubCellIndex] = bExplored;
		
		// TODO: Mark save data dirty for persistence
		UE_LOG(LogFCWorldMapManager, Verbose, TEXT("SetSubCellExplored - Area '%s' SubCell %d set to %s"), 
			*AreaID.ToString(), SubCellIndex, bExplored ? TEXT("Explored") : TEXT("Unexplored"));
	}
}

void UFCWorldMapManager::UpdateExplorationAtWorldPosition(const FVector& WorldPosition)
{
	// TODO: Implement world position to area + sub-cell index conversion
	// This will be called every frame from convoy Tick() to gradually unveil fog of war
	
	// Algorithm:
	// 1. Determine which area contains this world position (based on area boundaries/level name)
	// 2. Convert world position to sub-cell index (0-143) within 12×12 grid
	// 3. Call SetSubCellExplored() if not already explored
	
	// For now, log verbose to avoid spam
	// UE_LOG(LogFCWorldMapManager, VeryVerbose, TEXT("UpdateExplorationAtWorldPosition - Position: %s"), *WorldPosition.ToString());
}

float UFCWorldMapManager::GetExploredPercentage(FName AreaID) const
{
	const FFCMapAreaData* AreaData = GetAreaDataPtr(AreaID);
	if (!AreaData || AreaData->ExploredSubCells.Num() == 0)
	{
		return 0.0f;
	}

	int32 ExploredCount = 0;
	for (bool bExplored : AreaData->ExploredSubCells)
	{
		if (bExplored)
		{
			++ExploredCount;
		}
	}

	return static_cast<float>(ExploredCount) / static_cast<float>(AreaData->ExploredSubCells.Num());
}

bool UFCWorldMapManager::ValidatePlanningState() const
{
	// TODO: Implement validation of current planning state from UFCGameInstance
	// This will check:
	// 1. Selected area exists in DataTable
	// 2. Selected start point exists and belongs to selected area
	// 3. Player has sufficient resources (Money, Supplies) for selected start point
	
	UE_LOG(LogFCWorldMapManager, Warning, TEXT("ValidatePlanningState - Not yet implemented"));
	return false;
}
