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

public:
	/** Set parent convoy reference */
	void SetParentConvoy(AFCOverworldConvoy* InConvoy);
};
