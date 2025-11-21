// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/FCInputManager.h"
#include "FCPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ACameraActor;
class UUserWidget;
class UFCCameraManager;
class UFCInputManager;

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassPlayerController, Log, All);

UENUM(BlueprintType)
enum class EFCPlayerCameraMode : uint8
{
	FirstPerson = 0,
	TableView,
	MainMenu,
	SaveSlotView,
	TopDown  // Week 3: Overworld camera mode
};

UENUM(BlueprintType)
enum class EFCGameState : uint8
{
	MainMenu = 0,
	Gameplay,
	TableView,
	Paused,
	Loading
};

/**
 * Lightweight PlayerController scaffold for Week 1 prototypes.
 * Provides logging hooks for interaction, pause, and camera-mode changes.
 */
UCLASS()
class FC_API AFCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFCPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	EFCPlayerCameraMode GetCameraMode() const;
	bool IsPauseMenuDisplayed() const { return bIsPauseMenuDisplayed; }
	EFCInputMappingMode GetCurrentMappingMode() const;
	EFCGameState GetCurrentGameState() const { return CurrentGameState; }

	/**
	 * Switch to a different input mapping context (e.g., FirstPerson → TopDown).
	 * Delegates to UFCInputManager component for centralized input management.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputMappingMode(EFCInputMappingMode NewMode);

	/** Transition to the specified camera mode with smooth blending */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime = 2.0f);

	/** Initialize the main menu state (called on level start) */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void InitializeMainMenu();

	/** Transition from main menu to gameplay */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void TransitionToGameplay();

	/** Return to main menu state (from gameplay) */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ReturnToMainMenu();

	/**
	 * Handle table object click (raycast from camera to detect table objects)
	 * Bound to IA_TableClick input action
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Table Interaction")
	void HandleTableObjectClick();

	/**
	 * Perform table interaction with clicked object
	 * @param TableObject The actor to interact with (must implement IFCTableInteractable)
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Table Interaction")
	void OnTableObjectClicked(AActor* TableObject);

	/**
	 * Close current table widget and return to table view or first-person
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Table Interaction")
	void CloseTableWidget();

	/** Dev quick save (F5) */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void DevQuickSave();

	/** Dev quick load (F9) */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void DevQuickLoad();

	/** Timer-safe restore input after camera blend completes */
	UFUNCTION()
	void RestoreInputAfterBlend();

	/** Timer-safe deferred call to restore player position after level load */
	UFUNCTION()
	void RestorePlayerPositionDeferred();

	/** Fade screen to black with optional loading indicator */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void FadeScreenOut(float Duration = 1.0f, bool bShowLoading = false);

	/** Fade screen from black to clear */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void FadeScreenIn(float Duration = 1.0f);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPauseMenuDisplayed;

	/** Tracks whether player is currently in table view mode */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsInTableView = false;

	/** Current game state (MainMenu, Gameplay, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFCGameState CurrentGameState;

	/** Camera management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCCameraManager> CameraManager;

	/** Input management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCInputManager> InputManager;

	/** Input action for interaction (E key) */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> InteractAction;

	/** Input action for table object click */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> ClickAction;

	/** Input action for pause/escape (ESC key) */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> EscapeAction;

	/** Input action for quick save (F5 key) */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> QuickSaveAction;

	/** Input action for quick load (F9 key) */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> QuickLoadAction;

	/** Input action for Overworld camera pan (WASD) - Week 3 */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> OverworldPanAction;

	/** Input action for Overworld camera zoom (Mouse Wheel) - Week 3 */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> OverworldZoomAction;

	void HandleInteractPressed();
	void HandlePausePressed();
	void HandleQuickSavePressed();
	void HandleQuickLoadPressed();
	void HandleOverworldPan(const FInputActionValue& Value);
	void HandleOverworldZoom(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame();
	
	void ShowPauseMenuPlaceholder();
	void HidePauseMenuPlaceholder();
	void SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode);

private:
	void LogStateChange(const FString& Context) const;
};


