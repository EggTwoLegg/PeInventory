#pragma once

#include "UObject/Interface.h"
#include "PeInventoryContainer.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class PEINVENTORY_API UPeInventoryContainer : public UInterface
{
	GENERATED_BODY()
};

class PEINVENTORY_API IPeInventoryContainer
{
	GENERATED_BODY()
	
public:
	virtual bool CanPlayerAccessInventory(const class APlayerController* PlayerController) const = 0;
	virtual class UPeSpatialInventory* GetInventory() = 0;
};
