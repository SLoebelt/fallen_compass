// Copyright Slomotion Games. All Rights Reserved.

#include "Expedition/FCExpeditionManager.h"

#include "Core/UFCGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "WorldMap/FCWorldMapSaveGame.h"

DEFINE_LOG_CATEGORY(LogFCExpedition);
DEFINE_LOG_CATEGORY(LogFCWorldMap);

void UFCExpeditionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogFCExpedition, Log, TEXT("UFCExpeditionManager::Initialize - subsystem created for world %s"),
		*GetWorld()->GetName());
	CurrentExpedition = nullptr;

	// Copy configuration from GameInstance
	if (UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance()))
	{
		LandMaskTexture = GI->WorldMapLandMaskTexture;
		OverworldWorldMin = GI->OverworldWorldMin;
		OverworldWorldMax = GI->OverworldWorldMax;
		OfficeGridId = GI->OfficeGridId;
		OfficeSubId = GI->OfficeSubId;
		AvailableStartGridId = GI->AvailableStartGridId;
		AvailableStartSubId = GI->AvailableStartSubId;
		PreviewTargetGridId = GI->PreviewTargetGridId;
		PreviewTargetSubId = GI->PreviewTargetSubId;
		DefaultRevealedWorldMapGridIds = GI->DefaultRevealedWorldMapGridIds;

		UE_LOG(LogFCExpedition, Log, TEXT("Expedition Manager configured from GameInstance"));
	}
	else
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("Failed to get UFCGameInstance - using default subsystem config"));
	}

	WorldMap_InitOrLoad();

	FogTexture = CreateMaskTexture256();
	RouteTexture = CreateMaskTexture256();
	RouteMask.Init(0, FFCWorldMapExploration::GlobalCount);

	WorldMap_SyncFogTexture_Full();
	UpdateMaskTextureFull(RouteTexture, RouteMask);
}

void UFCExpeditionManager::Deinitialize()
{
	if (bExplorationDirty)
	{
		WorldMap_SaveNow();
	}

	if (CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("Deinitialize called with active expedition: %s"),
			*CurrentExpedition->ExpeditionName);
	}

	Super::Deinitialize();
}

UFCExpeditionData* UFCExpeditionManager::StartNewExpedition(const FString& ExpeditionName, int32 AllocatedSupplies)
{
	if (CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("StartNewExpedition called while expedition already active: %s"),
			*CurrentExpedition->ExpeditionName);
		EndExpedition(false);
	}

	CurrentExpedition = NewObject<UFCExpeditionData>(this);
	CurrentExpedition->ExpeditionName = ExpeditionName;
	CurrentExpedition->StartingSupplies = AllocatedSupplies;
	CurrentExpedition->StartDate = FString::Printf(TEXT("Day %d"), 1);
	CurrentExpedition->TargetRegion = TEXT("Unknown Region");
	CurrentExpedition->ExpeditionStatus = EFCExpeditionStatus::InProgress;

	UE_LOG(LogFCExpedition, Log, TEXT("Started expedition: %s (Supplies: %d)"),
		*ExpeditionName, AllocatedSupplies);

	OnExpeditionStateChanged.Broadcast(CurrentExpedition);
	return CurrentExpedition;
}

void UFCExpeditionManager::EndExpedition(bool bSuccess)
{
	if (!CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("EndExpedition called with no active expedition"));
		return;
	}

	CurrentExpedition->ExpeditionStatus = bSuccess ? EFCExpeditionStatus::Completed : EFCExpeditionStatus::Failed;

	UE_LOG(LogFCExpedition, Log, TEXT("Ended expedition: %s (Success: %s)"),
		*CurrentExpedition->ExpeditionName,
		bSuccess ? TEXT("true") : TEXT("false"));

	OnExpeditionStateChanged.Broadcast(CurrentExpedition);
	CurrentExpedition = nullptr;
}

bool UFCExpeditionManager::IsExpeditionActive() const
{
	return CurrentExpedition != nullptr &&
		CurrentExpedition->ExpeditionStatus == EFCExpeditionStatus::InProgress;
}

// -----------------------------------------------------------------------------
// World map runtime helpers
// -----------------------------------------------------------------------------

