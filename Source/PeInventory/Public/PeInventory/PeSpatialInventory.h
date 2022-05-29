#pragma once

#include "UObject/NoExportTypes.h"
#include "PeInventory/PeItemStack.h"

#include "PeSpatialInventory.generated.h"

USTRUCT()
struct PEINVENTORY_API FPeInventorySlot
{
	GENERATED_BODY()

	/*
	 * Index into the item stack array. If this is -1, this means no stack is allotted to this slot.
	 */
	UPROPERTY()
	int32 IdxStack;

	FPeInventorySlot() : IdxStack(-1) {}
};


DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackAdded,    const int32)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackRemoved,  const int32)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStackChanged,  const int32)

DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksAdded,   const TArrayView<int32>&)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksRemoved, const TArrayView<int32>&)
DECLARE_EVENT_OneParam(UInventoryBase, FInventoryItemStacksChanged, const TArrayView<int32>&)

struct FPeItemStackMoveResult
{
	uint32 NumFit;
	uint32 X;
	uint32 Y;
	struct FPeItemStack* SrcStack;
	struct FPeItemStack* DstStack;

	FPeItemStackMoveResult() : NumFit(0), X(0), Y(0), SrcStack(nullptr), DstStack(nullptr) {}
	FPeItemStackMoveResult(uint32 InNumFit, uint32 InX, uint32 InY, struct FPeItemStack* InSrcStack = nullptr, struct FPeItemStack* InDstStack = nullptr) :
		NumFit(InNumFit), X(InX), Y(InY), SrcStack(InSrcStack), DstStack(InDstStack) {}
};

UCLASS()
class PEINVENTORY_API UPeSpatialInventory : public UActorComponent
{
    GENERATED_BODY()

	friend struct FPeItemStackArray;

	typedef FPeItemStackMoveResult StackMoveResult;
	
protected:
	FInventoryItemStackAdded    ItemStackAddedEvent;
	FInventoryItemStackRemoved	ItemStackRemovedEvent;
	FInventoryItemStackChanged  ItemStackChangedEvent;
	
	FInventoryItemStacksAdded   ItemStacksAddedEvent;
	FInventoryItemStacksRemoved ItemStacksRemovedEvent;
	FInventoryItemStacksChanged ItemStacksChangedEvent;
	
	UPROPERTY(Replicated)
	uint32 NumSlotsX;

	UPROPERTY(Replicated)
	uint32 NumSlotsY;

	// Doesn't need to be replicated, as it's implicit based off the ItemStacks below.
	// This is an empty array when bIsSpatialInventory is false.
	TArray<FPeInventorySlot> Slots;
	
	UPROPERTY(Replicated)
	FPeItemStackArray ItemStacks;
	

	/*
	 * Maximum number of item stacks that can exist in this inventory.
	 * Only relevant for non-spatial inventories.
	 */
	UPROPERTY(Replicated, EditAnywhere)
	uint32 MaxStacks;

	UPROPERTY(Replicated, EditAnywhere)
	bool bIsSpatialInventory;
	
protected:
	void NotifyItemStacksAdded  (const TArrayView<int32>& AddedIndices,   int32 FinalSize) const;
	void NotifyItemStacksRemoved(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const;
	void NotifyItemStacksChanged(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const;
	
public:
	virtual void BeginPlay() override;
	
	FPeItemStack* GetStackAtSlot(uint32 X, uint32 Y);
	bool  GetStackIdxAtSlot(uint32 X, uint32 Y, int32& OutIdx) const;
    bool  GetStackByItemId(int32 ItemId, const struct FPeItemStack*& OutStack) const;
    void  EmptySlot(uint32 SlotX, uint32 SlotY);
	void  EmptySlots(uint32 StartX, uint32 EndX, uint32 StartY, uint32 EndY);
    void  EmptyAllSlots();
    StackMoveResult GetStackMoveResult(const FPeItemStack& Stack, uint32 SlotX, uint32 SlotY) const;
	static StackMoveResult GetStackMoveResult(const UPeSpatialInventory* SrcInventory, const UPeSpatialInventory* DstInventory, uint32 SrcX, uint32 SrcY, uint32 DstX, uint32 DstY);
    StackMoveResult AddItemStack(const FPeItemStack& Stack, uint32 SlotX, uint32 SlotY);
	void  AdjustItemCountOfStackAtSlot(uint32 Amount, uint32 SlotX, uint32 SlotY);
    void RemoveStackAt(uint32 IdxStack);

    void  PointSlotsToStack(const FPeItemStack& Stack, int32 StartSlotX, int32 StartSlotY, bool bEmpty = false);
	void  PointSlotsToStackIdx(const int32 IdxStack, int32 StartSlotX, int32 EndSlotX, int32 StartSlotY, int32 EndSlotY);
    FPeItemStackMoveResult MoveStack(UPeSpatialInventory* SrcInventory, uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY);
    StackMoveResult MoveStack(uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY);
	void  GetSlotsOverlappedByStackAtSlot(const FPeItemStack& Stack, uint32 IdxStartX, uint32 IdxStartY, TArray<uint32, TInlineAllocator<16>>& SlotIds) const;
    void SetCapacity(uint32 SizeX, uint32 SizeY);
    void PostReplicatedSetup();
	
	bool IsValidSlot(uint32 X, uint32 Y) const;
	bool HasStackAtSlot(int32 X, int32 Y) const;

	bool BinPack(const TArray<FPeItemStack>& Stacks);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	// Determines if the player (as represented by their controller) can access this inventory.
	// This might seem redundant, as we usually test inventory containers for access validaity,
	// but we have a very solid reason behind doing it this way: If the player receives a pointer to an inventory,
	// they might invoke network requests that would no longer be valid for the inventory's container.
	// We *could* make all actions abstracted to the container itself, but then we have a lot of indirection, as
	// opposed to simply checking against this method, which (by default) checks with its containing owner.
	bool CanPlayerAccess(const class APlayerController* PlayerController) const;

	void ForEachStack(TFunction<void(const FPeItemStack&)> Functor);

	
};