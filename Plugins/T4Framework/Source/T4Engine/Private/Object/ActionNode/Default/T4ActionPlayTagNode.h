// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  * #81
 */
class FT4ActionNodeControl;
class FT4ActionPlayTagNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionPlayTagNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionPlayTagNode();

	static FT4ActionPlayTagNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4PlayTagAction& InAction,
		const FT4ActionParameters* InParameters
	);

	const ET4ActionType GetType() const override { return ET4ActionType::PlayTag; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override { return false; }

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	FName PlayTagName;
	ET4PlayTagType PlayTagType;
	FT4ActionKey PlayTagActionKey;
#if WITH_EDITOR
	FT4PlayTagAction ActionCached;
#endif
};