void UFCExpeditionManager::WorldMap_InitOrLoad()
{
	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_InitOrLoad: Begin (SaveSlot=%s, UserIndex=%d)"),
		TEXT("FC_WorldMapExploration"), WorldMapSaveUserIndex);
	// NOTE: Save/load temporarily disabled while debugging default reveal behavior.
	//UFCWorldMapSaveGame* Save = nullptr;
	//if (UGameplayStatics::DoesSaveGameExist(WorldMapSaveSlot, WorldMapSaveUserIndex))
	//{
	//	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_InitOrLoad: Save game exists for slot %s"), WorldMapSaveSlot);
	//	Save = Cast<UFCWorldMapSaveGame>(UGameplayStatics::LoadGameFromSlot(WorldMapSaveSlot, WorldMapSaveUserIndex));
	//}
	//if (Save && Save->RevealMask.Num() == FFCWorldMapExploration::GlobalCount)
	//{
	//	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_InitOrLoad: Loaded existing exploration state (RevealMask=%d, LandMask=%d)"),
	//		Save->RevealMask.Num(), Save->LandMask.Num());
	//	WorldMap.GetRevealMaskMutable() = Save->RevealMask;
	//	if (Save->LandMask.Num() == FFCWorldMapExploration::GlobalCount)
	//	{
	//		WorldMap.SetLandMask(Save->LandMask);
	//	}
	//	else
	//	{
	//		WorldMap_LoadLandMaskIfAvailable();
	//	}
	//	UE_LOG(LogFCWorldMap, Log, TEXT("Loaded world map exploration from save game."));
	//	return;
	//}

	WorldMap.ApplyDefaultRevealedAreas_NewGame(DefaultRevealedWorldMapGridIds);
	WorldMap_LoadLandMaskIfAvailable();

	//bExplorationDirty = true;
	//WorldMap_SaveNow();

	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_InitOrLoad: Initialized new exploration state (RevealMask non-zero regions applied)."));
}

void UFCExpeditionManager::WorldMap_LoadLandMaskIfAvailable()
{
	TArray<uint8> NewLandMask;
	NewLandMask.Init(1, FFCWorldMapExploration::GlobalCount);

	UTexture2D* LandMaskAsset = LandMaskTexture.LoadSynchronous();
	if (!LandMaskAsset || !LandMaskAsset->GetPlatformData() || LandMaskAsset->GetPlatformData()->Mips.Num() == 0)
	{
		WorldMap.SetLandMask(NewLandMask);
		return;
	}

	const int32 ExpectedW = FFCWorldMapExploration::GlobalSize;
	const int32 ExpectedH = FFCWorldMapExploration::GlobalSize;
	const FTexture2DMipMap& Mip = LandMaskAsset->GetPlatformData()->Mips[0];

	if (Mip.SizeX != ExpectedW || Mip.SizeY != ExpectedH)
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("Land mask texture size mismatch (%dx%d, expected 256x256)."),
			Mip.SizeX, Mip.SizeY);
		WorldMap.SetLandMask(NewLandMask);
		return;
	}

	const void* DataPtr = Mip.BulkData.LockReadOnly();
	if (!DataPtr)
	{
		Mip.BulkData.Unlock();
		WorldMap.SetLandMask(NewLandMask);
		return;
	}

	const int32 ByteCount = Mip.BulkData.GetBulkDataSize();
	const uint8* Bytes = static_cast<const uint8*>(DataPtr);

	if (ByteCount == (ExpectedW * ExpectedH))
	{
		for (int32 Index = 0; Index < FFCWorldMapExploration::GlobalCount; ++Index)
		{
			// In the land mask texture, black (low value) represents land,
			// white (high value) represents water. Interpret values <= 128
			// as land, and > 128 as water.
			NewLandMask[Index] = Bytes[Index] <= 128 ? 1 : 0;
		}
	}
	else if (ByteCount == (ExpectedW * ExpectedH * 4))
	{
		for (int32 Index = 0; Index < FFCWorldMapExploration::GlobalCount; ++Index)
		{
			const uint8 R = Bytes[Index * 4];
			// Same convention for RGBA textures: dark pixels are land,
			// bright pixels are water.
			NewLandMask[Index] = R <= 128 ? 1 : 0;
		}
	}
	else
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("Land mask texture bulk data unexpected size: %d bytes"), ByteCount);
	}

	Mip.BulkData.Unlock();
	WorldMap.SetLandMask(NewLandMask);
}

void UFCExpeditionManager::WorldMap_SyncFogTexture_Full()
{
	UE_LOG(LogFCWorldMap, Verbose, TEXT("WorldMap_SyncFogTexture_Full: Syncing fog texture from RevealMask (Num=%d)"),
		WorldMap.GetRevealMask().Num());

	if (FogTexture)
	{
		UpdateMaskTextureFull(FogTexture, WorldMap.GetRevealMask());
	}
}

