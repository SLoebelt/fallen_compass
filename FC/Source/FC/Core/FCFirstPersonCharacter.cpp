// Copyright Epic Games, Inc. All Rights Reserved.

#include "FCFirstPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputAction.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"

DEFINE_LOG_CATEGORY(LogFallenCompassCharacter);

AFCFirstPersonCharacter::AFCFirstPersonCharacter()
{
    // Set capsule size for first-person character
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Configure character to use controller rotation for aiming (standard FPS)
    bUseControllerRotationPitch = false; // Camera pitch controlled via ClampCameraPitch
    bUseControllerRotationYaw = true;    // Character turns with mouse X
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = false; // Don't rotate to movement direction
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Create first-person camera at eye level (~64 units from capsule base)
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(25.0f, 0.0f, 66.0f)); // Eye height
    FirstPersonCamera->bUsePawnControlRotation = true; // Camera follows controller rotation

    // Configure mesh to use Manny simple
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        // Load SKM_Manny_Simple mesh
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshAsset(
            TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
        if (MannyMeshAsset.Succeeded())
        {
            MeshComp->SetSkeletalMesh(MannyMeshAsset.Object);
            UE_LOG(LogFallenCompassCharacter, Log, TEXT("Loaded SKM_Manny_Simple mesh"));
        }
        else
        {
            UE_LOG(LogFallenCompassCharacter, Warning,
                TEXT("Failed to load SKM_Manny_Simple. Ensure Characters/Mannequins content is available."));
        }

            // Try to load a project-specific animation blueprint first (e.g. APC_Unarmed).
            // Replace the path below with your actual AnimBlueprint asset path if it differs.
			static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimBP(
				TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
            if (UnarmedAnimBP.Succeeded())
            {
                MeshComp->SetAnimInstanceClass(UnarmedAnimBP.Class);
				MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
                UE_LOG(LogFallenCompassCharacter, Log, TEXT("Loaded APB_Unarmed animation blueprint"));
            }
			else
			{
				UE_LOG(LogFallenCompassCharacter, Warning,
					TEXT("Failed to load APB_Unarmed. Ensure Characters/Mannequins content is available."));
			}

        // Position mesh relative to capsule (standard FPS setup - mesh at feet)
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Face forward

        // Hide mesh in first-person view (player doesn't see their body)
        MeshComp->SetOwnerNoSee(false);
    }

    // Enable tick for pitch clamping
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Load Enhanced Input actions
    static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionFinder(TEXT("/Game/FC/Input/IA_Move"));
    if (MoveActionFinder.Succeeded())
    {
        MoveAction = MoveActionFinder.Object;
        UE_LOG(LogFallenCompassCharacter, Log, TEXT("Loaded IA_Move"));
    }
    else
    {
        UE_LOG(LogFallenCompassCharacter, Warning, TEXT("Failed to load IA_Move"));
    }

    static ConstructorHelpers::FObjectFinder<UInputAction> LookActionFinder(TEXT("/Game/FC/Input/IA_Look"));
    if (LookActionFinder.Succeeded())
    {
        LookAction = LookActionFinder.Object;
        UE_LOG(LogFallenCompassCharacter, Log, TEXT("Loaded IA_Look"));
    }
    else
    {
        UE_LOG(LogFallenCompassCharacter, Warning, TEXT("Failed to load IA_Look"));
    }
}

void AFCFirstPersonCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogFallenCompassCharacter, Log, TEXT("AFCFirstPersonCharacter spawned | LookSensitivity=%.2f | PitchClamp=[%.1f, %.1f]"),
        LookSensitivity, MinPitchAngle, MaxPitchAngle);

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
        if (AnimInst)
        {
            UE_LOG(LogFallenCompassCharacter, Log, TEXT("AnimInstance present: %s"), *AnimInst->GetClass()->GetName());
        }
        else
        {
            UE_LOG(LogFallenCompassCharacter, Warning, TEXT("No AnimInstance assigned to mesh at BeginPlay."));
        }
    }
}

void AFCFirstPersonCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Continuously clamp camera pitch to prevent over-rotation
    ClampCameraPitch();
}

void AFCFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        UE_LOG(LogFallenCompassCharacter, Error, TEXT("EnhancedInputComponent missing! Check Project Settings -> Input."));
        return;
    }

    // Bind movement action (WASD)
    if (MoveAction)
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFCFirstPersonCharacter::HandleMove);
        UE_LOG(LogFallenCompassCharacter, Log, TEXT("Bound IA_Move to HandleMove"));
    }
    else
    {
        UE_LOG(LogFallenCompassCharacter, Warning, TEXT("MoveAction not assigned"));
    }

    // Bind look action (Mouse)
    if (LookAction)
    {
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFCFirstPersonCharacter::HandleLook);
        UE_LOG(LogFallenCompassCharacter, Log, TEXT("Bound IA_Look to HandleLook"));
    }
    else
    {
        UE_LOG(LogFallenCompassCharacter, Warning, TEXT("LookAction not assigned"));
    }
}

void AFCFirstPersonCharacter::HandleMove(const FInputActionValue& Value)
{
    // Get 2D movement vector from input
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        // Get forward and right vectors relative to controller yaw
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement input
        AddMovementInput(ForwardDirection, MovementVector.Y); // Y = forward/backward (W/S)
        AddMovementInput(RightDirection, MovementVector.X);   // X = right/left (D/A)
    }
}

void AFCFirstPersonCharacter::HandleLook(const FInputActionValue& Value)
{
    // Get 2D look vector from input
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        // Apply look sensitivity to mouse input
        AddControllerYawInput(LookAxisVector.X * LookSensitivity);   // Mouse X = yaw (turn left/right)
        AddControllerPitchInput(-LookAxisVector.Y * LookSensitivity); // Mouse Y = pitch (look up/down) - inverted

        // Note: Pitch clamping is handled in Tick() via ClampCameraPitch()
    }
}

void AFCFirstPersonCharacter::ClampCameraPitch()
{
    if (Controller)
    {
        FRotator ControlRotation = Controller->GetControlRotation();

        // Normalize pitch to [-180, 180] range
        float CurrentPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

        // Clamp pitch within configured bounds
        float ClampedPitch = FMath::Clamp(CurrentPitch, MinPitchAngle, MaxPitchAngle);

        // Only update if clamping occurred (avoid unnecessary setter calls)
        if (!FMath::IsNearlyEqual(CurrentPitch, ClampedPitch, 0.01f))
        {
            ControlRotation.Pitch = ClampedPitch;
            Controller->SetControlRotation(ControlRotation);
        }
    }
}
