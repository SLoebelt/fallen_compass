// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FCConvoyMember.generated.h"

class AFCOverworldConvoy;

/**
 * AFCConvoyMember - Base class for convoy member characters
 *
 * Handles capsule collision, POI overlap detection, and parent convoy communication.
 * Blueprint children configure mesh, materials, and CharacterMovement parameters.
 */
UCLASS()
class FC_API AFCConvoyMember : public ACharacter
{
	GENERATED_BODY()

public:
	AFCConvoyMember();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** Reference to parent convoy actor */
	UPROPERTY()
	AFCOverworldConvoy* ParentConvoy;

	/** Handle capsule overlap events */
	UFUNCTION()
	void OnCapsuleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	TArray<FVector> PathPoints;
	int32 CurrentPathIndex = INDEX_NONE;
	bool bIsFollowingPath = false;

	// NEW: leader-following state (used in .cpp)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
    bool bIsFollowingLeader = false;

    UPROPERTY()
    TWeakObjectPtr<AFCConvoyMember> Leader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
    FVector FollowerOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Pathfinding", meta = (AllowPrivateAccess = "true"))
    float AcceptRadius = 50.0f;

    FVector FinalTarget = FVector::ZeroVector;

public:
	/** Set parent convoy reference */
	void SetParentConvoy(AFCOverworldConvoy* InConvoy);

	UFUNCTION(BlueprintCallable, Category="FC|Convoy")
    void MoveConvoyMemberToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category="FC|Convoy")
    void StopConvoyMovement();

	UFUNCTION(BlueprintCallable, Category="FC|Convoy")
	void StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset);

	UFUNCTION(BlueprintCallable, Category="FC|Convoy")
	void StopFollowingLeader();
};
