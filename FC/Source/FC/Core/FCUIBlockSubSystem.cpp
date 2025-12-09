#include "FCUIBlockSubsystem.h"
#include "Blueprint/UserWidget.h"

void UFCUIBlockSubsystem::RegisterBlocker(UUserWidget* Widget, bool bBlocksClick, bool bBlocksInteract)
{
    if (!Widget) return;

    ActiveBlockers.Add(Widget, FBlockerFlags{bBlocksClick, bBlocksInteract});
    UpdateCache();
}

void UFCUIBlockSubsystem::UnregisterBlocker(UUserWidget* Widget)
{
    if (!Widget) return;

    ActiveBlockers.Remove(Widget);
    UpdateCache();
}

void UFCUIBlockSubsystem::PruneInvalid()
{
    for (auto It = ActiveBlockers.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void UFCUIBlockSubsystem::UpdateCache()
{
    PruneInvalid();

    bBlocksWorldClickCached = false;
    bBlocksWorldInteractCached = false;

#if !UE_BUILD_SHIPPING
    BlockingReasonsCached.Reset();
#endif

    for (const auto& Pair : ActiveBlockers)
    {
        const TWeakObjectPtr<UUserWidget> Widget = Pair.Key;
        const FBlockerFlags Flags = Pair.Value;

        if (!Widget.IsValid()) continue;

        if (Flags.bClick)
        {
            bBlocksWorldClickCached = true;
#if !UE_BUILD_SHIPPING
            BlockingReasonsCached.Add(FString::Printf(TEXT("%s blocks click"), *Widget->GetName()));
#endif
        }

        if (Flags.bInteract)
        {
            bBlocksWorldInteractCached = true;
#if !UE_BUILD_SHIPPING
            BlockingReasonsCached.Add(FString::Printf(TEXT("%s blocks interact"), *Widget->GetName()));
#endif
        }
    }
}

FFCWorldInputBlockState UFCUIBlockSubsystem::GetWorldInputBlockState() const
{
    FFCWorldInputBlockState State;
    State.bBlocksWorldClick = bBlocksWorldClickCached;
    State.bBlocksWorldInteract = bBlocksWorldInteractCached;
#if !UE_BUILD_SHIPPING
    State.BlockingReasons = BlockingReasonsCached;
#endif
    return State;
}
