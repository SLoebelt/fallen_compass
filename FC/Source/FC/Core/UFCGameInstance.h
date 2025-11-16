#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "UFCGameInstance.generated.h"

class UFCSaveGame;

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
    /** Editor-configurable map for boot (Main Menu until more flow states exist). */
    UPROPERTY(EditDefaultsOnly, Category = "Boot")
    FName StartupMap = TEXT("L_MainMenu");

    /** Editor-configurable office map used after Start Game. */
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

    /** Active expedition identifier; persists across level loads. */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition")
    FString CurrentExpeditionId;

    /** Simple fog-of-war memory: which regions have been discovered already. */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition")
    TArray<FName> DiscoveredRegions;

    /** Tracks how many expeditions have been undertaken; seeds rest/skip systems later. */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    int32 ExpeditionsCounter = 1;

    /** Marks whether unsaved changes exist (crew edits, loadout tweaks, etc.). */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    bool bIsSessionDirty = false;

    /** Broadcast whenever global expedition context changes. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnExpeditionContextChanged OnExpeditionContextChanged;

    /** Returns the current game version string */
    UFUNCTION(BlueprintPure, Category = "Version")
    FString GetGameVersion() const;

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
