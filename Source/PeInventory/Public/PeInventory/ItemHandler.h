#pragma once

struct FItemStack;

enum class EItemHandlerOverrides {
    Loot,
    Use,
    Equip,
    Destroy,
    UseStackOnMe,
    Setup,
    Drop,
    StackCreated
};

// Utility template to check if an override is present.
template<EItemHandlerOverrides Override, EItemHandlerOverrides... Overrides>
struct IsOverridden;

template<EItemHandlerOverrides Override, EItemHandlerOverrides First, EItemHandlerOverrides... Rest>
struct IsOverridden<Override, First, Rest...>
    : std::conditional_t<Override == First, std::true_type, IsOverridden<Override, Rest...>> {};

template<EItemHandlerOverrides Override>
struct IsOverridden<Override> : std::false_type {};

struct FItemHandler
{
    using SetupFunc = void (*)();
    using UseFunc = bool (*)(AActor*, AActor*, FItemStack&, uint32);
    using UseStackOnMeFunc = bool (*)(FItemStack&, uint32);
    using LootFunc = bool (*)(AActor*, FItemStack&, uint32);
    using DropFunc = bool (*)(AActor*, FItemStack&, uint32);
    using DestroyFunc = bool (*)(AActor*, FItemStack&, uint32);
    using EquipFunc = bool (*)(AActor*, FItemStack&, uint32);
    using StackCreatedFunc = bool (*)(FItemStack&, uint32);
    using CanUseFunc = bool (*)(const AActor*, const AActor*, const FItemStack&, uint32);
    using CanUseStackOnMeFunc = bool (*)(const FItemStack&, uint32);
    using CanLootFunc = bool (*)(const AActor*, const FItemStack&, uint32);
    using CanDropFunc = bool (*)(const AActor*, const FItemStack&, uint32);
    using CanDestroyFunc = bool (*)(const AActor*, const FItemStack&, uint32);
    using CanEquipFunc = bool (*)(const AActor*, const FItemStack&, uint32);

    SetupFunc Setup = nullptr;
    UseFunc Use = nullptr;
    UseStackOnMeFunc UseStackOnMe = nullptr;
    LootFunc Loot = nullptr;
    DropFunc Drop = nullptr;
    DestroyFunc Destroy = nullptr;
    EquipFunc Equip = nullptr;
    StackCreatedFunc StackCreated = nullptr;
    CanUseFunc CanUse = nullptr;
    CanUseStackOnMeFunc CanStackBeUsedOnMe = nullptr;
    CanLootFunc CanLoot = nullptr;
    CanDropFunc CanDrop = nullptr;
    CanDestroyFunc CanDestroy = nullptr;
    CanEquipFunc CanEquip = nullptr;
};

// Base template for item handlers.
template<const char* ItemName, typename Derived, EItemHandlerOverrides... Overrides>
struct TItemHandler
{
    static FName GetItemName() {
        static const FName ItemNameFName(ItemName);
        return ItemNameFName;
    }

    static FItemHandler GetImplementedHandler()
    {
        FItemHandler Handler;
        
        Handler.Setup = &Derived::Setup;
        Handler.Use = &Derived::Use;
        Handler.UseStackOnMe = &Derived::UseStackOnMe;
        Handler.Loot = &Derived::Loot;
        Handler.Drop = &Derived::Drop;
        Handler.Destroy = &Derived::Destroy;
        Handler.Equip = &Derived::Equip;
        Handler.StackCreated = &Derived::StackCreated;
        Handler.CanUse = &Derived::CanUse;
        Handler.CanStackBeUsedOnMe = &Derived::CanStackBeUsedOnMe;
        Handler.CanLoot = &Derived::CanLoot;
        Handler.CanDrop = &Derived::CanDrop;
        Handler.CanDestroy = &Derived::CanDestroy;
        Handler.CanEquip = &Derived::CanEquip;

        return Handler;
    }

    static void Setup()
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Setup, Overrides...>::value) {
            Derived::SetupImpl();
        }
    }
    
    static bool CanUse(const AActor* User, const AActor* Target, const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Use, Overrides...>::value) {
            return Derived::CanUseImpl(User, Target, ItemStack, Context);
        }
        return false;
    }

    static bool CanStackBeUsedOnMe(const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::UseStackOnMe, Overrides...>::value) {
            return Derived::CanStackBeUsedOnMeImpl(ItemStack, Context);
        }
        return false;
    }

    static bool CanLoot(const AActor* Looter, const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Loot, Overrides...>::value) {
            return Derived::CanLootImpl(Looter, ItemStack, Context);
        }
        return false;
    }

    static bool CanDrop(const AActor* Dropper, const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Drop, Overrides...>::value) {
            return Derived::CanDropImpl(Dropper, ItemStack, Context);
        }
        return false;
    }

    static bool CanDestroy(const AActor* Destroyer, const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Destroy, Overrides...>::value) {
            return Derived::CanDestroyImpl(Destroyer, ItemStack, Context);
        }
        return false;
    }

    static bool CanEquip(const AActor* Equipper, const FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Equip, Overrides...>::value) {
            return Derived::CanEquipImpl(Equipper, ItemStack, Context);
        }
        return false;
    }

    static bool Use(AActor* User, AActor* Target, FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Use, Overrides...>::value) {
            return Derived::UseImpl(User, Target, ItemStack, Context);
        }
        return false;
    }

    static bool UseStackOnMe(FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::UseStackOnMe, Overrides...>::value) {
            return Derived::UseStackOnMeImpl(ItemStack, Context);
        }
        return false;
    }

    static bool Loot(AActor* Looter, FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Loot, Overrides...>::value) {
            return Derived::LootImpl(Looter, ItemStack, Context);
        }
        return false;
    }

    static bool Drop(AActor* Dropper, FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Drop, Overrides...>::value) {
            return Derived::DropImpl(Dropper, ItemStack, Context);
        }
        return false;
    }

    static bool Destroy(AActor* Destroyer, FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Destroy, Overrides...>::value) {
            return Derived::DestroyImpl(Destroyer, ItemStack, Context);
        }
        return false;
    }

    static bool Equip(AActor* Equipper, FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::Equip, Overrides...>::value) {
            return Derived::EquipImpl(Equipper, ItemStack, Context);
        }
        return false;
    }

    static bool StackCreated(FItemStack& ItemStack, const uint32 Context)
    {
        if constexpr (IsOverridden<EItemHandlerOverrides::StackCreated, Overrides...>::value) {
            return Derived::StackCreatedImpl(ItemStack, Context);
        }
        return false;
    }
};

struct FItemHandlerRegistry
{
    template<typename THandler>
    static void RegisterItemHandler(const FName& Name)
    {
        const TSharedPtr<FItemHandler> Handler = MakeShared<FItemHandler>(THandler::GetImplementedHandler());
        RegisterItemHandler(Name, Handler);
    }
    
    static void RegisterItemHandler(const FName& Name, const TSharedPtr<FItemHandler>& Handler)
    {
        Store.Add(Name, Handler);
    }
    
    static TSharedPtr<FItemHandler> GetItemHandler(const FName& Name)
    {
        TSharedPtr<FItemHandler>* Entry = Store.Find(Name);
        if (nullptr != Entry)
        {
            return *Entry;
        }
        return nullptr;
    }

private:
    static TMap<FName, TSharedPtr<FItemHandler>> Store;
};

TMap<FName, TSharedPtr<FItemHandler>> FItemHandlerRegistry::Store;
