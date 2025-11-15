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
	TableView
};

UENUM(BlueprintType)
enum class EFCInputMappingMode : uint8
{
	FirstPerson = 0,
	TopDown,
	Fight,
	StaticScene
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

	/** Switch to a different input mapping context (e.g., FirstPerson → TopDown) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputMappingMode(EFCInputMappingMode NewMode);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFCPlayerCameraMode CameraMode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPauseMenuDisplayed;

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
	TObjectPtr<UInputAction> PauseAction;

	void HandleInteractPressed();
	void HandlePausePressed();

	void EnterTableViewPlaceholder();
	void ExitTableViewPlaceholder();
	void ShowPauseMenuPlaceholder();
	void HidePauseMenuPlaceholder();
	void SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode);

private:
	void LogStateChange(const FString& Context) const;
};


