#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FC_ExplorerCharacter.generated.h"

/**
 * Prototype explorer character used in Camp and other local scenes.
 * Represents the designated explorer (male or female) and supports
 * simple top-down point-and-click movement.
 * 
 * Uses AI controller for pathfinding and smooth movement (similar to convoy members).
 */
UENUM(BlueprintType)
enum class EFCExplorerType : uint8
{
    Male    UMETA(DisplayName = "Male"),
    Female  UMETA(DisplayName = "Female")
};

UCLASS()
class FC_API AFC_ExplorerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFC_ExplorerCharacter(const FObjectInitializer& ObjectInitializer);

    /** Current explorer type (male / female). Drives mesh selection in BP. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explorer")
    EFCExplorerType ExplorerType = EFCExplorerType::Male;

    /** Get current movement speed (for Animation Blueprint debugging) */
    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetMovementSpeed() const;

    /** Is character currently moving? (for Animation Blueprint debugging) */
    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsCharacterMoving() const;

    /** Debug tick to monitor velocity */
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    /** Initialize animation system (ensure AnimBP is active) */
    void InitializeAnimation();
};
