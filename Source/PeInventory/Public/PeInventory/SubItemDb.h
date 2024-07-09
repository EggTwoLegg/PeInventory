#pragma once

#include "Engine/DataTable.h"
#include "PeInventory/ItemInfoBase.h"

struct FSubItemDbBase
{
	virtual ~FSubItemDbBase() = default;
	
	uint8 ItemType;
	
	explicit FSubItemDbBase(const uint8 InItemType) : ItemType(InItemType) {}

	static FSubItemDbBase* Create(const uint8 ItemType, const TCHAR* DbPath, const TCHAR* ContextName)
	{
		FSubItemDbBase* Instance = new FSubItemDbBase(ItemType);
		
		UDataTable* LoadedTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, DbPath));
		
		check(LoadedTable);

		const int32 NumRows = LoadedTable->GetRowMap().Num();
		Instance->Items.Reserve(NumRows);
		Instance->NameToIndex.Reserve(NumRows);
		Instance->IndexToName.Reserve(NumRows);
		
		// Linearize the database such that we may access by index.
		LoadedTable->ForeachRow<FItemInfoBase>(ContextName, [Instance](const FName& Key, const FItemInfoBase& Value)
		{
			const uint32 IdxItem = Instance->Items.Num();
			Instance->NameToIndex.Add(Key, IdxItem);
			Instance->IndexToName.Add(IdxItem, Key);
			Instance->Items.Add(Value);
		});

		return Instance;
	}

	FItemInfoBase* GetItemInfo(uint32 Idx)
	{
		if (!Items.IsValidIndex(Idx)) { return nullptr; }

		return &Items[Idx];
	}

	bool GetItemInfo(uint32 Idx, FItemInfoBase& OutInfo)
	{
		if (!Items.IsValidIndex(Idx)) { return false; }

		OutInfo = Items[Idx];
		return true;
	}
	
	FItemInfoBase* GetItemInfo(const FName& ItemName)
	{
		const uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return nullptr; }

		return &Items[*Idx];
	}

	bool GetItemIdx(const FName& ItemName, uint32& OutItemInfo) const
	{
		const uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return false; }

		OutItemInfo = *Idx;
		return true;
	}

	FName GetItemName(const uint32 ItemIdx)
	{
		return IndexToName[ItemIdx];
	}
	
	bool GetItemInfoDerived(int32 Idx, FItemInfoBase& OutInfo)
	{
		if (!Items.IsValidIndex(Idx)) { return false; }

		OutInfo = Items[Idx];
		return true;
	}
	
	bool GetItemInfoDerived(const FName& ItemName, FItemInfoBase& OutInfo)
	{
		const uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return false; }

		OutInfo = (Items[*Idx]);
		return true;
	}
	
	FItemInfoBase* GetItemInfoDerived(uint32 Idx)
	{
		if (!Items.IsValidIndex(Idx)) { return nullptr; }

		return &Items[Idx];
	}
	
	FItemInfoBase* GetItemInfoDerived(const FName& ItemName)
	{
		const uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return nullptr; }

		return &Items[*Idx];
	}
	
	TArray<FItemInfoBase> Items;
	TMap<FName, uint32> NameToIndex;
	TMap<uint32, FName> IndexToName;
};
