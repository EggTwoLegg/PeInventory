#include "PeInventory/Public/PeInventory/ItemStack.h"

#include "PeInventory/ItemDb.h"
#include "PeInventory/Public/PeInventory/SpatialInventory.h"

FItemStack::FItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, bool bInRotated): ItemId(InId),
	ItemType(InItemType), Amount(InAmount), bRotated(bInRotated), SlotX(0), SlotY(0), IdxStackArray(0), MaxAmount(0), SlotWidth(0),
	SlotHeight(0)
{
	ItemName = FItemDb::Get().GetItemName(InId, InItemType);
}

FItemStack::FItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, bool bInRotated, uint32 InSlotX, uint32 InSlotY,
	uint32 InIdxStackArray): ItemId(InId),
	                         ItemType(InItemType), Amount(InAmount), bRotated(bInRotated), SlotX(InSlotX), SlotY(InSlotY), IdxStackArray(InIdxStackArray), MaxAmount(0), SlotWidth(0),
	                         SlotHeight(0)
{
	ItemName = FItemDb::Get().GetItemName(InId, InItemType);
}

FItemStack::FItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, uint32 InMaxAmount, uint8 InSlotWidth,
	uint8 InSlotHeight, bool bInRotated, uint32 InSlotX, uint32 InSlotY, uint32 InIdxStackArray): ItemId(InId), ItemType(InItemType),
	Amount(InAmount), bRotated(bInRotated), SlotX(InSlotX), SlotY(InSlotY), IdxStackArray(InIdxStackArray), MaxAmount(InMaxAmount), SlotWidth(InSlotWidth), SlotHeight(InSlotHeight)
{
	ItemName = FItemDb::Get().GetItemName(InId, InItemType);
}

FString FItemStack::ToString() const
{
	FString Result = FString::Printf(TEXT("ItemId: %d, ItemType: %d, Amount: %d, Rotated: %d, MaxAmount: %d, SlotWidth: %d, SlotHeight: %d, SlotX: %d, SlotY: %d, IdxStackArray: %d, Handler: %s"),
												ItemId, ItemType, Amount, bRotated, MaxAmount, SlotWidth, SlotHeight, SlotX, SlotY, IdxStackArray);
	return Result;
}

bool FItemStack::NetSerialize(FArchive& Archive, UPackageMap* PackageMap, bool& bOutSuccess)
{
	Archive.SerializeIntPacked(ItemId);
	Archive << ItemType;
	Archive.SerializeIntPacked(Amount);
	Archive << bRotated;
	Archive.SerializeIntPacked(MaxAmount);
	Archive << SlotWidth;
	Archive << SlotHeight;
	Archive << SlotX;
	Archive << SlotY;
	Archive.SerializeIntPacked(IdxStackArray);

	bOutSuccess = true;
	return true;
}

void FItemStackArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksAdded(AddedIndices, FinalSize);
}

void FItemStackArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksRemoved(RemovedIndices, FinalSize);
}

void FItemStackArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksChanged(ChangedIndices, FinalSize);	
}
