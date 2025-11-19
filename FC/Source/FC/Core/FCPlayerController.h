// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FCPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassPlayerController, Log, All);

UENUM(BlueprintType)
enum class EFCPlayerCameraMode : uint8
{
	FirstPerson = 0,
	TableView,
	MainMenu,
	SaveSlotView
};

UENUM(BlueprintType)
enum class EFCInputMappingMode : uint8
{
	FirstPerson = 0,
	TopDown,
	Fight,
	StaticScene
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

	EFCPlayerCameraMode GetCameraMode() const { return CameraMode; }
	bool IsPauseMenuDisplayed() const { return bIsPauseMenuDisplayed; }
	EFCInputMappingMode GetCurrentMappingMode() const { return CurrentMappingMode; }
	EFCGameState GetCurrentGameState() const { return CurrentGameState; }

	/** Switch to a different input mapping context (e.g., FirstPerson → TopDown) */
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFCPlayerCameraMode CameraMode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPauseMenuDisplayed;

	/** Tracks whether player is currently in table view mode */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsInTableView = false;

	/** Current game state (MainMenu, Gameplay, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFCGameState CurrentGameState;

	/** Reference to the MenuCamera actor in L_Office */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<class ACameraActor> MenuCamera;



	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 DefaultMappingPriority = 0;

	/** Current input mapping mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	EFCInputMappingMode CurrentMappingMode;

	/** Input mapping context for first-person office exploration */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> FirstPersonMappingContext;

	/** Input mapping context for top-down overworld navigation */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> TopDownMappingContext;

	/** Input mapping context for combat/fight sequences */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> FightMappingContext;

	/** Input mapping context for static scenes (cutscenes, dialogue) */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> StaticSceneMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> EscapeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSaveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickLoadAction;

	void HandleInteractPressed();
	void HandlePausePressed();
	void HandleQuickSavePressed();
	void HandleQuickLoadPressed();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame();
	
	void EnterTableViewPlaceholder();
	void ExitTableViewPlaceholder();
	void ShowPauseMenuPlaceholder();
	void HidePauseMenuPlaceholder();
	void SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode);

private:
	void LogStateChange(const FString& Context) const;
};


