#include "PeInventory/SpatialInventory.h"

#include "GameFramework/PlayerController.h"
#include "PeInventory/2DBinPacker.h"
#include "PeInventory/ItemDb.h"
#include "PeInventory/ItemHandler.h"

// void FSpatialInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
//     DOREPLIFETIME(FSpatialInventory, ItemStacks);
//     DOREPLIFETIME(FSpatialInventory, NumSlotsX);
//     DOREPLIFETIME(FSpatialInventory, NumSlotsY);
// }

void FSpatialInventory::ForEachStack(const TFunction<void(const FItemStack&)>& Functor)
{
    for (const auto& ItemStack : ItemStacks.Items)
    {
        Functor(ItemStack);
    }
}

inline void FSpatialInventory::NotifyItemStacksAdded(const TArrayView<int32>& AddedIndices, int32 FinalSize) const
{
    ItemStacksAddedEvent.Broadcast(AddedIndices);
}

inline void FSpatialInventory::NotifyItemStacksRemoved(const TArrayView<int32>& RemovedIndices, int32 FinalSize) const
{
    ItemStacksRemovedEvent.Broadcast(RemovedIndices);
}

inline void FSpatialInventory::NotifyItemStacksChanged(const TArrayView<int32>& ChangedIndices, int32 FinalSize) const
{
    ItemStacksChangedEvent.Broadcast(ChangedIndices);
}

bool FSpatialInventory::IsValidSlot(uint32 X, uint32 Y) const
{
    const uint32 IdxSlot = Y * GetWidth() + X;
    return IdxSlot < GetWidth() * GetHeight();
}

bool FSpatialInventory::HasStackAtSlot(int32 X, int32 Y) const
{
    const int32 IdxSlot  = Y * GetWidth() + X;
    const int32 IdxStack = Slots[IdxSlot].IdxStack;

    return IdxStack >= 0;
}

bool FSpatialInventory::BinPack(const TArray<FItemStack>& Stacks)
{
    struct FPack
    {
        FItemStack Stack;
        uint32 X, Y;

        FPack(): X(0), Y(0)
        {
        }

        FPack(const FItemStack& InStack, uint32 InX, uint32 InY) : Stack(InStack), X(InX), Y(InY) {}
    };
    
    TArray<F2DBinPackNode> Nodes;
    Nodes.Reserve(Stacks.Num() / 2);

    TArray<FPack> Packed;
    Packed.Reserve(Stacks.Num());

    Nodes.Add(F2DBinPackNode(FIntRect(0, 0, GetWidth(), GetHeight())));
    
    for(const auto& Stack : Stacks)
    {
        FIntRect OutRect;
        if(F2DBinPackNode::Insert(Nodes, 0, FIntRect(0, 0, Stack.SlotWidth, Stack.SlotHeight), OutRect))
        {
            Packed.Emplace(Stack, OutRect.Min.X, OutRect.Min.Y);
        }
        else
        {
            return false;
        }
    }

    bool bSuccess = true;
    for(auto& Pack : Packed)
    {
        bSuccess &= AddItemStack(Pack.Stack, Pack.X, Pack.Y).NumFit > 0;
    }

    return bSuccess;
}

FItemStack* FSpatialInventory::GetStackAtSlot(uint32 X, uint32 Y)
{
    const uint32 IdxSlot = Y * GetWidth() + X;
    if(IdxSlot >= GetWidth() * GetHeight()) { return nullptr; }

    const int32 IdxStack = Slots[IdxSlot].IdxStack;
    if(IdxStack < 0 || IdxStack >= ItemStacks.Items.Num()) { return nullptr; }

    return &ItemStacks.Items[IdxStack];
}

bool FSpatialInventory::GetStackIdxAtSlot(uint32 X, uint32 Y, int32& OutIdxStack) const
{
    const uint32 IdxSlot = Y * GetWidth() + X;
    if(IdxSlot >= GetWidth() * GetHeight())
    {
        return false;
    }
    
    OutIdxStack = Slots[IdxSlot].IdxStack;
    return OutIdxStack >= 0;
}

bool FSpatialInventory::GetStackByItemId(int32 ItemId, const FItemStack*& OutStack) const
{
    OutStack = ItemStacks.Items.FindByPredicate([ItemId](const FItemStack& Stack)
    {
        return Stack.ItemId == ItemId;
    });

    return OutStack != nullptr;
}

void FSpatialInventory::EmptySlot(uint32 X, uint32 Y)
{
    const FItemStack* StackAtSlot = GetStackAtSlot(X, Y);
    if(nullptr != StackAtSlot)
    {
        PointSlotsToStack(*StackAtSlot, X, Y, true);
    }
}

