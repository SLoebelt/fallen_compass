#include "GameModes/FCCampGameMode.h"
#include "Core/FCBaseGameMode.h"

#include "FCPlayerController.h"
#include "Characters/FC_ExplorerCharacter.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"


AFCCampGameMode::AFCCampGameMode()
{
	// If your explorer is always pre-placed, you *can* leave DefaultPawnClass null.
	// Keeping it set gives you a fallback if the level forgets to place one.
	DefaultPawnClass = AFC_ExplorerCharacter::StaticClass();
	PlayerControllerClass = AFCPlayerController::StaticClass();
}

void AFCCampGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Register camp camera (if provided)
	if (!CampCameraActor)
	{
		TArray<AActor*> Cameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("CampCamera"), Cameras);
		if (Cameras.Num() > 0) { CampCameraActor = Cast<ACameraActor>(Cameras[0]); }
		UE_LOG(LogTemp, Warning, TEXT("CampCameraActor not set -> resolved by tag CampCamera"));
	}

	if (CampCameraActor)
	{
		if (AFCPlayerController* PC = Cast<AFCPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->SetPOISceneCameraActor(CampCameraActor);
		}
	}

	// Safety net: if a pre-placed explorer exists and player has no pawn, possess it.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (!PC->GetPawn())
		{
			TArray<AActor*> FoundExplorers;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFC_ExplorerCharacter::StaticClass(), FoundExplorers);
			if (FoundExplorers.Num() > 0)
			{
				if (APawn* ExplorerPawn = Cast<APawn>(FoundExplorers[0]))
				{
					PC->Possess(ExplorerPawn);
				}
			}
		}
	}
}

/* APawn* AFCCampGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	// Camp/POI pattern: if an explorer is already placed, don't spawn another.
	TArray<AActor*> FoundExplorers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFC_ExplorerCharacter::StaticClass(), FoundExplorers);

	if (FoundExplorers.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("AFCCampGameMode: Explorer pre-placed -> skipping pawn spawn"));
		return nullptr;
	}

	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}
 */
