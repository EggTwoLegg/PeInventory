#pragma once

#include "UObject/Interface.h"
#include "InventoryContainer.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class PEINVENTORY_API UInventoryContainer : public UInterface
{
	GENERATED_BODY()
};

class PEINVENTORY_API IInventoryContainer
{
	GENERATED_BODY()
	
public:
	virtual bool CanPlayerAccessInventory(const class APlayerController* PlayerController) const = 0;
	virtual struct FSpatialInventory* GetInventory() = 0;
};
