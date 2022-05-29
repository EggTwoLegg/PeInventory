#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "PeItemHandler.h"
#include "PeNBT.h"
#include "Engine/NetSerialization.h"

#include "PeItemStack.generated.h"

class UPeSpatialInventory;
class UPeItemHandler;

USTRUCT()
struct PEINVENTORY_API FPeItemStack : public FFastArraySerializerItem
{
    GENERATED_BODY()
	
    FPeItemStack() : ItemId(GetEmptyStackId()), ItemType(0), Amount(0), bRotated(false), SlotX(0), SlotY(0), IdxStackArray(0), MaxAmount(0), SlotWidth(0), SlotHeight(0) {}

	FPeItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, bool bInRotated, UPeItemHandler* InHandler = nullptr) : ItemId(InId),
		ItemType(InItemType), Amount(InAmount), bRotated(bInRotated), SlotX(0), SlotY(0), IdxStackArray(0), MaxAmount(0), SlotWidth(0),
		SlotHeight(0), ItemHandler(InHandler) {}

	FPeItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, bool bInRotated, uint32 InSlotX, uint32 InSlotY, uint32 InIdxStackArray, UPeItemHandler* InHandler = nullptr) : ItemId(InId),
		ItemType(InItemType), Amount(InAmount), bRotated(bInRotated), SlotX(InSlotX), SlotY(InSlotY), IdxStackArray(InIdxStackArray), MaxAmount(0), SlotWidth(0),
		SlotHeight(0), ItemHandler(InHandler) {}
	
	FPeItemStack(uint32 InId, uint8 InItemType, uint32 InAmount, uint32 InMaxAmount, uint8 InSlotWidth, uint8 InSlotHeight, bool bInRotated,
		uint32 InSlotX, uint32 InSlotY, uint32 InIdxStackArray, UPeItemHandler* InHandler = nullptr) : ItemId(InId), ItemType(InItemType),
		Amount(InAmount), bRotated(bInRotated), SlotX(InSlotX), SlotY(InSlotY), IdxStackArray(InIdxStackArray), MaxAmount(InMaxAmount), SlotWidth(InSlotWidth), SlotHeight(InSlotHeight),
		ItemHandler(InHandler) {}

	FPeItemStack(const FPeItemStack& Other)
    {
    	ItemId		  = Other.ItemId;
    	ItemType      = Other.ItemType;
    	Amount	      = Other.Amount;
    	bRotated      = Other.bRotated;
    	MaxAmount     = Other.MaxAmount;
    	SlotWidth     = Other.SlotWidth;
    	SlotHeight    = Other.SlotHeight;
    	SlotX         = Other.SlotX;
    	SlotY         = Other.SlotY;
    	IdxStackArray = Other.IdxStackArray;
    	ItemHandler   = Other.ItemHandler;
    }

    /*
	 * The unique item Id.
	 * Used by clients to determine how to render the item.
	 * Used by the server to determine how to process the item.
	 */
    UPROPERTY()
    uint32 ItemId;

	/*
	 * The item type.
	 * Example usage is an index to a database array that you use the Id to query from individual item databases (for organization purposes).
	 */
	UPROPERTY()
	uint8 ItemType;

	/*
	 * Number of items that exist in this stack.
	 */
    UPROPERTY()
    uint32 Amount;

	/*
	 * Determines whether or not this ItemStack is rotated by 90 degrees.
	 */
	UPROPERTY()
	bool bRotated;

	/*
	 * The X index within the containing inventory's spatial grid.
	 */
	UPROPERTY()
	uint32 SlotX;

	/*
	 * The X index within the containing inventory's spatial grid.
	 */
	UPROPERTY()
	uint32 SlotY;

	/*
	 * The index within the containing inventory's ItemStack array.
	 */
	UPROPERTY()
	uint32 IdxStackArray;

	// Cached fields from the ItemDb to avoid excess lookups.
	// These aren't replicated, as the client can look them up using the replicated data above.

	/* The maximum amount that this stack can hold. */
	uint32 MaxAmount;

	/* The number of slots this stack takes up horizontally in spatial inventories. */
	uint8 SlotWidth;

	/* The number of slots this stack takes up vertically in spatial inventories. */
	uint8 SlotHeight;

	/*
	 * Handles the functionality of this item. It is a singleton, so it does not store unique state on that item.
	 * For storing unique item state, use the Meta array below.
	 */
	TWeakObjectPtr<class UPeItemHandler> ItemHandler;
	
	/*
	 * Named binary tags for arbitrary data on the item stack.
	 * Credit goes to Markus Persson (Notch) for the idea.
	 */
	TMap<uint32, FPeNBT> Meta;

	FORCEINLINE static int32 GetEmptyStackId() { return UINT32_MAX; }

	FString ToString() const;

	bool NetSerialize(FArchive& Archive, class UPackageMap* PackageMap, bool& bOutSuccess);
};

/*
 * In order to use this, we must:
 * - Call "MarkItemDirty" in order for changes to be reflected to clients connected to the host.
 * - Call "MarkArrayDirty" to signal to the replication system that this array should be swept for deltas.
 * - Within the GetLifetimeReplicatedProps of any implementing class, we must call the DOREPLIFETIME macro, passing in the name of the FPeItemStackArray property.
 * -- e.g: DOREPLIFETIME(SomeClass, ItemStackArrayName)
 */
USTRUCT()
struct PEINVENTORY_API FPeItemStackArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FPeItemStack> Items;

	UPROPERTY()
	UPeSpatialInventory* OwningInventory;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FPeItemStack, FPeItemStackArray>(Items, DeltaParams, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize) const;
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const;
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const;
};



// Metaprogramming: The compiler will generate code specializations ad-hoc as it finds them in the codebase. Faster than runtime evaluation.
template<>
struct TStructOpsTypeTraits<FPeItemStackArray> : public TStructOpsTypeTraitsBase2<FPeItemStackArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

template<>
struct TStructOpsTypeTraits<FPeItemStack> : public TStructOpsTypeTraitsBase2<FPeItemStack>
{
	enum
	{
		WithNetSerializer = true
	};
};