void FSpatialInventory::EmptySlots(uint32 StartX, uint32 EndX, uint32 StartY, uint32 EndY)
{
    for(uint32 IdxY = StartY; IdxY < EndY; ++IdxY)
    for(uint32 IdxX = StartX; IdxX < EndX; ++IdxX)
    {
        EmptySlot(IdxX, IdxY);
    }
}

void FSpatialInventory::SetCapacity(uint32 SizeX, uint32 SizeY)
{
    NumSlotsX = SizeX;
    NumSlotsY = SizeY;

    const uint32 NumSlots = SizeX * SizeY;
    Slots.Reserve(SizeX * SizeY);

    for(uint32 IdxSlot = Slots.Num(); IdxSlot < NumSlots; ++IdxSlot)
    {
        Slots.Emplace();
    }
}

void FSpatialInventory::PostReplicatedSetup()
{
    SetCapacity(GetWidth(), GetHeight());

    for(auto& Stack : ItemStacks.Items)
    {
        // FItemDb::Get().SetupItemStack(Stack);
        PointSlotsToStack(Stack, Stack.SlotX, Stack.SlotY);
    }
}

FItemStackMoveResult FSpatialInventory::GetStackMoveResult(const FItemStack& InputStack, uint32 SlotX, uint32 SlotY) const
{
    // If a stack exists at our destination slot, see if we can early-out.
    FItemStack* ItemStackAtDestSlot = const_cast<FSpatialInventory*>(this)->GetStackAtSlot(SlotX, SlotY);
    if(nullptr != ItemStackAtDestSlot)
    {
        const int32 NumItemsInInputStack = InputStack.Amount;
        
        if(ItemStackAtDestSlot->ItemId == InputStack.ItemId && ItemStackAtDestSlot->ItemType == InputStack.ItemType)
        {
            const int32 Clearance = ItemStackAtDestSlot->MaxAmount - ItemStackAtDestSlot->Amount;
            return FItemStackMoveResult(FMath::Min(NumItemsInInputStack, Clearance), SlotX, SlotY,
                const_cast<FSpatialInventory*>(this)->GetStackAtSlot(InputStack.SlotX, InputStack.SlotY), ItemStackAtDestSlot);
        }
        
        // Different Id, therefore it's impossible to fit this item stack in.
        return FItemStackMoveResult(0, 0, 0);
    }

    // If we're here, there's no stack at the destination slot, but there might be stacks that span the bounds
    // of the input stack.

    const uint32 IdxSlot = SlotY * GetWidth() + SlotX;
    const int32 IdxInputStack = Slots[IdxSlot].IdxStack;
    
    // Handle rotations by swapping width and height of input item stack.
    int32 Width  = InputStack.SlotWidth;
    int32 Height = InputStack.SlotHeight;
    if(InputStack.bRotated) { Swap(Width, Height); }
    
    // Ensure we aren't trying to put this itemstack outside the bounds of the inventory.
    const uint32 EndX = SlotX + Width;
    const uint32 EndY = SlotY + Height;
    if(EndY > GetHeight() || EndX > GetWidth())
    {
        return FItemStackMoveResult(0, 0, 0);
    }

    // Starting at the root slot, iterate to find if the ItemStack fits in the space or if it can be consolidated into another stack.
    for(uint32 IdxY = SlotY; IdxY < EndY; ++IdxY)
    for(uint32 IdxX = SlotX; IdxX < EndX; ++IdxX)
    {
        // We can always fit into an empty slot.
        const FItemStack* StackAtSlot = const_cast<FSpatialInventory*>(this)->GetStackAtSlot(IdxX, IdxY);
        if(nullptr == StackAtSlot) { continue; }

        // If we've overlapped the same stack (meaning we've nudged this stack only a little), we can just early-out.
        // if(StackAtSlot->IdxStackArray == IdxInputStack) { return FPeItemStackMoveResult(ItemStacks.Items[IdxInputStack].Amount, SlotX, SlotY); }
        if(StackAtSlot->IdxStackArray == IdxInputStack) { continue; }
        
        // If any slots in the search don't match our input Stack's item id, there's no possibility to consolidate stacks.
        if(StackAtSlot->ItemId != InputStack.ItemId || StackAtSlot->ItemType != InputStack.ItemType)
        {
            return FItemStackMoveResult(0, 0, 0);
        }

        // At this point, we're not an empty slot and we have the same item id.
        const int32 MaxSlotted = FMath::Min(InputStack.Amount, StackAtSlot->MaxAmount - StackAtSlot->Amount);
        return FItemStackMoveResult(MaxSlotted, IdxX, IdxY);
    }

    // If we've reached here, we've not early-outed prior, which means all slots were empty. This means we can directly fit the entirety of the item stack
    // into the area spanned by its width and height.
    return FItemStackMoveResult(InputStack.Amount, SlotX, SlotY);
}

