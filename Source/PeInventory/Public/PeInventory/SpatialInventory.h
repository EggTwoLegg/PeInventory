#pragma once

#include "PeInventory/ItemStack.h"

#include "SpatialInventory.generated.h"

USTRUCT()
struct PEINVENTORY_API FInventorySlot
{
	GENERATED_BODY()

	/*
	 * Index into the item stack array. If this is -1, this means no stack is allotted to this slot.
	 */
	UPROPERTY()
	int32 IdxStack;

	FInventorySlot() : IdxStack(-1) {}
};

DECLARE_MULTICAST_DELEGATE(FSpatialInventoryDeleted)

DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackAdded,    const int32)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackRemoved,  const int32)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackChanged,  const int32)

DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksAdded,   const TArrayView<int32>&)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksRemoved, const TArrayView<int32>&)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksChanged, const TArrayView<int32>&)

struct FItemStackMoveResult
{
	FItemStack* SrcStack;
	FItemStack* DstStack;
	bool   bUsedOn;
	uint32 NumFit;
	uint32 X;
	uint32 Y;

	FItemStackMoveResult() : SrcStack(nullptr), DstStack(nullptr), bUsedOn(false), NumFit(0), X(0), Y(0) {}
	FItemStackMoveResult(uint32 InNumFit, uint32 InX, uint32 InY, FItemStack* InSrcStack = nullptr, FItemStack* InDstStack = nullptr) :
		 SrcStack(InSrcStack), DstStack(InDstStack), bUsedOn(false), NumFit(InNumFit), X(InX), Y(InY) {}
};

USTRUCT()
struct PEINVENTORY_API FSpatialInventory
{
    GENERATED_BODY()

	friend struct FItemStackArray;

	~FSpatialInventory()
	{
		InventoryDeletedEvent.Broadcast();
	}
	
protected:
	FSpatialInventoryDeleted InventoryDeletedEvent;
	
	FInventoryItemStackAdded    ItemStackAddedEvent;
	FInventoryItemStackRemoved	ItemStackRemovedEvent;
	FInventoryItemStackChanged  ItemStackChangedEvent;
	
	FInventoryItemStacksAdded   ItemStacksAddedEvent;
	FInventoryItemStacksRemoved ItemStacksRemovedEvent;
	FInventoryItemStacksChanged ItemStacksChangedEvent;
	
	UPROPERTY()
	uint32 NumSlotsX;

	UPROPERTY()
	uint32 NumSlotsY;

	// Doesn't need to be replicated, as it's implicit based off the ItemStacks below.
	// This is an empty array when bIsSpatialInventory is false.
	TArray<FInventorySlot> Slots;
	
	UPROPERTY()
	FItemStackArray ItemStacks;
	
	/*
	 * Maximum number of item stacks that can exist in this inventory.
	 * Only relevant for non-spatial inventories.
	 */
	UPROPERTY(EditAnywhere)
	uint32 MaxStacks;

	UPROPERTY(EditAnywhere)
	bool bIsSpatialInventory;
	
protected:
	void NotifyItemStacksAdded  (const TArrayView<int32>& AddedIndices,   int32 FinalSize) const;
	void NotifyItemStacksRemoved(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const;
	void NotifyItemStacksChanged(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const;
	
public:
	FItemStack* GetStackAtSlot(uint32 X, uint32 Y);
	bool  GetStackIdxAtSlot(uint32 X, uint32 Y, int32& OutIdx) const;
    bool  GetStackByItemId(int32 ItemId, const struct FItemStack*& OutStack) const;
    void  EmptySlot(uint32 SlotX, uint32 SlotY);
	void  EmptySlots(uint32 StartX, uint32 EndX, uint32 StartY, uint32 EndY);
    void  EmptyAllSlots();
    FItemStackMoveResult GetStackMoveResult(const FItemStack& Stack, uint32 SlotX, uint32 SlotY) const;
	static FItemStackMoveResult GetStackMoveResult(const TSharedPtr<FSpatialInventory>& SrcInventory, const TSharedPtr<FSpatialInventory>& DstInventory, uint32 SrcX, uint32 SrcY, uint32 DstX, uint32 DstY);
    FItemStackMoveResult AddItemStack(const FItemStack& Stack, uint32 SlotX, uint32 SlotY);
	void AdjustItemCountOfStackAtSlot(uint32 Amount, uint32 SlotX, uint32 SlotY);
    void RemoveStackAt(uint32 IdxStack);
	bool CanStackBeUsedOnAnotherStack(uint32 SrcX, uint32 SrcY, uint32 DstX, uint32 DstY);

    void  PointSlotsToStack(const FItemStack& Stack, int32 StartSlotX, int32 StartSlotY, bool bEmpty = false);
	void  PointSlotsToStackIdx(const int32 IdxStack, int32 StartSlotX, int32 EndSlotX, int32 StartSlotY, int32 EndSlotY);
    FItemStackMoveResult TransferItemStackToAnotherInventory(TSharedPtr<FSpatialInventory>& DstInventory, uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY);
    FItemStackMoveResult MoveStack(uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY);
	void  GetSlotsOverlappedByStackAtSlot(const FItemStack& Stack, uint32 IdxStartX, uint32 IdxStartY, TArray<uint32, TInlineAllocator<16>>& SlotIds) const;
    void SetCapacity(uint32 SizeX, uint32 SizeY);
    void PostReplicatedSetup();
	
	bool IsValidSlot(uint32 X, uint32 Y) const;
	bool HasStackAtSlot(int32 X, int32 Y) const;

	bool BinPack(const TArray<FItemStack>& Stacks);

	FORCEINLINE FInventoryItemStackAdded&	   OnItemStackAdded()    { return ItemStackAddedEvent; }
	FORCEINLINE FInventoryItemStackRemoved&    OnItemStackRemoved()  { return ItemStackRemovedEvent; }
	FORCEINLINE FInventoryItemStackChanged&    OnItemStackChanged()  { return ItemStackChangedEvent; }
	
	FORCEINLINE FInventoryItemStacksAdded&	   OnItemStacksAdded()   { return ItemStacksAddedEvent; }
	FORCEINLINE FInventoryItemStacksRemoved&   OnItemStacksRemoved() { return ItemStacksRemovedEvent; }
	FORCEINLINE FInventoryItemStacksChanged&   OnItemStacksChanged() { return ItemStacksChangedEvent; }
	
	FORCEINLINE uint32 GetWidth()         const { return NumSlotsX; }
	FORCEINLINE uint32 GetHeight()        const { return NumSlotsY; }
	FORCEINLINE uint32 GetMaxSlots()      const { return GetWidth() * GetHeight(); } 
	FORCEINLINE uint32 GetNumStacks()     const { return ItemStacks.Items.Num(); }
	FORCEINLINE uint32 GetMaxStacks()     const { return MaxStacks; }
	FORCEINLINE bool IsSpatialInventory() const { return bIsSpatialInventory; }

	void ForEachStack(const TFunction<void(const FItemStack&)>& Functor);
};