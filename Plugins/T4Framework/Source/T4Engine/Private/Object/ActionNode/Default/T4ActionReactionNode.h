// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionNodeControl;
class FT4ActionReactionNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionReactionNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionReactionNode();

	static FT4ActionReactionNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4ReactionAction& InAction,
		const FT4ActionParameters* InParameters
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Reaction; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override; // #104 : 플레이 되었다면 종료 조건이 되어야 한다.

	bool PlayInternal(float InOffsetTimeSec) override; // #56

	ET4EntityReactionType GetReactionType(FName InReactionName);

private:
	FName ReactionName; // #76
#if WITH_EDITOR
	FT4ReactionAction ActionCached;
#endif
};
