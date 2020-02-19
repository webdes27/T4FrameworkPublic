// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #114
 */
struct FT4GameplayAIStat // #114
{
	FT4GameplayAIStat()
		: Health_Point(0.0f)
		, Mana_Point(0.0f)
		, Striking_Power(0.0f)
		, Defensive_Power(0.0f)
		, Hit_Ratio(0.0f)
		, Dodge_Ratio(0.0f)
		, Result_EXP(0.0f)
	{
	}

	// FT4GameBuiltin_StatTableRow
	// Total Stat = InitializeNPCStatDataID + MainWeaponDataID (Stat)

	float Health_Point; // 피, 체력
	float Mana_Point; // 마력, 기
	float Striking_Power; // 공격력
	float Defensive_Power; // 방어력
	float Hit_Ratio; // 명중률
	float Dodge_Ratio; // 회피률
	float Result_EXP; // 경험치
};