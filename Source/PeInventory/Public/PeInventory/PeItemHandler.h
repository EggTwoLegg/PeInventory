#pragma once

#include "PeNBT.h"
#include "UObject/Object.h"
#include "PeItemHandler.generated.h"

struct FPeItemStack;

UCLASS(Abstract, Blueprintable, BlueprintType)
class PEINVENTORY_API UPeItemHandler : public UObject
{
	GENERATED_BODY()
	
protected:
	TMap<FName, FPeNBT> Properties;
	
protected:
	virtual void Setup();
	
	virtual bool OnUse(AActor* User, AActor* Target, FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool OnLoot(AActor* Looter, FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool OnDrop(AActor* Dropper, FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool OnDestroy(AActor* Destroyer, FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool OnEquip(AActor* Equipper, FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool OnStackCreated(FPeItemStack& ItemStack, const uint32 Context) const;
	
public:
	virtual bool CanUse(const AActor* User, const AActor* Target, const FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool CanLoot(const AActor* Looter, const FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool CanDrop(const AActor* Dropper, const FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool CanDestroy(const AActor* Destroyer, const FPeItemStack& ItemStack, const uint32 Context) const;
	virtual bool CanEquip(const AActor* Equipper, const FPeItemStack& ItemStack, const uint32 Context) const;

	bool Use(AActor* User, AActor* Target, FPeItemStack& ItemStack, const uint32 Context) const;
	bool Loot(AActor* Looter, FPeItemStack& ItemStack, const uint32 Context) const;
	bool Drop(AActor* Dropper, FPeItemStack& ItemStack, const uint32 Context) const;
	bool Destroy(AActor* Destroyer, FPeItemStack& ItemStack, const uint32 Context) const;
	bool Equip(AActor* Equipper, FPeItemStack& ItemStack, const uint32 Context) const;
	bool StackCreated(FPeItemStack& ItemStack, const uint32 Context) const;

	const TMap<FName, FPeNBT>& GetProperties() const { return Properties; }
};
