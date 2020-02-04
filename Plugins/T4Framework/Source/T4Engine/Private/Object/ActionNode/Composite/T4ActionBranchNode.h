// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCompositeNodeBase.h"

/**
  * #54
 */
class FT4ActionNodeControl;
class FT4ActionBranchNode : public FT4ActionCompositeNodeBase
{
public:
	explicit FT4ActionBranchNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionBranchNode();

	static FT4ActionBranchNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4BranchAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Branch; }

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
	ET4BranchCondition Contition; // #54
	FName ConditionName; // #54

	FSoftObjectPath AssetPath;
	FT4ContiAssetLoader AssetLoader;
	ET4LoadingPolicy LoadingPolicy; // #56

#if WITH_EDITOR
	FT4BranchAction ActionCached;
#endif
};
