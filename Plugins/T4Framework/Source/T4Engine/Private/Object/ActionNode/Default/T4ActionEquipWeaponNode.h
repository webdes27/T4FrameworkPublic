// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"

/**
  * #107
 */
class FT4ActionNodeControl;
class FT4ActionEquipWeaponNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionEquipWeaponNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionEquipWeaponNode();

	static FT4ActionEquipWeaponNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4EquipWeaponAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::EquipWeapon; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	bool Play() override;
	void Stop() override;

	bool PlayInternal(float InOffsetTimeSec) override;

private:
	bool bAttached;
	FT4ActionKey EquipmentActionKey; // #111

	FT4EquipWeaponEntityData MainWeaponData; // #111
	TArray<FT4EquipWeaponEntityData> SubWeaponDatas; // #111

	bool bChangeStanceInEntity; // #110 : Weapon Entity 에 설정된 Stance 도 함께 설정해준다.
	bool bChangeStanceSync; // #111 : 스탠스 변경과 동기화한다.

#if WITH_EDITOR
	FT4EquipWeaponAction ActionCached;
#endif
};
