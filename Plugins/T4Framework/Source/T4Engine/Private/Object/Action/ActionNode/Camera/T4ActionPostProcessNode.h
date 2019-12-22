// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCameraNode.h"
#include "Public/T4Engine.h"
#include "Engine/Scene.h" // #100

/**
  * #100
 */
class FT4ActionControl;
class FT4ActionPostProcessNode : public FT4ActionCameraNode
{
public:
	explicit FT4ActionPostProcessNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionPostProcessNode();

	static FT4ActionPostProcessNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4PostProcessAction& InAction,
		const FT4ActionParameters* InParameters
	);

	const ET4ActionType GetType() const override { return ET4ActionType::PostProcess; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override { return false; }

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	FPostProcessSettings PostProcessSettings;

#if WITH_EDITOR
	FT4PostProcessAction ActionCached;
#endif
};