FItemStackMoveResult FSpatialInventory::GetStackMoveResult(
    const TSharedPtr<FSpatialInventory>& SrcInventory, const TSharedPtr<FSpatialInventory>& DstInventory, uint32 SrcX, uint32 SrcY,
    uint32 DstX, uint32 DstY)
{
    if (nullptr == SrcInventory || nullptr == DstInventory)
    {
        return FItemStackMoveResult();
    }

    const FItemStack* SrcStack = SrcInventory->GetStackAtSlot(SrcX, SrcY);
    if (nullptr == SrcStack)
    {
        return FItemStackMoveResult();
    }
    
    return DstInventory->GetStackMoveResult(*SrcStack, DstX, DstY);
}

FItemStackMoveResult FSpatialInventory::AddItemStack(const FItemStack& InputStack, uint32 SlotX, uint32 SlotY)
{
    const FItemStackMoveResult Res = GetStackMoveResult(InputStack, SlotX, SlotY);
    if(Res.NumFit < 1) { return Res; }
    
    const int32 IdxToSlot = Res.Y * GetWidth() + Res.X;
    const FInventorySlot& DestSlot = Slots[IdxToSlot];

    // Empty slot, create stack.
    if(DestSlot.IdxStack < 0)
    {
        const int32 NewStackIdx = ItemStacks.Items.Num();
        
        FItemStack& NewStack  = ItemStacks.Items.Add_GetRef(InputStack);
        NewStack.SlotX          = Res.X;
        NewStack.SlotY          = Res.Y;
        NewStack.IdxStackArray  = NewStackIdx;
        ItemStacks.MarkItemDirty(NewStack);

        PointSlotsToStack(NewStack, Res.X, Res.Y);
        
        ItemStackAddedEvent.Broadcast(NewStackIdx);

        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, NewStack.ToString());

        return Res;
    }
    
    // Existing stack at slot.
    FItemStack& ExistingStack  = ItemStacks.Items[DestSlot.IdxStack];
    ExistingStack.Amount        += Res.NumFit;
    ItemStacks.MarkItemDirty(ExistingStack);
    ItemStackChangedEvent.Broadcast(ExistingStack.IdxStackArray);
    
    // PointSlotsToStack(ExistingStack, Res.X, Res.Y);
    return Res;
}

void FSpatialInventory::AdjustItemCountOfStackAtSlot(uint32 Amount, uint32 SlotX, uint32 SlotY)
{
    FItemStack* Stack = GetStackAtSlot(SlotX, SlotY);
    if(nullptr != Stack)
    {
        Stack->Amount = Amount;
        if(Amount == 0)
        {
            EmptySlot(SlotX, SlotY);
            RemoveStackAt(Stack->IdxStackArray);
        }
        else
        {
            ItemStacks.MarkItemDirty(*Stack);
            ItemStackChangedEvent.Broadcast(Stack->IdxStackArray);
        }
    }
}

void FSpatialInventory::RemoveStackAt(const uint32 IdxStack)
{
    const uint32 NumStacks = ItemStacks.Items.Num();
    if(IdxStack >= NumStacks || NumStacks == 0) { return; }

    const uint32 IdxLastStack = NumStacks - 1;
    
    if(IdxStack != IdxLastStack)
    {
        // Swap with last stack only if we're not removing the last stack
        ItemStacks.Items.SwapMemory(IdxStack, IdxLastStack);

        // Make sure stack swapped from the end updates its IdxStackArray
        FItemStack& SwappedStack = ItemStacks.Items[IdxStack];
        SwappedStack.IdxStackArray = IdxStack;
        ItemStacks.MarkItemDirty(SwappedStack);
    }

    // Remove last item stack
    ItemStacks.Items.RemoveAt(IdxLastStack);
    ItemStacks.MarkArrayDirty();

    ItemStackRemovedEvent.Broadcast(IdxStack);
}

bool FSpatialInventory::CanStackBeUsedOnAnotherStack(uint32 SrcX, uint32 SrcY, uint32 DstX, uint32 DstY)
{
    const FItemStack* SrcStack = GetStackAtSlot(SrcX, SrcY);
    if (nullptr == SrcStack) { return false; }

    const FItemStack* DstStack = GetStackAtSlot(DstX, DstY);
    if (nullptr == DstStack) { return false; }

    const FName& ItemName = DstStack->ItemName;
    const TSharedPtr<FItemHandler> Handler = FItemHandlerRegistry::GetItemHandler(ItemName);
    if(nullptr == Handler) { return false; }

    const bool bCanBeUsedOn = Handler->CanStackBeUsedOnMe(*SrcStack, 0);
    return bCanBeUsedOn;
}


