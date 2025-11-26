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
 * FFCExpeditionPlanningState
 *
 * Runtime state for expedition planning (non-DataTable).
 * Stores player's current area and start point selections for the map UI.
 * Persists across UI open/close to support interrupted planning sessions.
 */
USTRUCT(BlueprintType)
struct FFCExpeditionPlanningState
{
    GENERATED_BODY()

    /** Currently selected area ID */
    UPROPERTY(BlueprintReadWrite, Category = "Planning")
    FName SelectedAreaID;

    /** Currently selected start point ID */
    UPROPERTY(BlueprintReadWrite, Category = "Planning")
    FName SelectedStartPointID;

    /** True if player ESC'd from planning widget (resume planning on next open) */
    UPROPERTY(BlueprintReadWrite, Category = "Planning")
    bool bPlanningInProgress;

    /** Default constructor */
    FFCExpeditionPlanningState()
        : SelectedAreaID(NAME_None)
        , SelectedStartPointID(NAME_None)
        , bPlanningInProgress(false)
    {
    }

    /** Clear all selections */
    void ClearSelection()
    {
        SelectedAreaID = NAME_None;
        SelectedStartPointID = NAME_None;
        bPlanningInProgress = false;
    }

    /** Check if state is valid (both area and start point selected) */
    bool IsValid() const
    {
        return SelectedAreaID != NAME_None && SelectedStartPointID != NAME_None;
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

    /** Table widget registry: Maps table object classes to their widget classes */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TMap<TSubclassOf<AActor>, TSubclassOf<UUserWidget>> TableWidgetMap;

    /** Level metadata DataTable (data-driven level configuration) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
    TObjectPtr<UDataTable> LevelMetadataTable;

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

    /** Current expedition planning state (Week 4 feature) */
    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    FFCExpeditionPlanningState CurrentPlanningState;

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

    /** Save current planning state (called by WBP_WorldMap when selections change) */
    UFUNCTION(BlueprintCallable, Category = "Expedition Planning")
    void SavePlanningState(FName AreaID, FName StartPointID);

    /** Load current planning state (called by WBP_WorldMap on open) */
    UFUNCTION(BlueprintPure, Category = "Expedition Planning")
    FFCExpeditionPlanningState LoadPlanningState() const { return CurrentPlanningState; }

    /** Clear planning state (called after expedition starts) */
    UFUNCTION(BlueprintCallable, Category = "Expedition Planning")
    void ClearPlanningState();

    /** Get World Map Manager subsystem (convenience accessor) */
    UFUNCTION(BlueprintPure, Category = "Expedition Planning")
    class UFCWorldMapManager* GetWorldMapManager() const;

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
