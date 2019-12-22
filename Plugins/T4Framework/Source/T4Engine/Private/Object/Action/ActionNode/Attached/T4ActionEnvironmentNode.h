// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionAttachedNode.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  * #99
 */
class UT4ZoneEntityAsset;
class FT4ActionControl;
struct FT4EnvironmentAction;
class UT4EnvironmentZoneComponent;
class FT4ActionEnvironmentNode : public FT4ActionAttachedNode
{
public:
	explicit FT4ActionEnvironmentNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionEnvironmentNode();

	static FT4ActionEnvironmentNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4EnvironmentAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Environment; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	ET4PlayTarget PlayTarget; // #100
	bool bOverrideBlendTime;
	float OverrideBlendInTimeSec;
	float OverrideBlendOutTimeSec;

	TWeakObjectPtr<const UT4ZoneEntityAsset> EntityAssetPtr;
	TWeakObjectPtr<UT4EnvironmentZoneComponent> EnvironmentZoneComponentPtr;

	bool bBlendStart; // #100

#if WITH_EDITOR
	FT4EnvironmentAction ActionCached;
#endif
};
