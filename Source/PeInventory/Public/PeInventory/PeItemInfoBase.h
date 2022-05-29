#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "PeItemInfoBase.generated.h"

USTRUCT()
struct PEINVENTORY_API FPeItemInfoBase : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FText DisplayName;

	UPROPERTY(EditAnywhere)
	FText Description;

	UPROPERTY(EditAnywhere)
	uint8 SlotWidth;

	UPROPERTY(EditAnywhere)
	uint8 SlotHeight;

	UPROPERTY(EditAnywhere)
	uint32 MaxAmount;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture> Icon;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<class UPeItemHandler> Handler;
};