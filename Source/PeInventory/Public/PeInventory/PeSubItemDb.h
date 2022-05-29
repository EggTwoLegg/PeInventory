#pragma once

#include "PeItemHandler.h"
#include "Engine/DataTable.h"
#include "PeInventory/PeItemInfoBase.h"

struct FPeSubItemDbBase
{
	virtual ~FPeSubItemDbBase() = default;
	
	uint8 ItemType;
	
	explicit FPeSubItemDbBase(const uint8 InItemType) : ItemType(InItemType) {}
	
	class UPeItemHandler* GetItemHandler(const uint32 IdxItem) const
	{
		if(!Handlers.IsValidIndex(IdxItem)) { return nullptr; }
		return Handlers[IdxItem];
	}

	static FPeSubItemDbBase* Create(const uint8 ItemType, const TCHAR* DbPath, const TCHAR* ContextName)
	{
		FPeSubItemDbBase* Instance = new FPeSubItemDbBase(ItemType);
		
		UDataTable* LoadedTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, DbPath));
		
		check(LoadedTable);

		const int32 NumRows = LoadedTable->GetRowMap().Num();
		Instance->Items.Reserve(NumRows);
		Instance->NameToIndex.Reserve(NumRows);
		Instance->Handlers.Reserve(NumRows);
		
		// Linearize the database such that we may access by index.
		LoadedTable->ForeachRow<FPeItemInfoBase>(ContextName, [Instance](const FName& Key, const FPeItemInfoBase& Value)
		{
			const uint32 IdxItem = Instance->Items.Num();
			Instance->NameToIndex.Add(Key, IdxItem);
			Instance->Items.Add(Value);

			// If we need to load a handler.
			// TODO: Make this async.
			UClass* HandlerClass = Value.Handler.LoadSynchronous(); // Loads the UClass, not the instance.
			if(nullptr != HandlerClass)
			{
				UPeItemHandler* TheHandler = GetSingletonUObject<UPeItemHandler>(HandlerClass);
				Instance->Handlers.Add(TheHandler);
			}
			else
			{
				Instance->Handlers.Add(nullptr);
			}
		});

		return Instance;
	}

	FPeItemInfoBase* GetItemInfo(uint32 Idx)
	{
		if (!Items.IsValidIndex(Idx)) { return nullptr; }

		return &Items[Idx];
	}

	bool GetItemInfo(uint32 Idx, FPeItemInfoBase& OutInfo)
	{
		if (!Items.IsValidIndex(Idx)) { return false; }

		OutInfo = Items[Idx];
		return true;
	}
	
	FPeItemInfoBase* GetItemInfo(const FName& ItemName)
	{
		uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return nullptr; }

		return static_cast<FPeItemInfoBase*>(&Items[*Idx]);
	}

	bool GetItemIdx(const FName& ItemName, uint32& OutItemInfo) const
	{
		const uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return false; }

		OutItemInfo = *Idx;
		return true;
	}
	
	bool GetItemInfoDerived(int32 Idx, FPeItemInfoBase& OutInfo)
	{
		if (!Items.IsValidIndex(Idx)) { return false; }

		OutInfo = Items[Idx];
		return true;
	}
	
	bool GetItemInfoDerived(const FName& ItemName, FPeItemInfoBase& OutInfo)
	{
		uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return false; }

		OutInfo = (Items[*Idx]);
		return true;
	}
	
	FPeItemInfoBase* GetItemInfoDerived(uint32 Idx)
	{
		if (!Items.IsValidIndex(Idx)) { return nullptr; }

		return &Items[Idx];
	}
	
	FPeItemInfoBase* GetItemInfoDerived(const FName& ItemName)
	{
		uint32* Idx = NameToIndex.Find(ItemName);
		if(nullptr == Idx) { return nullptr; }

		return &Items[*Idx];
	}

	TArray<class UPeItemHandler*> Handlers;
	TArray<FPeItemInfoBase> Items;
	TMap<FName, uint32> NameToIndex;
};
