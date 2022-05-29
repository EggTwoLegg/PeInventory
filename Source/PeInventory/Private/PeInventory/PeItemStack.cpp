#include "PeInventory/PeItemStack.h"
#include "PeInventory/PeSpatialInventory.h"
#include "Serialization/ArrayWriter.h"

FString FPeItemStack::ToString() const
{
	FString Result = FString::Printf(TEXT("ItemId: %d, ItemType: %d, Amount: %d, Rotated: %d, MaxAmount: %d, SlotWidth: %d, SlotHeight: %d, SlotX: %d, SlotY: %d, IdxStackArray: %d, Handler: %s"),
												ItemId, ItemType, Amount, bRotated, MaxAmount, SlotWidth, SlotHeight, SlotX, SlotY, IdxStackArray, ItemHandler.Get() ? *ItemHandler->GetName() : TEXT("No handler"));
	return Result;
}

bool FPeItemStack::NetSerialize(FArchive& Archive, UPackageMap* PackageMap, bool& bOutSuccess)
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
	Archive << Meta;

	bOutSuccess = true;
	return true;
}

void FPeItemStackArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksAdded(AddedIndices, FinalSize);
}

void FPeItemStackArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksRemoved(RemovedIndices, FinalSize);
}

void FPeItemStackArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const
{
	if(nullptr == OwningInventory) { return; }

	OwningInventory->NotifyItemStacksChanged(ChangedIndices, FinalSize);	
}
