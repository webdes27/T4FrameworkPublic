// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCompositeNodeBase.h"

/**
  * #63
 */
class FT4ActionNodeControl;
class FT4ActionProjectileNode : public FT4ActionCompositeNodeBase
{
public:
	explicit FT4ActionProjectileNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionProjectileNode();

	// IT4ActionNode
	const FName GetActionPoint() const override { return ActionPoint; } // #63

public:

	static FT4ActionProjectileNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4ProjectileAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Projectile; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void AdvanceLoading(const FT4UpdateTime& InUpdateTime) override;
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	void StartLoading() override;
	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

	void ThrowProjectile(float ThrowOffsetTimeSec);

#if !UE_BUILD_SHIPPING
	void NotifyDebugPaused(bool bPause) override; // #54
#endif

private:
	FName ActionPoint;

	bool bThrowed;
	float ThrowDelayTimeSec;

	float CastingStopDelayTimeSec;
	float CastingStopTimeSec;
	float ProjectileSpeed;
	float ProjectileDurationSec; // #63 : 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec, Hit 가 없으면 0이다.
	float ProjectileLength; // #112 : Projectile 의 길이, 충돌 계산에서 Offset 으로 사용. (원점 에서의 길이)

	bool bEnableHitAttached; // #112
	float HitAttachedTimeSec; // #112

	FSoftObjectPath CastingAssetPath;
	FT4ContiAssetLoader CastingAssetLoader;
	ET4LoadingPolicy LoadingPolicy; // #56

	FSoftObjectPath HeadAssetPath;
	FT4ContiAssetLoader HeadAssetLoader;

	FSoftObjectPath EndAssetPath;

#if WITH_EDITOR
	FT4ProjectileAction ActionCached;
#endif
};
