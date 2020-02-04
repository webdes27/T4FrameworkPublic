// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseEquipment.h"

/**
  * #68, #107
 */
class UT4StaticMeshComponent;
class FT4StaticMeshEquipment : public FT4BaseEquipment
{
public:
	FT4StaticMeshEquipment(AT4GameObject* InOwnerObject, const FT4ActionKey& InActionKey);
	~FT4StaticMeshEquipment();

	bool IsWeapon() const override { return true; }

protected:
	void Reset() override;
	void Advance(const FT4UpdateTime& InUpdateTime) override;
	void AttackPrepare(const FT4EntityKey& InEntityKey, FName InOverrideEquipPoint) override;
	void DetachPrepare() override;
	void StartLoading() override;

private:
	void AdvanceLoading(const FT4UpdateTime& InUpdateTime);
	void AdvanceLoadComplete(const FT4UpdateTime& InUpdateTime);
	void AdvanceTryAttach(const FT4UpdateTime& InUpdateTime);

private:
	ET4EquipLoadState LoadState;
	FT4StaticMeshLoader MeshLoader;

	TWeakObjectPtr<UT4StaticMeshComponent> ComponentPtr;
};
