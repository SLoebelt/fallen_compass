// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/IFCInteractablePOI.h"
#include "FCUIManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCUIManager, Log, All);

/**
 * UFCUIManager
 * 
 * Game Instance Subsystem responsible for centralizing all UI widget creation
 * and lifecycle management. Eliminates Blueprint coupling from PlayerController
 * and ensures widgets persist across level transitions.
 */
UCLASS()
class FC_API UFCUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Widget class references (configured by GameInstance)
	UPROPERTY()
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	
	UPROPERTY()
	TSubclassOf<UUserWidget> SaveSlotSelectorWidgetClass;
	
	UPROPERTY()
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	// Widget class registry: TableObjectClass → WidgetClass (configured from GameInstance)
	UPROPERTY()
	TMap<TSubclassOf<AActor>, TSubclassOf<UUserWidget>> TableWidgetMap;

	/** Action selection widget class for POI interactions (configured from GameInstance) */
	UPROPERTY()
	TSubclassOf<UUserWidget> ActionSelectionWidgetClass;

	// Cached widget instances
	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> SaveSlotSelectorWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget;

	// Currently displayed table widget
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentTableWidget;

	// Currently displayed POI action selection widget
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentPOIActionSelectionWidget;

	// Widget lifecycle methods
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideMainMenu();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowSaveSlotSelector();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideSaveSlotSelector();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	// Table widget lifecycle methods
	UFUNCTION(BlueprintCallable, Category = "UI|Table")
	void ShowTableWidget(AActor* TableObject);
	
	UFUNCTION(BlueprintCallable, Category = "UI|Table")
	void CloseTableWidget();

	// Table widget queries
	UFUNCTION(BlueprintPure, Category = "UI|Table")
	UUserWidget* GetCurrentTableWidget() const { return CurrentTableWidget; }
	
	UFUNCTION(BlueprintPure, Category = "UI|Table")
	bool IsTableWidgetOpen() const { return CurrentTableWidget != nullptr; }

	// POI action selection widget lifecycle methods
	UFUNCTION(BlueprintCallable, Category = "UI|POI")
	UUserWidget* ShowPOIActionSelection(const TArray<FFCPOIActionData>& Actions, class UFCInteractionComponent* InteractionComponent);

	UFUNCTION(BlueprintCallable, Category = "UI|POI")
	void ClosePOIActionSelection();

	// POI widget queries
	UFUNCTION(BlueprintPure, Category = "UI|POI")
	UUserWidget* GetCurrentPOIActionSelectionWidget() const { return CurrentPOIActionSelectionWidget; }

	UFUNCTION(BlueprintPure, Category = "UI|POI")
	bool IsPOIActionSelectionOpen() const { return CurrentPOIActionSelectionWidget != nullptr; }

	// Button callback methods (called from Blueprint widgets)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleNewLegacyClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleContinueClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleLoadSaveClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleOptionsClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleQuitClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleBackFromSaveSelector();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleSaveSlotSelected(const FString& SlotName);
};