void FSpatialInventory::PointSlotsToStack(const FItemStack& Stack, int32 StartX, int32 StartY, bool bEmpty)
{
    const int32 EndX = StartX + Stack.SlotWidth;
    const int32 EndY = StartY + Stack.SlotHeight;
    
    PointSlotsToStackIdx(!bEmpty ? Stack.IdxStackArray : -1, StartX, EndX, StartY, EndY);
}

void FSpatialInventory::PointSlotsToStackIdx(const int32 IdxStack, int32 StartSlotX, int32 EndSlotX, int32 StartSlotY,
    int32 EndSlotY)
{
    for(int32 IdxY = StartSlotY; IdxY < EndSlotY; ++IdxY)
    for(int32 IdxX = StartSlotX; IdxX < EndSlotX; ++IdxX)
    {
        const int32 IdxSlot = IdxY * GetWidth() + IdxX;
        FInventorySlot& Slot = Slots[IdxSlot];
        Slot.IdxStack = IdxStack;
    }
}

FItemStackMoveResult FSpatialInventory::TransferItemStackToAnotherInventory(TSharedPtr<FSpatialInventory>& DstInventory, uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY)
{
    if(nullptr == DstInventory) { return FItemStackMoveResult(); }
    
    if(DstInventory.Get() == this) { return MoveStack(SrcX, SrcY, DestX, DestY); }

    const FItemStack* SrcStack = GetStackAtSlot(SrcX, SrcY);
    if(nullptr == SrcStack) { return FItemStackMoveResult(); }

    const FItemStackMoveResult AddResult = DstInventory->AddItemStack(*SrcStack, DestX, DestY);
    if(AddResult.NumFit > 0)
    {
        const uint32 NewAmount = SrcStack->Amount - FMath::Min(SrcStack->Amount, AddResult.NumFit);
        AdjustItemCountOfStackAtSlot(NewAmount, SrcX, SrcY);
    }

    return AddResult;
}

FItemStackMoveResult FSpatialInventory::MoveStack(uint32 SrcX, uint32 SrcY, uint32 DestX, uint32 DestY)
{
    FItemStack* SourceStack = GetStackAtSlot(SrcX, SrcY);
    if(nullptr == SourceStack)
    {
        return FItemStackMoveResult(0,0,0);
    }
    
    const FItemStackMoveResult MoveResult = GetStackMoveResult(*SourceStack, DestX, DestY);
    if(MoveResult.NumFit < 1) { return MoveResult; }

    // We know that we're going to update this stack below, either in the branch or beneath that, so flag this now.
    ItemStacks.MarkItemDirty(*SourceStack);
    
    // If there's a stack at the destination, we move as much of the source stack as we can to the destination.
    const FItemStack* DestStack = MoveResult.DstStack;
    if(nullptr != DestStack)
    {
        AdjustItemCountOfStackAtSlot(SourceStack->Amount - MoveResult.NumFit, SrcX, SrcY);
        AdjustItemCountOfStackAtSlot(DestStack->Amount + MoveResult.NumFit, DestX, DestY);
        
        return MoveResult;
    }

    // No stack at destination, and we've determined that we can move here.
    SourceStack->SlotX = DestX;
    SourceStack->SlotY = DestY;
    PointSlotsToStack(*SourceStack, DestX, DestY);
    PointSlotsToStack(*SourceStack, SrcX, SrcY, true); // Entire stack was moved, clear old location.

    ItemStackChangedEvent.Broadcast(SourceStack->IdxStackArray);
    
    return MoveResult;
}

void FSpatialInventory::GetSlotsOverlappedByStackAtSlot(const FItemStack& Stack, uint32 IdxStartX, uint32 IdxStartY, TArray<uint32, TInlineAllocator<16>>& SlotIds) const
{
    int32 Width  = Stack.SlotWidth;
    int32 Height = Stack.SlotHeight;

    if(Stack.bRotated) { Swap(Width, Height); }

    const uint32 EndX = FMath::Min(GetWidth(), IdxStartX + Width);
    const uint32 EndY = FMath::Min(GetHeight(), IdxStartY + Height);

    for(uint32 IdxY = IdxStartY; IdxY < EndY; ++IdxY)
    for(uint32 IdxX = IdxStartX; IdxX < EndX; ++IdxX)
    {
        const int32 IdxLoopSlot = IdxY * GetWidth() + IdxX;
        SlotIds.Add(IdxLoopSlot);
    }
}

void FSpatialInventory::EmptyAllSlots()
{
    const int32 Width  = GetWidth();
    const int32 Height = GetHeight();
    
    for(int32 IdxY = 0; IdxY < Height; ++IdxY)
    for(int32 IdxX = 0; IdxX < Width; ++IdxX)
    {
        const int32 IdxSlot = IdxY * Width + IdxX;
        FInventorySlot& Slot = Slots[IdxSlot];
        Slot.IdxStack = -1;
    }

    ItemStacks.Items.Reset();
    ItemStacks.MarkArrayDirty();
}