// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WorldMap/FCWorldMapExploration.h"
#include "Expedition/FCExpeditionData.h"

class UTexture2D;

DECLARE_LOG_CATEGORY_EXTERN(LogFCExpedition, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFCWorldMap, Log, All);

#include "FCExpeditionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpeditionStateChanged, UFCExpeditionData*, ExpeditionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldMapChanged);

/**
 * UFCExpeditionManager
 *
 * Game Instance subsystem that orchestrates expedition lifecycle and
 * milestone world-map planning features.
 */
UCLASS()
class FC_API UFCExpeditionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---------------------------------------------------------------------
	// Expedition lifecycle
	// ---------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	UFCExpeditionData* StartNewExpedition(const FString& ExpeditionName, int32 AllocatedSupplies);

	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	UFCExpeditionData* GetCurrentExpedition() const { return CurrentExpedition; }

	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	void EndExpedition(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	bool IsExpeditionActive() const;

	UPROPERTY(BlueprintAssignable, Category = "FC|Expedition")
	FOnExpeditionStateChanged OnExpeditionStateChanged;

	// ---------------------------------------------------------------------
	// World map API (Blueprint-facing hooks for UI and gameplay)
	// ---------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "FC|WorldMap")
	UTexture2D* WorldMap_GetFogTexture() const { return FogTexture; }

	UFUNCTION(BlueprintPure, Category = "FC|WorldMap")
	UTexture2D* WorldMap_GetRouteTexture() const { return RouteTexture; }

	UFUNCTION(BlueprintCallable, Category = "FC|WorldMap|Planning")
	bool WorldMap_SelectGridArea(int32 GridId);

	UFUNCTION(BlueprintCallable, Category = "FC|WorldMap|Planning")
	bool WorldMap_BuildPreviewRoute();

	UFUNCTION(BlueprintCallable, Category = "FC|WorldMap|Planning")
	void WorldMap_BeginRoutePreviewPaint(float StepSeconds = 0.03f);

	UFUNCTION(BlueprintCallable, Category = "FC|WorldMap|Planning")
	void WorldMap_ClearRoutePreview();

	UFUNCTION(BlueprintCallable, Category = "FC|WorldMap|Overworld")
	void WorldMap_RecordVisitedWorldLocation(const FVector& WorldLocation);

	UPROPERTY(BlueprintAssignable, Category = "FC|WorldMap")
	FOnWorldMapChanged OnWorldMapChanged;

	// ---------------------------------------------------------------------
	// Config (initialized from UFCGameInstance in Initialize())
	// ---------------------------------------------------------------------

	/** Land mask texture (configured via GameInstance) */
	TSoftObjectPtr<UTexture2D> LandMaskTexture;

	/** Overworld world bounds (configured via GameInstance) */
	FVector2D OverworldWorldMin = FVector2D(-50000.f, -50000.f);
	FVector2D OverworldWorldMax = FVector2D(50000.f, 50000.f);

	/** World map grid configuration (configured via GameInstance) */
	int32 OfficeGridId = 8;
	int32 OfficeSubId = 0;
	int32 AvailableStartGridId = 24;
	int32 AvailableStartSubId = 26;
	int32 PreviewTargetGridId = 25;
	int32 PreviewTargetSubId = 10;

	/** GridIds (bottom-left origin) that start revealed on a new game */
	UPROPERTY(EditDefaultsOnly, Category = "FC|WorldMap")
	TArray<int32> DefaultRevealedWorldMapGridIds;

private:
	UPROPERTY()
	TObjectPtr<UFCExpeditionData> CurrentExpedition;

	// World map runtime state ------------------------------------------------
	void WorldMap_InitOrLoad();
	void WorldMap_LoadLandMaskIfAvailable();
	void WorldMap_SyncFogTexture_Full();
	void WorldMap_UpdateFogPixel(int32 GlobalId);
	void WorldMap_StartAutosaveDebounced();
	void WorldMap_SaveNow();

	void RoutePaint_Tick();

	UTexture2D* CreateMaskTexture256();
	void UpdateMaskTextureFull(UTexture2D* Texture, const TArray<uint8>& Data256);
	void UpdateMaskTexturePixel(UTexture2D* Texture, const TArray<uint8>& Data256, int32 GlobalId);
	void ComputeCostsForPath(const TArray<int32>& Path, int32& OutMoney, int32& OutRisk) const;

	UPROPERTY()
	TObjectPtr<UTexture2D> FogTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> RouteTexture;

	TArray<uint8> RouteMask;

	FFCWorldMapExploration WorldMap;

	FTimerHandle RoutePaintTimer;
	FTimerHandle AutosaveTimer;

	int32 RoutePaintIndex = 0;
	bool bExplorationDirty = false;

	static constexpr const TCHAR* WorldMapSaveSlot = TEXT("FC_WorldMapExploration");
	static constexpr int32 WorldMapSaveUserIndex = 0;
};
