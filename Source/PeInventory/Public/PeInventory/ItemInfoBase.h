#pragma once

#include "Engine/DataTable.h"

#include "ItemInfoBase.generated.h"

USTRUCT()
struct PEINVENTORY_API FItemInfoBase : public FTableRowBase
{
	GENERATED_BODY()
	
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
};