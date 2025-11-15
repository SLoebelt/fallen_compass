// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FCFirstPersonCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UFCInteractionComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassCharacter, Log, All);

/**
 * First-person character for office exploration.
 * Features camera at eye level, pitch clamping, and configurable look sensitivity.
 */
UCLASS()
class FC_API AFCFirstPersonCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFCFirstPersonCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /** Returns the first-person camera component */
    FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

    /** Returns the interaction component */
    FORCEINLINE UFCInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

    /** Get current camera look sensitivity multiplier */
    FORCEINLINE float GetLookSensitivity() const { return LookSensitivity; }

protected:
    /** First-person camera positioned at eye level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    /** Interaction component for detecting and interacting with objects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UFCInteractionComponent> InteractionComponent;

    /** Camera look sensitivity multiplier (applied to mouse input in Task 3.3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float LookSensitivity = 1.0f;

    /** Maximum pitch angle in degrees (prevents looking too far up/down) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    float MaxPitchAngle = 60.0f;

    /** Minimum pitch angle in degrees (negative value for looking down) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    float MinPitchAngle = -60.0f;

    // Enhanced Input Actions
    /** Input action for movement (WASD) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    /** Input action for looking (Mouse) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    /** Input action for interacting (E key) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    /** Optional AnimBlueprint to use for this character. Use Editor defaults to assign `ABP_Unarmed`. */
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    TSoftClassPtr<UAnimInstance> DefaultAnimBlueprint;

    // Input Handlers
    /** Handle movement input (WASD) */
    void HandleMove(const FInputActionValue& Value);

    /** Handle look input (Mouse) with sensitivity and pitch clamping */
    void HandleLook(const FInputActionValue& Value);

    /** Handle interact input (E key) */
    void HandleInteract();

private:
    /** Clamps camera pitch to configured min/max angles */
    void ClampCameraPitch();
};
