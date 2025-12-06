#include "GameModes/FCOverworldGameMode.h"
#include "Core/FCBaseGameMode.h"

#include "FCPlayerController.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"


AFCOverworldGameMode::AFCOverworldGameMode()
{
	// If convoy is always level-placed, pawn can be null.
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AFCPlayerController::StaticClass();
}

void AFCOverworldGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Fallback search if not wired explicitly
	if (!DefaultConvoy)
	{
		TArray<AActor*> FoundConvoys;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFCOverworldConvoy::StaticClass(), FoundConvoys);

		if (FoundConvoys.Num() > 0)
		{
			DefaultConvoy = Cast<AFCOverworldConvoy>(FoundConvoys[0]);
			UE_LOG(LogTemp, Warning, TEXT("AFCOverworldGameMode: DefaultConvoy not set -> using %s"), *DefaultConvoy->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AFCOverworldGameMode: No convoy found in level!"));
			return;
		}
	}

	if (AFCPlayerController* PC = Cast<AFCPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetActiveConvoy(DefaultConvoy);
	}

	if (!OverworldCameraActor)
	{
		TArray<AActor*> Cameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("OverworldCamera"), Cameras);
		if (Cameras.Num() > 0) { OverworldCameraActor = Cast<ACameraActor>(Cameras[0]); }
		UE_LOG(LogTemp, Warning, TEXT("OverworldCameraActor not set -> resolved by tag OverworldCamera"));
	}

}
