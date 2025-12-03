// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/FCInputManager.h"
#include "Interaction/IFCInteractablePOI.h"
#include "FCPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ACameraActor;
class UUserWidget;
class UFCCameraManager;
class UFCInputManager;
class AFCOverworldConvoy;
class AFCConvoyMember;
struct FInputActionValue;

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

/** Sub-mode for Office table view to distinguish desk vs. object focus. */
UENUM(BlueprintType)
enum class EOfficeTableViewSubMode : uint8
{
	None = 0,
	Desk,
	Object
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

	// 4.7.4: InitializeMainMenu is intended to be driven by
	// UFCLevelTransitionManager (via InitializeOnLevelStart) rather
	// than called directly from Level Blueprints once the startup
	// refactor is complete.
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

	/** Returns true if UI state allows world interaction traces (no blocking map/table widgets). */
	bool CanProcessWorldInteraction() const;

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

	/** Get the possessed convoy reference */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	AFCOverworldConvoy* GetPossessedConvoy() const { return PossessedConvoy; }

	/** Move convoy to target location (called by InteractionComponent for POI navigation) */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	void MoveConvoyToLocation(const FVector& TargetLocation);

	/** Set the menu camera actor used by the camera manager for main menu view. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetMenuCameraActor(ACameraActor* InMenuCamera);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPauseMenuDisplayed;

	/** Tracks whether player is currently in table view mode */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsInTableView = false;

	/** Reference to possessed convoy in Overworld */
	UPROPERTY()
	AFCOverworldConvoy* PossessedConvoy;

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

	/** Input action for toggling the Overworld map (M key) - Week 4 */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
	TObjectPtr<UInputAction> ToggleOverworldMapAction;

	/** View-only Overworld map widget currently open (if any) */
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentOverworldMapWidget;

	/** Sub-mode within Office table view (desk overview vs. object focus). */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	EOfficeTableViewSubMode OfficeTableViewSubMode = EOfficeTableViewSubMode::None;

	void HandleInteractPressed();
	void HandlePausePressed();
	void HandleQuickSavePressed();
	void HandleQuickLoadPressed();
	void HandleOverworldPan(const FInputActionValue& Value);
	void HandleOverworldZoom(const FInputActionValue& Value);
	void HandleClick(const FInputActionValue& Value);
	void HandleOverworldClickMove();
	void HandleToggleOverworldMap();

	/** Handle game state changes (bind to GameStateManager.OnStateChanged) */
	UFUNCTION()
	void OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame();

	void ShowPauseMenuPlaceholder();
	void HidePauseMenuPlaceholder();
	void SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode);

private:
	void LogStateChange(const FString& Context) const;
};