void UFCExpeditionManager::WorldMap_UpdateFogPixel(int32 GlobalId)
{
	if (FogTexture)
	{
		UpdateMaskTexturePixel(FogTexture, WorldMap.GetRevealMask(), GlobalId);
	}
}

void UFCExpeditionManager::WorldMap_StartAutosaveDebounced()
{
	bExplorationDirty = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutosaveTimer);
		World->GetTimerManager().SetTimer(AutosaveTimer, this, &UFCExpeditionManager::WorldMap_SaveNow, 0.75f, false);
	}
}

void UFCExpeditionManager::WorldMap_SaveNow()
{
	UFCWorldMapSaveGame* Save = Cast<UFCWorldMapSaveGame>(UGameplayStatics::CreateSaveGameObject(UFCWorldMapSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}

	Save->RevealMask = WorldMap.GetRevealMask();
	Save->LandMask = WorldMap.GetLandMask();

	UGameplayStatics::SaveGameToSlot(Save, WorldMapSaveSlot, WorldMapSaveUserIndex);
	bExplorationDirty = false;
	UE_LOG(LogFCWorldMap, VeryVerbose, TEXT("World map exploration saved."));
}

// -----------------------------------------------------------------------------
// Route preview handling
// -----------------------------------------------------------------------------

bool UFCExpeditionManager::WorldMap_SelectGridArea(int32 GridId)
{
	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_SelectGridArea: Requested GridId=%d (AvailableStartGridId=%d)"),
		GridId, AvailableStartGridId);

	if (GridId != AvailableStartGridId)
	{
		UE_LOG(LogFCWorldMap, Verbose, TEXT("WorldMap_SelectGridArea rejected: GridId %d"), GridId);
		return false;
	}

	if (!CurrentExpedition)
	{
		CurrentExpedition = NewObject<UFCExpeditionData>(this);
		CurrentExpedition->ExpeditionStatus = EFCExpeditionStatus::Planning;

		UE_LOG(LogFCExpedition, Log, TEXT("WorldMap_SelectGridArea: Created new Planning expedition object (%p)"),
			CurrentExpedition.Get());
	}

	CurrentExpedition->SelectedGridId = GridId;
	CurrentExpedition->SelectedStartGridId = AvailableStartGridId;
	CurrentExpedition->SelectedStartSubId = AvailableStartSubId;
	CurrentExpedition->PreviewTargetGridId = PreviewTargetGridId;
	CurrentExpedition->PreviewTargetSubId = PreviewTargetSubId;

	UE_LOG(LogFCExpedition, Log, TEXT("WorldMap_SelectGridArea: Expedition planning selection -> GridId=%d, StartGrid=%d/%d, TargetGrid=%d/%d"),
		GridId,
		AvailableStartGridId, AvailableStartSubId,
		PreviewTargetGridId, PreviewTargetSubId);

	return WorldMap_BuildPreviewRoute();
}

bool UFCExpeditionManager::WorldMap_BuildPreviewRoute()
{
	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_BuildPreviewRoute: Begin (CurrentExpedition=%p)"), CurrentExpedition.Get());

	if (!CurrentExpedition)
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("WorldMap_BuildPreviewRoute: No CurrentExpedition - aborting"));
		return false;
	}

	const int32 StartGlobal = FFCWorldMapExploration::AreaSubToGlobalId(OfficeGridId, OfficeSubId);
	const int32 GoalGlobal = FFCWorldMapExploration::AreaSubToGlobalId(PreviewTargetGridId, PreviewTargetSubId);

	TArray<int32> Path;
	if (!WorldMap.FindShortestPath_BFS(StartGlobal, GoalGlobal, Path))
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("Failed to build preview route from office to preview target."));
		CurrentExpedition->PlannedRouteGlobalIds.Reset();
		CurrentExpedition->PlannedMoneyCost = 0;
		CurrentExpedition->PlannedRiskCost = 0;
		return false;
	}

	CurrentExpedition->PlannedRouteGlobalIds = Path;
	int32 Money = 0;
	int32 Risk = 0;
	ComputeCostsForPath(Path, Money, Risk);
	CurrentExpedition->PlannedMoneyCost = Money;
	CurrentExpedition->PlannedRiskCost = Risk;

	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_BuildPreviewRoute: Success (PathLen=%d, Money=%d, Risk=%d)"),
		Path.Num(), Money, Risk);

	return true;
}

