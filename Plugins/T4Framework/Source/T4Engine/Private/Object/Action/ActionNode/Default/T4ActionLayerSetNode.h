// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "Public/T4Engine.h"

/**
  * #81
 */
class FT4ActionControl;
class FT4ActionLayerSetNode : public FT4ActionNode
{
public:
	explicit FT4ActionLayerSetNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionLayerSetNode();

	static FT4ActionLayerSetNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4LayerSetAction& InAction,
		const FT4ActionParameters* InParameters
	);

	const ET4ActionType GetType() const override { return ET4ActionType::LayerSet; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override { return false; }

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	FName LayerTagName;
	ET4LayerTagType LayerTagType;
	FT4ActionKey LayerSetActionKey;
#if WITH_EDITOR
	FT4LayerSetAction ActionCached;
#endif
};
