// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FCOverworldPlayerController.generated.h"

class UFCInputManager;
class AFCOverworldCamera;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogFCOverworldController, Log, All);

/**
 * Player controller for Overworld level with top-down camera control.
 * Uses UFCInputManager to switch to TopDown input mapping context.
 * Binds input actions and calls AFCOverworldCamera's HandlePan/HandleZoom methods.
 * Possesses BP_OverworldCamera and sets PlayerPawn reference for distance limiting.
 */
UCLASS()
class FC_API AFCOverworldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFCOverworldPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	/** Handle pan input and forward to camera */
	void OnPanAction(const FInputActionValue& Value);

	/** Handle zoom input and forward to camera */
	void OnZoomAction(const FInputActionValue& Value);

private:
	/** Input manager component (handles TopDown input context) */
	UPROPERTY()
	TObjectPtr<UFCInputManager> InputManager;

	/** Reference to possessed Overworld camera */
	UPROPERTY()
	TObjectPtr<AFCOverworldCamera> OverworldCamera;

	/** Input action for camera pan (WASD) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PanAction;

	/** Input action for camera zoom (Mouse Wheel) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;
};