void UFCExpeditionManager::WorldMap_BeginRoutePreviewPaint(float StepSeconds)
{
	if (!CurrentExpedition || CurrentExpedition->PlannedRouteGlobalIds.Num() == 0)
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("WorldMap_BeginRoutePreviewPaint: No expedition or empty route (Expedition=%p, RouteLen=%d)"),
			CurrentExpedition.Get(),
			CurrentExpedition ? CurrentExpedition->PlannedRouteGlobalIds.Num() : 0);
		return;
	}

	WorldMap_ClearRoutePreview();
	RoutePaintIndex = 0;

	UE_LOG(LogFCWorldMap, Log, TEXT("WorldMap_BeginRoutePreviewPaint: Starting route paint (RouteLen=%d, StepSeconds=%.3f)"),
		CurrentExpedition->PlannedRouteGlobalIds.Num(), StepSeconds);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoutePaintTimer, this, &UFCExpeditionManager::RoutePaint_Tick, StepSeconds, true);
	}
}

void UFCExpeditionManager::WorldMap_ClearRoutePreview()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoutePaintTimer);
	}

	RoutePaintIndex = 0;
	RouteMask.Init(0, FFCWorldMapExploration::GlobalCount);

	// Debug: Always mark GlobalId 4240 in the route mask so the
	// goal/target selection is visible regardless of whether a
	// valid route was found.
	if (FFCWorldMapExploration::IsValidGlobalId(4240))
	{
		RouteMask[4240] = 1;
	}

	UpdateMaskTextureFull(RouteTexture, RouteMask);
}

void UFCExpeditionManager::RoutePaint_Tick()
{
	if (!CurrentExpedition)
	{
		UE_LOG(LogFCWorldMap, Warning, TEXT("RoutePaint_Tick: No CurrentExpedition - clearing preview"));
		WorldMap_ClearRoutePreview();
		return;
	}

	const TArray<int32>& Route = CurrentExpedition->PlannedRouteGlobalIds;
	if (RoutePaintIndex >= Route.Num())
	{
		UE_LOG(LogFCWorldMap, Log, TEXT("RoutePaint_Tick: Finished painting route (Length=%d)"), Route.Num());
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RoutePaintTimer);
		}
		return;
	}

	const int32 GlobalId = Route[RoutePaintIndex];
	if (FFCWorldMapExploration::IsValidGlobalId(GlobalId))
	{
		RouteMask[GlobalId] = 255;
		UpdateMaskTexturePixel(RouteTexture, RouteMask, GlobalId);
	}

	++RoutePaintIndex;
}

// -----------------------------------------------------------------------------
// Exploration tracking
// -----------------------------------------------------------------------------

void UFCExpeditionManager::WorldMap_RecordVisitedWorldLocation(const FVector& WorldLocation)
{
	const float MinX = OverworldWorldMin.X;
	const float MinY = OverworldWorldMin.Y;
	const float MaxX = OverworldWorldMax.X;
	const float MaxY = OverworldWorldMax.Y;

	const float DenX = FMath::Max(1.f, MaxX - MinX);
	const float DenY = FMath::Max(1.f, MaxY - MinY);

	const float U = FMath::Clamp((WorldLocation.X - MinX) / DenX, 0.f, 0.99999f);
	const float V = FMath::Clamp((WorldLocation.Y - MinY) / DenY, 0.f, 0.99999f);

	const int32 GX = FMath::FloorToInt(U * FFCWorldMapExploration::GlobalSize);
	const int32 GY = FMath::FloorToInt(V * FFCWorldMapExploration::GlobalSize);
	const int32 GlobalId = FFCWorldMapExploration::XYToGlobalId(GX, GY);

	const bool bChanged = WorldMap.SetRevealed_Global(GlobalId, true);
	if (bChanged)
	{
		WorldMap_UpdateFogPixel(GlobalId);
		WorldMap_StartAutosaveDebounced();
		OnWorldMapChanged.Broadcast();
	}
}

// -----------------------------------------------------------------------------
// Cost helpers
// -----------------------------------------------------------------------------

