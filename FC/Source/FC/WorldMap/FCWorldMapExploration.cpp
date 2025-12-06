// Copyright Slomotion Games. All Rights Reserved.

#include "WorldMap/FCWorldMapExploration.h"

#include "Algo/Reverse.h"
#include "Containers/Queue.h"

#include "Logging/LogMacros.h"
#include "Expedition/FCExpeditionManager.h" // for LogFCWorldMap declaration

FFCWorldMapExploration::FFCWorldMapExploration()
{
	RevealMask.Init(0, GlobalCount);
	LandMask.Init(1, GlobalCount); // Default to land everywhere until a mask is provided.
}

bool FFCWorldMapExploration::IsValidGridId(int32 GridId)
{
	return GridId >= 0 && GridId < (GridSize * GridSize);
}

bool FFCWorldMapExploration::IsValidSubId(int32 SubId)
{
	return SubId >= 0 && SubId < (SubSize * SubSize);
}

bool FFCWorldMapExploration::IsValidGlobal(int32 X, int32 Y)
{
	return X >= 0 && X < GlobalSize && Y >= 0 && Y < GlobalSize;
}

bool FFCWorldMapExploration::IsValidGlobalId(int32 GlobalId)
{
	return GlobalId >= 0 && GlobalId < GlobalCount;
}

// Top-left origin for grid areas (row 0 = top).
void FFCWorldMapExploration::GridIdToXY(int32 GridId, int32& OutX, int32& OutY)
{
	OutX = GridId % GridSize;
	OutY = GridId / GridSize;
}

int32 FFCWorldMapExploration::XYToGridId(int32 X, int32 Y)
{
	return X + (Y * GridSize);
}

void FFCWorldMapExploration::SubIdToXY(int32 SubId, int32& OutSX, int32& OutSY)
{
	OutSX = SubId % SubSize;
	OutSY = SubId / SubSize;
}

int32 FFCWorldMapExploration::XYToSubId(int32 SX, int32 SY)
{
	return SX + (SY * SubSize);
}

int32 FFCWorldMapExploration::AreaSubToGlobalId(int32 GridId, int32 SubId)
{
	int32 AreaX, AreaY;
	GridIdToXY(GridId, AreaX, AreaY);

	int32 SubX, SubY;
	SubIdToXY(SubId, SubX, SubY);

	const int32 GlobalX = AreaX * SubSize + SubX;
	const int32 GlobalY = AreaY * SubSize + SubY;
	return XYToGlobalId(GlobalX, GlobalY);
}

void FFCWorldMapExploration::GlobalIdToAreaSub(int32 GlobalId, int32& OutGridId, int32& OutSubId)
{
	int32 GlobalX, GlobalY;
	GlobalIdToXY(GlobalId, GlobalX, GlobalY);

	const int32 AreaX = GlobalX / SubSize;
	const int32 AreaY = GlobalY / SubSize;
	const int32 SubX = GlobalX % SubSize;
	const int32 SubY = GlobalY % SubSize;

	OutGridId = XYToGridId(AreaX, AreaY);
	OutSubId = XYToSubId(SubX, SubY);
}

void FFCWorldMapExploration::GlobalIdToXY(int32 GlobalId, int32& OutGX, int32& OutGY)
{
	OutGX = GlobalId % GlobalSize;
	OutGY = GlobalId / GlobalSize;
}

int32 FFCWorldMapExploration::XYToGlobalId(int32 GX, int32 GY)
{
	return GX + (GY * GlobalSize);
}

bool FFCWorldMapExploration::IsRevealed_Global(int32 GlobalId) const
{
	if (!IsValidGlobalId(GlobalId))
	{
		return false;
	}

	return RevealMask[GlobalId] >= 128;
}

bool FFCWorldMapExploration::SetRevealed_Global(int32 GlobalId, bool bRevealed)
{
	if (!IsValidGlobalId(GlobalId))
	{
		return false;
	}

	const uint8 NewValue = bRevealed ? 255 : 0;
	if (RevealMask[GlobalId] == NewValue)
	{
		return false;
	}

	RevealMask[GlobalId] = NewValue;
	return true;
}

void FFCWorldMapExploration::SetLandMask(const TArray<uint8>& InLandMask)
{
	if (InLandMask.Num() == GlobalCount)
	{
		LandMask = InLandMask;
	}
}

bool FFCWorldMapExploration::IsLand_Global(int32 GlobalId) const
{
	if (!IsValidGlobalId(GlobalId))
	{
		return false;
	}

	return LandMask[GlobalId] != 0;
}

void FFCWorldMapExploration::ApplyDefaultRevealedAreas_NewGame(const TArray<int32>& DefaultGridIds)
{
	UE_LOG(LogFCWorldMap, Log, TEXT("FFCWorldMapExploration::ApplyDefaultRevealedAreas_NewGame: Applying default revealed areas (Count=%d)"), DefaultGridIds.Num());

	auto RevealAreaFully = [this](int32 GridId)
	{
		int32 AreaX, AreaY;
		GridIdToXY(GridId, AreaX, AreaY);

		for (int32 SY = 0; SY < SubSize; ++SY)
		{
			for (int32 SX = 0; SX < SubSize; ++SX)
			{
				const int32 GlobalX = AreaX * SubSize + SX;
				const int32 GlobalY = AreaY * SubSize + SY;
				const int32 GlobalId = XYToGlobalId(GlobalX, GlobalY);
				SetRevealed_Global(GlobalId, true);
			}
		}
	};

	for (int32 GridId : DefaultGridIds)
	{
		if (!IsValidGridId(GridId))
		{
			UE_LOG(LogFCWorldMap, Warning, TEXT("ApplyDefaultRevealedAreas_NewGame: Ignoring invalid GridId %d"), GridId);
			continue;
		}

		RevealAreaFully(GridId);
	}

	UE_LOG(LogFCWorldMap, Log, TEXT("FFCWorldMapExploration::ApplyDefaultRevealedAreas_NewGame: Completed default reveal"));
}

