#pragma once

#include "SubItemDb.h"

struct PEINVENTORY_API FItemDb
{
	TSharedPtr<FSubItemDbBase> RegisterItemType(const uint8 ItemType, const TCHAR* DbPath, const TCHAR* ContextName)
	{
		checkf(!RegisteredItemTypes.Contains(ItemType), TEXT("FPeItemDb::RegisterItemType | Item Type %d already registered!"), ItemType);

		RegisteredItemTypes.Add(ItemType);
		
		FSubItemDbBase* NewSubDb = FSubItemDbBase::Create(ItemType, DbPath, ContextName);
		TSharedPtr<FSubItemDbBase> SharedSubDbPtr = MakeShareable(NewSubDb);
		SubDbs.Add(SharedSubDbPtr);
		
		return SharedSubDbPtr;
	}
	
	bool GetItemInfo(uint32 IdxItem, uint8 ItemType, FItemInfoBase& OutItemInfo) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return false; }

		FSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return false; }
	
		return DbBase->GetItemInfo(IdxItem, OutItemInfo);
	}

	FItemInfoBase* GetItemInfo(uint32 IdxItem, uint8 ItemType) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return nullptr; }

		FSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return nullptr; }
	
		return DbBase->GetItemInfo(IdxItem);
	}
	
	bool GetItemIdx(const FName& ItemName, uint8 ItemType, uint32& OutItemIdx) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return false; }

		FSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return false; }
	
		return DbBase->GetItemIdx(ItemName, OutItemIdx);
	}

	FName GetItemName(const uint32 IdxItem, uint8 ItemType) const
	{
		if(!SubDbs.IsValidIndex(ItemType) || !SubDbs[ItemType].IsValid()) { return NAME_None; }

		FSubItemDbBase* DbBase = SubDbs[ItemType].Get();
		if(DbBase->ItemType != ItemType) { return NAME_None; }

		return DbBase->GetItemName(IdxItem);
	}

	static FItemDb& Get()
	{
		static FItemDb Instance;
		return Instance;
	}

	void Reset()
	{
		SubDbs.Empty();
		RegisteredItemTypes.Empty();
	}

	void SetupItemStack(struct FItemStack& Stack) const;

private:
	FItemDb() { Reset(); }
	
	TArray<TSharedPtr<FSubItemDbBase>> SubDbs;
	TSet<uint8> RegisteredItemTypes;
};
