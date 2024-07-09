#include "PeInventory/ItemDb.h"

#include "PeInventory/ItemStack.h"

void FItemDb::SetupItemStack(FItemStack& Stack) const
{
	FItemInfoBase ItemInfo;
	if(GetItemInfo(Stack.ItemId, Stack.ItemType, ItemInfo))
	{
		Stack.MaxAmount  = ItemInfo.MaxAmount;
		Stack.SlotWidth  = ItemInfo.SlotWidth;
		Stack.SlotHeight = ItemInfo.SlotHeight;
	}
}