bool FFCWorldMapExploration::FindShortestPath_BFS(int32 StartGlobalId, int32 GoalGlobalId, TArray<int32>& OutPath) const
{
	OutPath.Reset();

	UE_LOG(LogFCWorldMap, Log, TEXT("FFCWorldMapExploration::FindShortestPath_BFS: Start=%d, Goal=%d"), StartGlobalId, GoalGlobalId);

	if (!IsValidGlobalId(StartGlobalId) || !IsValidGlobalId(GoalGlobalId))
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("FindShortestPath_BFS: Invalid Start(%d) or Goal(%d)"), StartGlobalId, GoalGlobalId);
		return false;
	}

	if (!IsTraversable_Global(StartGlobalId, GoalGlobalId))
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("FindShortestPath_BFS: Start cell %d not traversable"), StartGlobalId);
		return false;
	}

	TArray<int32> CameFrom;
	CameFrom.Init(-1, GlobalCount);

	TQueue<int32> Frontier;
	Frontier.Enqueue(StartGlobalId);
	CameFrom[StartGlobalId] = StartGlobalId;

	// Debug: Log 8-neighborhood around goal cell with land/reveal info.
	{
		int32 GX, GY;
		GlobalIdToXY(GoalGlobalId, GX, GY);
		const int32 Offsets[8][2] = {
			{ -1, -1 }, { 0, -1 }, { 1, -1 },
			{ -1,  0 },             { 1,  0 },
			{ -1,  1 }, { 0,  1 }, { 1,  1 }
		};

		for (int32 I = 0; I < 8; ++I)
		{
			const int32 NX = GX + Offsets[I][0];
			const int32 NY = GY + Offsets[I][1];
			if (!IsValidGlobal(NX, NY))
			{
				continue;
			}

			const int32 NeighborId = XYToGlobalId(NX, NY);
			const bool bIsLand = IsLand_Global(NeighborId);
			const bool bIsRev = IsRevealed_Global(NeighborId);
			UE_LOG(LogFCWorldMap, Log, TEXT("GoalNeighborhood: GlobalId=%d IsLand=%s IsRevealed=%s"),
				NeighborId,
				bIsLand ? TEXT("true") : TEXT("false"),
				bIsRev ? TEXT("true") : TEXT("false"));
		}
	}

	bool bFound = false;

	while (!Frontier.IsEmpty())
	{
		int32 Current;
		Frontier.Dequeue(Current);

		if (Current == GoalGlobalId)
		{
			bFound = true;
			UE_LOG(LogFCWorldMap, Verbose, TEXT("FindShortestPath_BFS: Reached goal %d"), GoalGlobalId);
			break;
		}

		int32 X, Y;
		GlobalIdToXY(Current, X, Y);

		const int32 NeighborX[4] = { X + 1, X - 1, X,     X };
		const int32 NeighborY[4] = { Y,     Y,     Y + 1, Y - 1 };

		for (int32 Index = 0; Index < 4; ++Index)
		{
			const int32 NX = NeighborX[Index];
			const int32 NY = NeighborY[Index];
			if (!IsValidGlobal(NX, NY))
			{
				UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("FindShortestPath_BFS: Neighbor out of bounds NX=%d NY=%d"), NX, NY);
				continue;
			}

			const int32 NeighborId = XYToGlobalId(NX, NY);
			if (CameFrom[NeighborId] != -1)
			{
				// Already visited
				continue;
			}

			if (!IsTraversable_Global(NeighborId, GoalGlobalId))
			{
				// Not traversable - skip
				continue;
			}

			CameFrom[NeighborId] = Current;
			Frontier.Enqueue(NeighborId);
		}
	}

	if (!bFound)
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("FindShortestPath_BFS: Failed to find path from %d to %d"), StartGlobalId, GoalGlobalId);
		return false;
	}

	int32 Node = GoalGlobalId;
	while (Node != StartGlobalId)
	{
		OutPath.Add(Node);
		Node = CameFrom[Node];
		if (Node == -1)
		{
			UE_LOG(LogFCWorldMap, Warning, TEXT("FindShortestPath_BFS: Broken predecessor chain while reconstructing path"));
			return false;
		}
	}
	OutPath.Add(StartGlobalId);

	Algo::Reverse(OutPath);

	UE_LOG(LogFCWorldMap, Log, TEXT("FindShortestPath_BFS: Path found with %d nodes"), OutPath.Num());
	return true;
}

bool FFCWorldMapExploration::IsTraversable_Global(int32 GlobalId, int32 GoalGlobalId) const
{
	if (!IsValidGlobalId(GlobalId))
	{
		UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("IsTraversable_Global: GlobalId %d invalid"), GlobalId);
		return false;
	}

	if (GlobalId == GoalGlobalId)
	{
		const bool bGoalIsLand = IsLand_Global(GlobalId);
		UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("IsTraversable_Global: Checking goal cell %d (IsLand=%s)"),
			GlobalId, bGoalIsLand ? TEXT("true") : TEXT("false"));
		return bGoalIsLand;
	}

	if (IsWater_Global(GlobalId))
	{
		UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("IsTraversable_Global: GlobalId %d is water -> traversable"), GlobalId);
		return true;
	}

	const bool bRevealed = IsRevealed_Global(GlobalId);
	UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("IsTraversable_Global: GlobalId %d land, revealed=%s"),
		GlobalId, bRevealed ? TEXT("true") : TEXT("false"));
	return bRevealed;
}
