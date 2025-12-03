// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * FFCWorldMapExploration is a pure helper that manages global map masks.
 * - 16x16 areas, each with 16x16 subcells -> 256x256 global grid.
 * - Stores reveal (fog) and terrain (land/water) data as byte arrays.
 * - Provides conversion helpers between grid/sub/global indices.
 * - Supplies a BFS shortest-path implementation for milestone preview routes.
 */
class FFCWorldMapExploration
{
public:
	static constexpr int32 GridSize = 16;     // 16x16 areas
	static constexpr int32 SubSize = 16;      // 16x16 subcells per area
	static constexpr int32 GlobalSize = GridSize * SubSize; // 256
	static constexpr int32 GlobalCount = GlobalSize * GlobalSize; // 65536

	FFCWorldMapExploration();

	// --- Reveal mask --------------------------------------------------------

	const TArray<uint8>& GetRevealMask() const { return RevealMask; }
	TArray<uint8>& GetRevealMaskMutable() { return RevealMask; }

	bool IsRevealed_Global(int32 GlobalId) const;
	bool SetRevealed_Global(int32 GlobalId, bool bRevealed);

	// --- Land mask ----------------------------------------------------------

	const TArray<uint8>& GetLandMask() const { return LandMask; }
	void SetLandMask(const TArray<uint8>& InLandMask);

	bool IsLand_Global(int32 GlobalId) const;
	bool IsWater_Global(int32 GlobalId) const { return !IsLand_Global(GlobalId); }

	// --- Coordinate helpers -------------------------------------------------

	static bool IsValidGridId(int32 GridId);
	static bool IsValidSubId(int32 SubId);
	static bool IsValidGlobal(int32 X, int32 Y);
	static bool IsValidGlobalId(int32 GlobalId);

	static void GridIdToXY(int32 GridId, int32& OutX, int32& OutY);
	static int32 XYToGridId(int32 X, int32 Y);

	static void SubIdToXY(int32 SubId, int32& OutSX, int32& OutSY);
	static int32 XYToSubId(int32 SX, int32 SY);

	static int32 AreaSubToGlobalId(int32 GridId, int32 SubId);
	static void GlobalIdToAreaSub(int32 GlobalId, int32& OutGridId, int32& OutSubId);

	static void GlobalIdToXY(int32 GlobalId, int32& OutGX, int32& OutGY);
	static int32 XYToGlobalId(int32 GX, int32 GY);

	// --- Initialization -----------------------------------------------------

	/**
	 * Apply default revealed areas for a new game.
	 * If DefaultGridIds is empty, no areas are revealed.
	 */
	void ApplyDefaultRevealedAreas_NewGame(const TArray<int32>& DefaultGridIds);

	// --- Pathfinding --------------------------------------------------------

	/**
	 * Find the shortest path between StartGlobalId and GoalGlobalId using BFS.
	 * Traversal rules:
	 * - Water cells are always traversable (revealed or not).
	 * - Land cells require being revealed, except the goal cell may be unrevealed land.
	 */
	bool FindShortestPath_BFS(int32 StartGlobalId, int32 GoalGlobalId, TArray<int32>& OutPath) const;

private:
	bool IsTraversable_Global(int32 GlobalId, int32 GoalGlobalId) const;

	TArray<uint8> RevealMask;
	TArray<uint8> LandMask;
};
