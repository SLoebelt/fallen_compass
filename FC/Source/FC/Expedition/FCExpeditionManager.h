// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Logging/LogMacros.h"
#include "Expedition/FCExpeditionData.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCExpedition, Log, All);

#include "FCExpeditionManager.generated.h"

/**
 * Delegate fired when expedition state changes (started, ended, etc.)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExpeditionStateChanged, UFCExpeditionData*, ExpeditionData);

/**
 * UFCExpeditionManager
 *
 * Game Instance Subsystem managing expedition lifecycle and state.
 * Persists across level loads and provides unified API for expedition operations.
 *
 * Usage:
 *   UFCExpeditionManager* ExpedMgr = GetGameInstance()->GetSubsystem<UFCExpeditionManager>();
 *   ExpedMgr->StartNewExpedition(TEXT("Test Expedition"), 50);
 */
UCLASS()
class FC_API UFCExpeditionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Start a new expedition with given parameters
	 * @param ExpeditionName Display name for the expedition
	 * @param AllocatedSupplies Starting supplies for this expedition
	 * @return The created expedition data object
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	UFCExpeditionData* StartNewExpedition(const FString& ExpeditionName, int32 AllocatedSupplies);

	/**
	 * Get the currently active expedition (or nullptr if none)
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	UFCExpeditionData* GetCurrentExpedition() const { return CurrentExpedition; }

	/**
	 * End the current expedition
	 * @param bSuccess Whether the expedition succeeded or failed
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	void EndExpedition(bool bSuccess);

	/**
	 * Check if an expedition is currently active
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Expedition")
	bool IsExpeditionActive() const;

	/** Broadcast when expedition state changes */
	UPROPERTY(BlueprintAssignable, Category = "FC|Expedition")
	FOnExpeditionStateChanged OnExpeditionStateChanged;

private:
	/** Currently active expedition (nullptr if none) */
	UPROPERTY()
	TObjectPtr<UFCExpeditionData> CurrentExpedition;
};
