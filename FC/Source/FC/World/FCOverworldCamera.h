// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "FCOverworldCamera.generated.h"

class UInputAction;
class USpringArmComponent;
struct FInputActionValue;

/**
 * Top-down camera actor for Overworld level with Wartales-style distance-limited panning and zoom.
 * Camera movement is constrained by crew skills (CrewVisionRange parameter with separate multipliers).
 * Always north-aligned (Yaw=0) with no rotation support.
 * 
 * Architecture: C++ handles core logic (input, math, distance limiting, north lock, Tick),
 * Blueprint child (BP_OverworldCamera) provides designer-friendly parameter defaults.
 */
UCLASS()
class FC_API AFCOverworldCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	AFCOverworldCamera();

	virtual void Tick(float DeltaTime) override;

	/** Set the player pawn reference for distance limiting (called by controller) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPlayerPawn(APawn* NewPawn);

	/** Get current crew vision range (for UI or skill system integration) */
	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetCrewVisionRange() const { return CrewVisionRange; }

	/** Handle camera panning input (called by controller) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void HandlePan(const FInputActionValue& Value);

	/** Handle camera zoom input (called by controller) */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void HandleZoom(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;

	/** Apply distance limiting to camera position relative to player pawn */
	void ApplyDistanceLimit();

	/** Force camera rotation to north (Yaw=0) */
	void EnforceNorthAlignment();

	/** Draw debug visualization for pan/zoom limits */
	void DrawDebugLimits() const;

private:
	// ========== Movement Parameters ==========
	
	/** Camera pan speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Movement", meta = (AllowPrivateAccess = "true"))
	float PanSpeed = 500.0f;

	/** Camera zoom speed (units per wheel tick) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom", meta = (AllowPrivateAccess = "true"))
	float ZoomSpeed = 100.0f;

	/** Minimum zoom distance (closest to convoy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom", meta = (AllowPrivateAccess = "true"))
	float ZoomMin = 500.0f;

	// ========== Crew Skill Parameters ==========
	
	/** Base vision range for crew (drives both pan and zoom limits) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Crew Skills", meta = (AllowPrivateAccess = "true", Tooltip = "Base vision range, will be driven by crew skills in future"))
	float CrewVisionRange = 2000.0f;

	/** Multiplier for max pan distance from convoy (e.g., 1.5 = 150% of CrewVisionRange) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Movement", meta = (AllowPrivateAccess = "true"))
	float PanDistanceMultiplier = 1.5f;

	/** Multiplier for max zoom distance (e.g., 2.0 = 200% of CrewVisionRange) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Crew Skills", meta = (AllowPrivateAccess = "true"))
	float ZoomDistanceMultiplier = 2.0f;

	// ========== References ==========
	
	/** Player pawn reference (convoy) for distance limiting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|References", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<APawn> PlayerPawn;

	/** SpringArm component (optional, created in constructor) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	// ========== Debug ==========
	
	/** Show debug radius circles (yellow=pan, cyan=zoom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugRadius = false;

	/** Pending pan movement delta (accumulated from input) */
	FVector PendingMovementDelta = FVector::ZeroVector;
};
