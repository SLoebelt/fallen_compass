#include "GameModes/FCOfficeGameMode.h"
#include "Core/FCBaseGameMode.h"

#include "FCFirstPersonCharacter.h"
#include "FCPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"


AFCOfficeGameMode::AFCOfficeGameMode()
{
	DefaultPawnClass = AFCFirstPersonCharacter::StaticClass();
	PlayerControllerClass = AFCPlayerController::StaticClass();
}

void AFCOfficeGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Keep this light for now (wiring later as you design it).
	// Example: you could register the OfficeCameraActor somewhere if needed.
	if (!MenuCameraActor)
	{
		TArray<AActor*> Cameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("MenuCamera"), Cameras);
		if (Cameras.Num() > 0) { MenuCameraActor = Cast<ACameraActor>(Cameras[0]); }
		UE_LOG(LogTemp, Warning, TEXT("MenuCameraActor not set -> resolved by tag MenuCamera"));
	}

}
