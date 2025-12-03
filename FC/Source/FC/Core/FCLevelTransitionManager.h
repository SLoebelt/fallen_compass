#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FCGameStateManager.h"
#include "FCLevelTransitionManager.generated.h"

class UFCTransitionManager;
class UFCLevelManager;
class UFCUIManager;
class UFCExpeditionManager;

/**
 * UFCLevelTransitionManager
 *
 * Game-instance subsystem that orchestrates high-level transitions which
 * combine validated game state changes (UFCGameStateManager), level
 * loading (UFCLevelManager), screen fades (UFCTransitionManager), and
 * major UI flows (UFCUIManager).
 *
 * See Docs/Technical_Documentation.md "Transition Responsibilities &
 * Orchestration (Week 4 - Step 4.7)" for design details and examples.
 */
UCLASS()
class FC_API UFCLevelTransitionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Start an expedition from Office table view into Overworld_Travel. */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void StartExpeditionFromOfficeTableView();

	/** Return to main menu hosted in Office from any gameplay state. */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void ReturnToMainMenuFromGameplay();

	/** Return from Overworld_Travel to Office and show expedition summary. */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void ReturnFromOverworldToOfficeWithSummary();

	/** Close the expedition summary in Office and return to normal exploration. */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void CloseExpeditionSummaryAndReturnToOffice();

	/**
	 * Called on level start (e.g., Office BeginPlay) to complete any transitions
	 * that used TransitionViaLoading() before the level load. For example,
	 * if we entered Loading -> ExpeditionSummary and the Office level was loaded,
	 * this will transition into ExpeditionSummary and show the summary UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void InitializeLevelTransitionOnLevelStart();

	/**
	 * Future unified entry point for Office startup. Will wrap
	 * InitializeLevelTransitionOnLevelStart plus fresh-start and
	 * load-from-save behaviors (see 4.7.4).
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void InitializeOnLevelStart();

	/**
	 * Generic transition entry point (4.7): orchestrates game state, level,
	 * and fade in one place. For now only logs and defers to existing
	 * specialized helpers until call sites are migrated.
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void RequestTransition(EFCGameStateID TargetState, FName TargetLevelName = NAME_None, bool bUseFade = true);

	/**
	 * Table view flows (4.7): entry/exit points for Office table
	 * interaction. These centralize state transitions and delegate
	 * camera/input reactions to AFCPlayerController::OnGameStateChanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void EnterOfficeTableView(AActor* TableActor);

	UFUNCTION(BlueprintCallable, Category = "FC|Transition")
	void ExitOfficeTableView();

private:

	/** Pending target state for an in-progress TransitionViaLoading() flow. */
	EFCGameStateID PendingLoadingTarget = EFCGameStateID::None;

	UFCGameStateManager* GetGameStateManager() const;
	UFCTransitionManager* GetTransitionManager() const;
	UFCLevelManager* GetLevelManager() const;
	UFCUIManager* GetUIManager() const;
	UFCExpeditionManager* GetExpeditionManager() const;
};
