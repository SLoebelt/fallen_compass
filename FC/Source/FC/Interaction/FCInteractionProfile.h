#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "FCInteractionProfile.generated.h"

UENUM(BlueprintType)
enum class EFCProbeType : uint8
{
    LineTrace,
    SphereTrace,
    CursorHit,
    OverlapOnly
};

UCLASS(BlueprintType)
class FC_API UFCInteractionProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    EFCProbeType ProbeType = EFCProbeType::LineTrace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    float Range = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Probe")
    float Radius = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UUserWidget> PromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Filter")
    TArray<FName> AllowedTags;

    bool IsValidProfile() const { return Range > 0.f && Radius >= 0.f; }
};