void UFCExpeditionManager::ComputeCostsForPath(const TArray<int32>& Path, int32& OutMoney, int32& OutRisk) const
{
	OutMoney = 0;
	OutRisk = 0;

	for (int32 Index = 1; Index < Path.Num(); ++Index)
	{
		const int32 GlobalId = Path[Index];
		const bool bIsWater = WorldMap.IsWater_Global(GlobalId);
		const bool bWasRevealed = WorldMap.IsRevealed_Global(GlobalId);

		OutMoney += bIsWater ? 3 : 1;

		if (bIsWater)
		{
			OutRisk += bWasRevealed ? 2 : 3;
		}
		else
		{
			OutRisk += 1;
		}
	}
}

// -----------------------------------------------------------------------------
// Texture utilities
// -----------------------------------------------------------------------------

UTexture2D* UFCExpeditionManager::CreateMaskTexture256()
{
	UTexture2D* Texture = UTexture2D::CreateTransient(FFCWorldMapExploration::GlobalSize, FFCWorldMapExploration::GlobalSize, PF_G8);
	if (!Texture)
	{
		return nullptr;
	}

	Texture->SRGB = false;
	Texture->CompressionSettings = TC_Grayscale;
	Texture->Filter = TF_Nearest;
	Texture->AddressX = TA_Clamp;
	Texture->AddressY = TA_Clamp;
	Texture->UpdateResource();
	return Texture;
}

void UFCExpeditionManager::UpdateMaskTextureFull(UTexture2D* Texture, const TArray<uint8>& Data256)
{
	if (!Texture || Data256.Num() != FFCWorldMapExploration::GlobalCount)
	{
		return;
	}

	// CRITICAL: UpdateTextureRegions is async! Must allocate region + data on heap
	// FUpdateTextureRegion2D(DestX, DestY, SrcX, SrcY, Width, Height)
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(
		0, 0,                                    // Dest: top-left of texture
		0, 0,                                    // Src: top-left of source buffer
		FFCWorldMapExploration::GlobalSize,      // Width
		FFCWorldMapExploration::GlobalSize       // Height
	);

	// Allocate full 256x256 buffer on heap and copy data
	const int32 DataSize = FFCWorldMapExploration::GlobalCount;
	uint8* Src = new uint8[DataSize];
	FMemory::Memcpy(Src, Data256.GetData(), DataSize * sizeof(uint8));

	// Pitch = number of bytes per row in source data
	const uint32 SrcPitch = FFCWorldMapExploration::GlobalSize * sizeof(uint8);
	const uint32 SrcBpp = sizeof(uint8);

	Texture->UpdateTextureRegions(
		0,                    // Mip index
		1,                    // Num regions
		Region,               // Region pointer (heap allocated)
		SrcPitch,             // Src pitch (bytes per row)
		SrcBpp,               // Src bytes per pixel
		Src,                  // Source data pointer (heap allocated)
		[](uint8* InSrcData, const FUpdateTextureRegion2D* InRegions)
		{
			// Cleanup callback executed by render thread after update completes
			delete[] InSrcData;
			delete InRegions;
		}
	);
}

void UFCExpeditionManager::UpdateMaskTexturePixel(UTexture2D* Texture, const TArray<uint8>& Data256, int32 GlobalId)
{
	if (!Texture || Data256.Num() != FFCWorldMapExploration::GlobalCount)
	{
		return;
	}

	if (!FFCWorldMapExploration::IsValidGlobalId(GlobalId))
	{
		return;
	}

	int32 GX, GY;
	FFCWorldMapExploration::GlobalIdToXY(GlobalId, GX, GY);

	// CRITICAL: UpdateTextureRegions is async! Must allocate region + data on heap
	// so they remain valid when the render thread executes the update.
	// FUpdateTextureRegion2D(DestX, DestY, SrcX, SrcY, Width, Height)
	// SrcX/SrcY must be 0,0 because source buffer is only 1 byte
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(GX, GY, 0, 0, 1, 1);

	// Allocate 1 byte on heap and copy pixel value
	uint8* Src = new uint8[1];
	Src[0] = Data256[GlobalId];

	Texture->UpdateTextureRegions(
		0,                    // Mip index
		1,                    // Num regions
		Region,               // Region pointer (heap allocated)
		1,                    // Src pitch (1 byte per row for 1x1 region)
		1,                    // Src bytes per pixel
		Src,                  // Source data pointer (heap allocated)
		[](uint8* InSrcData, const FUpdateTextureRegion2D* InRegions)
		{
			// Cleanup callback executed by render thread after update completes
			delete[] InSrcData;
			delete InRegions;
		}
	);
}
