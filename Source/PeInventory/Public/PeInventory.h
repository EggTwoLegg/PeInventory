#pragma once

#include "CoreMinimal.h"

class FPeInventory : public IModuleInterface
{
	virtual void StartupModule() override;

	void RegisterCommonTags();

	virtual bool IsGameModule() const override
	{
		return true;
	}
};
