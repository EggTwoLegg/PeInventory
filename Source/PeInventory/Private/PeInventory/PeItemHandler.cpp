#include "PeInventory/PeItemHandler.h"

void UPeItemHandler::Setup() {}

bool UPeItemHandler::OnUse(AActor* User, AActor* Target, FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::OnLoot(AActor* Looter, FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::OnDrop(AActor* Dropper, FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::OnDestroy(AActor* Destroyer, FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::OnEquip(AActor* Equipper, FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::OnStackCreated(FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::CanUse(const AActor* User, const AActor* Target, const FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::CanLoot(const AActor* Looter, const FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::CanDrop(const AActor* Dropper, const FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::CanDestroy(const AActor* Destroyer, const FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}

bool UPeItemHandler::CanEquip(const AActor* TargetActor, const FPeItemStack& ItemStack, const uint32 Context) const
{
	return false;
}


bool UPeItemHandler::Use(AActor* User, AActor* Target, FPeItemStack& ItemStack, const uint32 Context) const
{
	if(!CanUse(User, Target, ItemStack, Context)) { return false; }
	return OnUse(User, Target, ItemStack, Context);
}

bool UPeItemHandler::Loot(AActor* Looter, FPeItemStack& ItemStack, const uint32 Context) const
{
	if(!CanLoot(Looter, ItemStack, Context)) { return false; }
	return OnLoot(Looter, ItemStack, Context);
}

bool UPeItemHandler::Drop(AActor* Dropper, FPeItemStack& ItemStack, const uint32 Context) const
{
	if(!CanDrop(Dropper, ItemStack, Context)) { return false; }
	return OnDrop(Dropper, ItemStack, Context);
}

bool UPeItemHandler::Destroy(AActor* Destroyer, FPeItemStack& ItemStack, const uint32 Context) const
{
	return OnDestroy(Destroyer, ItemStack, Context);
}

bool UPeItemHandler::Equip(AActor* Equipper, FPeItemStack& ItemStack, const uint32 Context) const
{
	return OnEquip(Equipper, ItemStack, Context);
}

bool UPeItemHandler::StackCreated(FPeItemStack& ItemStack, const uint32 Context) const
{
	return OnStackCreated(ItemStack, Context);
}	
