// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/FCInputManager.h"
#include "Interaction/IFCInteractablePOI.h"
#include "Input/FCInputConfig.h"
#include "Components/Data/FCPlayerCameraTypes.h"
#include "FCPlayerController.generated.h"


class UInputAction;
class UInputMappingContext;
class ACameraActor;
class UUserWidget;
class UFCCameraManager;
class UFCInputManager;
class UFCInteractionComponent;
class UFCPlayerModeCoordinator;
class AFCOverworldConvoy;
class AFCConvoyMember;
class UInputConfig;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassPlayerController, Log, All);

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

	// Called when this controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;

	EFCPlayerCameraMode GetCameraMode() const;
	bool IsPauseMenuDisplayed() const { return bIsPauseMenuDisplayed; }
	EFCInputMappingMode GetCurrentMappingMode() const;
	EFCGameState GetCurrentGameState() const { return CurrentGameState; }

    /** Convenience: exposes InputManager's currently assigned InputConfig (may be null). */
    const UFCInputConfig* GetInputConfig() const;

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

	bool CanWorldClick() const;
	bool CanWorldInteract() const;

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

	/** Timer-safe deferred call to restore player position after level load */
	UFUNCTION()
	void RestorePlayerPositionDeferred();

	/** Fade screen to black with optional loading indicator */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void FadeScreenOut(float Duration = 1.0f, bool bShowLoading = false);

	/** Fade screen from black to clear */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void FadeScreenIn(float Duration = 1.0f);

	/** Getter for the current active convoy */
	UFUNCTION(BlueprintCallable, Category="FC|Convoy")
	AFCOverworldConvoy* GetActiveConvoy() const { return ActiveConvoy; }

	UFUNCTION(BlueprintCallable, Category="FC|Convoy")
	void SetActiveConvoy(AFCOverworldConvoy* InConvoy);

	/** Move convoy to target location (called by InteractionComponent for POI navigation) */
	UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
	void MoveConvoyToLocation(const FVector& TargetLocation);

	// TODO - Remove after Week 1 - no longer AI-controlled
	/** Get the commanded explorer character reference (Camp/POI scenes) */
	UFUNCTION(BlueprintCallable, Category = "FC|Camp")
	AFC_ExplorerCharacter* GetCommandedExplorer() const { return CommandedExplorer; }

	/** Command explorer to move to target location (Camp/POI click-to-move) */
	UFUNCTION(BlueprintCallable, Category = "FC|Camp")
	void MoveExplorerToLocation(const FVector& TargetLocation);

	/** Set the fixed camera actor used for POI/local scenes (e.g. Camp). */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPOISceneCameraActor(ACameraActor* InPOICamera);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPauseMenuDisplayed;

	/** Tracks whether player is currently in table view mode */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsInTableView = false;

	/** Reference to possessed convoy in Overworld */
	UPROPERTY()
	AFCOverworldConvoy* ActiveConvoy;

	// TODO Remove after Week 1 - no longer AI-controlled
	/** Reference to commanded explorer in Camp/POI scenes (not possessed, AI-controlled) */
	UPROPERTY()
	AFC_ExplorerCharacter* CommandedExplorer;

	/** Current game state (MainMenu, Gameplay, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFCGameState CurrentGameState;

	/** Camera management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCCameraManager> CameraManager;

	/** Input management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCInputManager> InputManager;

	/** Interaction management component (handles POI interactions across all scenes) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCInteractionComponent> InteractionComponent;

	/** Player mode coordinator component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFCPlayerModeCoordinator> PlayerModeCoordinator;

	/** Fixed camera actor for POI/local scenes such as Camp */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<ACameraActor> POISceneCameraActor;

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

	void BindOverworldConvoyDelegates();
	void UnbindOverworldConvoyDelegates();

public:
	void ApplyPresentationForGameState(EFCGameStateID OldState, EFCGameStateID NewState);
};
