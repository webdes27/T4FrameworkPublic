// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseEquipment.h"

/**
  * #68, #107
 */
class UAnimMontage;
class UT4BaseAnimInstance;
class UT4SkeletalMeshComponent;
class UT4WeaponEntityAsset;
class FT4SkeletalMeshEquipment : public FT4BaseEquipment
{
public:
	FT4SkeletalMeshEquipment(AT4GameObject* InOwnerObject, const FT4ActionKey& InActionKey);
	~FT4SkeletalMeshEquipment();

	bool IsWeapon() const override { return true; }

	bool PlayAnimation(const FT4AnimParameters& InAnimParameters) override; // #107

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

	UT4BaseAnimInstance* GetAnimInstance() const;

private:
	ET4EquipLoadState LoadState;
	FT4SkeletalMeshLoader MeshLoader;

	bool bUseAnimation;
	const UT4WeaponEntityAsset* WeaponEntityAsset;
	FT4AnimBlueprintClassLoader AnimBPClassLoader;
	FT4BlendSpaceLoader BlendSpaceLoader;
	FT4AnimMontageLoader AnimMontageLoader;

	TWeakObjectPtr<UT4SkeletalMeshComponent> ComponentPtr;
};
