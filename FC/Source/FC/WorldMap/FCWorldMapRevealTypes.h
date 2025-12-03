// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FCWorldMapRevealTypes.generated.h"

USTRUCT(BlueprintType)
struct FC_API FFCWorldMapRevealRow : public FTableRowBase
{
	GENERATED_BODY()

	/** GridId (0..255) that should start revealed when bRevealed is true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridId = 0;

	/** Whether this cell starts revealed on a new game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRevealed = true;
};
