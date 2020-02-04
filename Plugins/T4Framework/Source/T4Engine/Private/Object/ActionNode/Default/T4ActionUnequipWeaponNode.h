// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"

/**
  * #48, #111
 */
class FT4ActionNodeControl;
class FT4ActionUnequipWeaponNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionUnequipWeaponNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionUnequipWeaponNode();

	static FT4ActionUnequipWeaponNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4UnequipWeaponAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::UnequipWeapon; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	bool Play() override;
	void Stop() override;

	bool PlayInternal(float InOffsetTimeSec) override;

private:
	FT4ActionKey EquipmentActionKey;
	bool bChangeDefaultStance; // #110 : Default Stance 로 변경해준다.
	bool bChangeStanceSync; // #111 : 스탠스 변경과 동기화한다.

#if WITH_EDITOR
	FT4UnequipWeaponAction ActionCached;
#endif
};
