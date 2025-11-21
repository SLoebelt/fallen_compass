// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "World/FCOverworldCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"

AFCOverworldCamera::AFCOverworldCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create SpringArm component for zoom functionality
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.0f; // Starting zoom distance
	SpringArm->bDoCollisionTest = false; // No collision for camera
	SpringArm->bEnableCameraLag = false; // Instant response
	SpringArm->SetRelativeRotation(FRotator(-70.0f, 0.0f, 0.0f)); // Top-down angle

	// Reattach camera to spring arm tip (override ACameraActor default)
	GetCameraComponent()->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	GetCameraComponent()->SetRelativeLocation(FVector::ZeroVector);
	GetCameraComponent()->SetRelativeRotation(FRotator::ZeroRotator);
}

void AFCOverworldCamera::BeginPlay()
{
	Super::BeginPlay();

	// Enforce initial north alignment
	EnforceNorthAlignment();
}

void AFCOverworldCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply pending pan movement with distance limiting
	if (!PendingMovementDelta.IsNearlyZero())
	{
		FVector NewLocation = GetActorLocation() + PendingMovementDelta * DeltaTime;
		SetActorLocation(NewLocation);
		ApplyDistanceLimit();
		PendingMovementDelta = FVector::ZeroVector;
	}

	// Force north alignment every frame
	EnforceNorthAlignment();

	// Draw debug visualization
	if (bShowDebugRadius)
	{
		DrawDebugLimits();
	}
}

void AFCOverworldCamera::SetPlayerPawn(APawn* NewPawn)
{
	PlayerPawn = NewPawn;
}

void AFCOverworldCamera::HandlePan(const FInputActionValue& Value)
{
	// Get 2D input vector from Enhanced Input
	// Enhanced Input: X=horizontal (left/right), Y=vertical (forward/back)
	const FVector2D InputVector = Value.Get<FVector2D>();

	// Convert to 3D world movement
	// Unreal coords: X=forward/back, Y=right/left, Z=up/down
	// So we need to swap: InputVector.Y → WorldX (forward), InputVector.X → WorldY (right)
	const FVector MovementDelta = FVector(InputVector.Y, InputVector.X, 0.0f) * PanSpeed;

	// Accumulate delta for Tick application (frame-rate independent)
	PendingMovementDelta += MovementDelta;
}

void AFCOverworldCamera::HandleZoom(const FInputActionValue& Value)
{
	if (!SpringArm) return;

	// Get zoom delta (mouse wheel: positive=zoom in, negative=zoom out)
	const float ZoomDelta = Value.Get<float>();

	// Calculate new arm length (subtract because wheel up should zoom in = decrease distance)
	float NewArmLength = SpringArm->TargetArmLength - (ZoomDelta * ZoomSpeed);

	// Calculate max zoom distance based on crew vision range
	const float MaxZoomDistance = CrewVisionRange * ZoomDistanceMultiplier;

	// Clamp between min and skill-based max
	NewArmLength = FMath::Clamp(NewArmLength, ZoomMin, MaxZoomDistance);

	// Apply new zoom distance
	SpringArm->TargetArmLength = NewArmLength;
}

void AFCOverworldCamera::ApplyDistanceLimit()
{
	// Only apply limit if player pawn reference is valid
	if (!PlayerPawn.IsValid()) return;

	const FVector PawnLocation = PlayerPawn->GetActorLocation();
	const FVector CameraLocation = GetActorLocation();

	// Calculate 2D distance (ignore Z-axis for top-down view)
	const float Distance = FVector::Dist2D(CameraLocation, PawnLocation);

	// Calculate max allowed pan distance
	const float MaxPanDistance = CrewVisionRange * PanDistanceMultiplier;

	// Clamp camera position to circular boundary if exceeding limit
	if (Distance > MaxPanDistance)
	{
		// Get 2D direction from pawn to camera
		FVector Direction = CameraLocation - PawnLocation;
		Direction.Z = 0.0f; // Flatten to 2D
		Direction.Normalize();

		// Calculate clamped position at max distance
		FVector ClampedLocation = PawnLocation + Direction * MaxPanDistance;
		ClampedLocation.Z = CameraLocation.Z; // Preserve Z height

		SetActorLocation(ClampedLocation);
	}
}

void AFCOverworldCamera::EnforceNorthAlignment()
{
	// Force camera rotation to always face north (Yaw=0)
	FRotator CurrentRotation = GetActorRotation();
	if (!FMath::IsNearlyZero(CurrentRotation.Yaw))
	{
		CurrentRotation.Yaw = 0.0f;
		SetActorRotation(CurrentRotation);
	}
}

void AFCOverworldCamera::DrawDebugLimits() const
{
	if (!PlayerPawn.IsValid()) return;

	const FVector PawnLocation = PlayerPawn->GetActorLocation();
	const UWorld* World = GetWorld();
	if (!World) return;

	// Draw yellow circle for pan distance limit
	const float PanRadius = CrewVisionRange * PanDistanceMultiplier;
	DrawDebugCircle(World, PawnLocation, PanRadius, 64, FColor::Yellow, false, -1.0f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0), false);

	// Draw cyan circle for zoom distance limit
	const float ZoomRadius = CrewVisionRange * ZoomDistanceMultiplier;
	DrawDebugCircle(World, PawnLocation, ZoomRadius, 64, FColor::Cyan, false, -1.0f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0), false);
}
