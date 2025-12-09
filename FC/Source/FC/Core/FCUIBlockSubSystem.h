#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FCUIBlockSubsystem.generated.h"

class UUserWidget;

USTRUCT(BlueprintType)
struct FFCWorldInputBlockState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool bBlocksWorldClick = false;
    UPROPERTY(BlueprintReadOnly) bool bBlocksWorldInteract = false;
	UPROPERTY(BlueprintReadOnly) TArray<FString> BlockingReasons;

	FFCWorldInputBlockState() = default;

#if WITH_EDITORONLY_DATA
    // Editor-only data is allowed to be wrapped
    UPROPERTY(EditAnywhere, Category="Clock|Editor")
    float DebugSpeedMultiplier = 1.0f;
#endif
};

UCLASS()
class FC_API UFCUIBlockSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void RegisterBlocker(UUserWidget* Widget, bool bBlocksClick, bool bBlocksInteract);
    void UnregisterBlocker(UUserWidget* Widget);

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    bool CanWorldClick() const { return !bBlocksWorldClickCached; }

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    bool CanWorldInteract() const { return !bBlocksWorldInteractCached; }

    UFUNCTION(BlueprintCallable, Category="UI Blocking")
    FFCWorldInputBlockState GetWorldInputBlockState() const;

private:
    struct FBlockerFlags { bool bClick=false; bool bInteract=false; };

    TMap<TWeakObjectPtr<UUserWidget>, FBlockerFlags> ActiveBlockers;

    bool bBlocksWorldClickCached = false;
    bool bBlocksWorldInteractCached = false;

#if !UE_BUILD_SHIPPING
    TArray<FString> BlockingReasonsCached;
#endif

    void UpdateCache();
    void PruneInvalid();
};
