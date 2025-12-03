#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "UFCGameInstance.generated.h"

class UFCSaveGame;

/**
 * FFCGameStateData
 *
 * Persistent game state data that survives level transitions.
 * Stores resources, economy, and time progression.
 */
USTRUCT(BlueprintType)
struct FFCGameStateData
{
    GENERATED_BODY()

    /** Current supplies available */
    UPROPERTY(BlueprintReadWrite, Category = "Game State")
    int32 Supplies = 100;

    /** Current money/gold available */
    UPROPERTY(BlueprintReadWrite, Category = "Game State")
    int32 Money = 500;

    /** Current day in campaign (starts at 1) */
    UPROPERTY(BlueprintReadWrite, Category = "Game State")
    int32 Day = 1;

    /** Default constructor */
    FFCGameStateData()
        : Supplies(100)
        , Money(500)
        , Day(1)
    {
    }
};

/**
 * UFCGameInstance centralizes long-lived expedition context per GDD §3.1.
 * Fields are placeholders for Week 1; they become real once meta-systems land.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExpeditionContextChanged);

UCLASS()
class FC_API UFCGameInstance : public UGameInstance
{
	GENERATED_BODY()

	public:
    virtual void Init() override;
    virtual void Shutdown() override;

    /** Sets the currently selected expedition slot/contract and notifies listeners. */
    UFUNCTION(BlueprintCallable, Category = "Expedition")
    void SetCurrentExpeditionId(const FString& InExpeditionId);

    /** Clears runtime flags when the session is safely written to disk. */
    UFUNCTION(BlueprintCallable, Category = "Expedition")
    void MarkSessionSaved();

	public:
    /** Editor-configurable office map (contains main menu and gameplay). */
    UPROPERTY(EditDefaultsOnly, Category = "Boot")
    FName OfficeMap = TEXT("L_Office");

    /** Placeholder for the future overworld/top-down expeditions map. */
    UPROPERTY(EditDefaultsOnly, Category = "Boot")
    FName ExpeditionsMap = NAME_None;

    /** Data asset hook for expedition templates (objectives, risks, etc.). */
    UPROPERTY(EditDefaultsOnly, Category = "Expedition")
    TSoftObjectPtr<UDataTable> ExpeditionDefinitions;

    /** Widget class for screen transitions (fades, loading screens) */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSoftClassPtr<class UFCScreenTransitionWidget> TransitionWidgetClass;

    /** Widget class for main menu (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;
    
    /** Widget class for save slot selector (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> SaveSlotSelectorWidgetClass;
    
    /** Widget class for pause menu (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    /** Action selection widget class for POI interactions (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> ActionSelectionWidgetClass;

    /** Overworld view-only map HUD widget class (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> OverworldMapHUDWidgetClass;

    /** Expedition summary widget class (configured in Blueprint, used by UIManager) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> ExpeditionSummaryWidgetClass;

    /** Table widget registry: Maps table object classes to their widget classes */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TMap<TSubclassOf<AActor>, TSubclassOf<UUserWidget>> TableWidgetMap;

    /** Level metadata DataTable (data-driven level configuration) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
    TObjectPtr<UDataTable> LevelMetadataTable;

    // ---------------------------------------------------------------------
    // Expedition Manager Subsystem Configuration (Week 4)
    // ---------------------------------------------------------------------

    /** Land mask texture for world map (256x256, white=land, black=water) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    TSoftObjectPtr<UTexture2D> WorldMapLandMaskTexture;

    /** Overworld minimum world bounds (for coordinate mapping) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    FVector2D OverworldWorldMin = FVector2D(-50000.f, -50000.f);

    /** Overworld maximum world bounds (for coordinate mapping) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    FVector2D OverworldWorldMax = FVector2D(50000.f, 50000.f);

    /** Office grid cell ID on the 16x16 planning map */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 OfficeGridId = 8;

    /** Office sub-cell ID within the grid cell (0-15) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 OfficeSubId = 0;

    /** Milestone: Available start point grid ID (for Week 4 demo) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 AvailableStartGridId = 24;

    /** Milestone: Available start point sub-cell ID (for Week 4 demo) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 AvailableStartSubId = 26;

    /** Milestone: Preview target grid ID (for route preview demo) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 PreviewTargetGridId = 25;

    /** Milestone: Preview target sub-cell ID (for route preview demo) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    int32 PreviewTargetSubId = 0;

    /** GridIds (bottom-left origin) that start revealed on a new game */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Expedition|WorldMap")
    TArray<int32> DefaultRevealedWorldMapGridIds;

    // ---------------------------------------------------------------------
    // Original Properties
    // ---------------------------------------------------------------------

    /** Active expedition identifier; persists across level loads. */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition")
    FString CurrentExpeditionId;

    /** Simple fog-of-war memory: which regions have been discovered already. */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition")
    TArray<FName> DiscoveredRegions;

    /** Tracks how many expeditions have been undertaken; seeds rest/skip systems later. */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    int32 ExpeditionsCounter = 1;

    /** Current game state data (Week 2 feature) */
    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    FFCGameStateData GameStateData;

    /** Current supplies available for expeditions (Week 2 feature) - DEPRECATED: Use GameStateData.Supplies */
    UPROPERTY(BlueprintReadOnly, Category = "Resources")
    int32 CurrentSupplies = 100;

    /** Marks whether unsaved changes exist (crew edits, loadout tweaks, etc.). */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    bool bIsSessionDirty = false;

    /** Broadcast whenever global expedition context changes. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnExpeditionContextChanged OnExpeditionContextChanged;

    /** Returns the current game version string */
    UFUNCTION(BlueprintPure, Category = "Version")
    FString GetGameVersion() const;

    /** Get current game state data (read-only access) */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    const FFCGameStateData& GetGameStateData() const { return GameStateData; }

    /** Get current supplies (Week 2 resource system) */
    UFUNCTION(BlueprintPure, Category = "Resources")
    int32 GetCurrentSupplies() const { return GameStateData.Supplies; }

    /** Add supplies to the pool */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddSupplies(int32 Amount);

    /** Consume supplies if available (returns success and remaining supplies) */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 ConsumeSupplies(int32 Amount, bool& bSuccess);

    /** Get current money (campaign currency) */
    UFUNCTION(BlueprintPure, Category = "Resources")
    int32 GetMoney() const { return GameStateData.Money; }

    /** Add money to the pool (can be negative, clamps at zero) */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddMoney(int32 Delta);

    /** Consume money if available; returns true on success, false if insufficient */
    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ConsumeMoney(int32 Amount);

    /** Save current game state to specified slot */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    bool SaveGame(const FString& SlotName);

    /** Load game state from specified slot (async) */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void LoadGameAsync(const FString& SlotName);

    /** Get list of available save slots with metadata */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    TArray<FString> GetAvailableSaveSlots();

    /** Get the most recent save slot name */
    UFUNCTION(BlueprintPure, Category = "SaveGame")
    FString GetMostRecentSave();

    /** Delegate for when async load completes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, bool, bSuccess);
    UPROPERTY(BlueprintAssignable, Category = "SaveGame")
    FOnGameLoaded OnGameLoaded;

    /** Restore player position from pending load data (called after level loads) */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void RestorePlayerPosition();

    /** Check if we're currently restoring from a save game */
    UFUNCTION(BlueprintPure, Category = "SaveGame")
    bool IsRestoringSaveGame() const { return PendingLoadData != nullptr; }

    /** Called when a level finishes loading - triggers fade-in if transition is active */
    void OnPostLoadMapWithWorld(UWorld* LoadedWorld);

private:
    /** Cached save data for restoring player position after level load */
    UPROPERTY()
    TObjectPtr<UFCSaveGame> PendingLoadData;

    /** Pending level name for deferred load after fade */
    FName PendingLevelLoad;

    /** Timer handle for level load delay */
    FTimerHandle LevelLoadTimerHandle;
};
