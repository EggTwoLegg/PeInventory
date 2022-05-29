#pragma once

#include "CoreMinimal.h"
#include "PeSingleton.h"
#include "PeInventory/PeSubItemDb.h"
#include "PeInventory/PeItemStack.h"
#include "PeInventory/PeItemInfoBase.h"
#include "PeItemHandler.h"

struct FPeItemDb
{
	TSharedPtr<FPeSubItemDbBase> RegisterItemType(const uint8 ItemType, const TCHAR* DbPath, const TCHAR* ContextName)
	{
		checkf(!RegisteredItemTypes.Contains(ItemType), TEXT("FPeItemDb::RegisterItemType | Item Type %d already registered!"), ItemType);

		RegisteredItemTypes.Add(ItemType);
		
		FPeSubItemDbBase* NewSubDb = FPeSubItemDbBase::Create(ItemType, DbPath, ContextName);
		TSharedPtr<FPeSubItemDbBase> SharedSubDbPtr = MakeShareable(NewSubDb);
		SubDbs.Add(SharedSubDbPtr);
		
		return SharedSubDbPtr;
	}
	
	bool GetItemInfo(uint32 IdxItem, uint8 ItemType, FPeItemInfoBase& OutItemInfo) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return false; }

		FPeSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return false; }
	
		return DbBase->GetItemInfo(IdxItem, OutItemInfo);
	}

	FPeItemInfoBase* GetItemInfo(uint32 IdxItem, uint8 ItemType) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return nullptr; }

		FPeSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return nullptr; }
	
		return DbBase->GetItemInfo(IdxItem);
	}
	
	bool GetItemIdx(const FName& ItemName, uint8 ItemType, uint32& OutItemIdx) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return false; }

		FPeSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return nullptr; }
	
		return DbBase->GetItemIdx(ItemName, OutItemIdx);
	}

	class UPeItemHandler* GetItemHandler(const uint32 IdxItem, uint8 ItemType) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return nullptr; }

		FPeSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return nullptr; }

		return DbBase->GetItemHandler(IdxItem);
	}

	static FPeItemDb& Get()
	{
		static FPeItemDb Instance;
		return Instance;
	}

	void Reset()
	{
		SubDbs.Empty();
		RegisteredItemTypes.Empty();
	}

	bool CreateItemStack(const FName& ItemName, uint8 ItemType, uint32 Amount, FPeItemStack& Stack, const bool bRotated = false) const
	{
		uint32 IdxItem;
		if(!GetItemIdx(ItemName, ItemType, IdxItem)) { return false; }
    
        FPeItemInfoBase* ItemInfo = SubDbs[ItemType].Get()->GetItemInfo(IdxItem);
        if(nullptr == ItemInfo) { return false; }

        UPeItemHandler* Handler = GetItemHandler(IdxItem, ItemType);
        Stack = FPeItemStack(IdxItem, ItemType, Amount, ItemInfo->MaxAmount, ItemInfo->SlotWidth, ItemInfo->SlotHeight, bRotated, 0, 0, 0, Handler);

        return true;
	}
	
	bool CreateItemStack(uint32 IdxItem, uint8 ItemType, uint32 Amount, FPeItemStack& Stack, const bool bRotated = false) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return false; }
		
		FPeItemInfoBase* ItemInfo = SubDbs[ItemType].Get()->GetItemInfo(IdxItem);
		if(nullptr == ItemInfo) { return false; }
		
		UPeItemHandler* Handler = GetItemHandler(IdxItem, ItemType);
		Stack = FPeItemStack(IdxItem, ItemType, Amount, ItemInfo->MaxAmount, ItemInfo->SlotWidth, ItemInfo->SlotHeight, bRotated, 0, 0, 0, Handler);

		if(nullptr != Handler) { Handler->StackCreated(Stack, 0); }
		
		return true;
	}

	void SetupItemStack(FPeItemStack& Stack) const
	{
		FPeItemInfoBase ItemInfo;
		if(GetItemInfo(Stack.ItemId, Stack.ItemType, ItemInfo))
		{
			Stack.MaxAmount  = ItemInfo.MaxAmount;
			Stack.SlotWidth  = ItemInfo.SlotWidth;
			Stack.SlotHeight = ItemInfo.SlotHeight;
		}
	}

private:
	FPeItemDb() { Reset(); }
	
	TArray<TSharedPtr<FPeSubItemDbBase>> SubDbs;
	TSet<uint8> RegisteredItemTypes;
};
