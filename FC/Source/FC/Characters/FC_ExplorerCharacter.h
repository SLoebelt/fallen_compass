#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FC_ExplorerCharacter.generated.h"

/**
 * Prototype explorer character used in Camp and other local scenes.
 * Represents the designated explorer (male or female) and supports
 * simple top-down point-and-click movement.
 */
UENUM(BlueprintType)
enum class EFCExplorerType : uint8
{
    Male    UMETA(DisplayName = "Male"),
    Female  UMETA(DisplayName = "Female")
};

class UFCInteractionComponent;

UCLASS()
class FC_API AFC_ExplorerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFC_ExplorerCharacter(const FObjectInitializer& ObjectInitializer);

    /** Current explorer type (male / female). Drives mesh selection in BP. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explorer")
    EFCExplorerType ExplorerType = EFCExplorerType::Male;

    /** Debug tick to monitor velocity */
    virtual void Tick(float DeltaTime) override;

	/** Starts pathfinding and movement toward the given world-space location (Camp/POI). */
    void MoveExplorerToLocation(const FVector& TargetLocation);

protected:
    virtual void BeginPlay() override;

	/** Path points obtained from NavMesh for the current move command. */
    UPROPERTY(VisibleInstanceOnly, Category = "Movement|Debug")
    TArray<FVector> PathPoints;

    /** Index of the current target point in PathPoints. */
    UPROPERTY(VisibleInstanceOnly, Category = "Movement|Debug")
    int32 CurrentPathIndex = INDEX_NONE;

    /** Whether the explorer is currently following a path. */
    UPROPERTY(VisibleInstanceOnly, Category = "Movement|Debug")
    bool bIsFollowingPath = false;

    /** Distance threshold to advance to the next path point. */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptRadius = 50.0f;

	/** Cached reference to the player's interaction component (for POI arrival notifications). */
    UPROPERTY()
    TWeakObjectPtr<UFCInteractionComponent> InteractionComponent;

    /** Handle capsule overlap with POIs (Camp arrival). */
    UFUNCTION()
    void OnCapsuleBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
};
