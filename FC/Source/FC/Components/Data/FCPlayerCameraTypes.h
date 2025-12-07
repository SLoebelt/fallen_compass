#pragma once

#include "CoreMinimal.h"
#include "FCPlayerCameraTypes.generated.h"

UENUM(BlueprintType)
enum class EFCPlayerCameraMode : uint8
{
	FirstPerson = 0,
	TableView,
	MainMenu,
	SaveSlotView,
	TopDown,
	POIScene
};
