#pragma once

#include "FCPlayerModeTypes.generated.h"

UENUM(BlueprintType)
enum class EFCPlayerMode : uint8
{
    Office     UMETA(DisplayName = "Office"),
    Overworld  UMETA(DisplayName = "Overworld"),
    Camp       UMETA(DisplayName = "Camp"),
    Static     UMETA(DisplayName = "Static"),
};
