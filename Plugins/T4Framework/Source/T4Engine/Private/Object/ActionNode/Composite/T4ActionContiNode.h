// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCompositeNodeBase.h"

/**
  *
 */
class FT4ActionNodeControl;
class UT4ContiAsset;
class FT4ActionContiNode : public FT4ActionCompositeNodeBase
{
public:
	explicit FT4ActionContiNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionContiNode();

	static FT4ActionContiNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4ContiAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Conti; }

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

private:
	FSoftObjectPath AssetPath;
	FT4ContiAssetLoader AssetLoader;
	ET4LoadingPolicy LoadingPolicy; // #56

#if WITH_EDITOR
	FT4ContiAction ActionCached;
#endif
};
