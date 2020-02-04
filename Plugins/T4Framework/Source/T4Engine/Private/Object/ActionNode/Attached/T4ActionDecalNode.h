// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionAttachedNodeBase.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  * #54
 */
class FT4ActionNodeControl;
class UT4DecalComponent;
class FT4ActionDecalNode : public FT4ActionAttachedNodeBase
{
public:
	explicit FT4ActionDecalNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionDecalNode();

	static FT4ActionDecalNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4DecalAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Decal; }

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
	FVector Scale; // #54
	int32 DecalSortOrder;
	FVector DecalSize; // #54
	float FadeInTimeSec;
	float FadeOutTimeSec;
	FT4MaterialLoader AssetLoader;
	TWeakObjectPtr<UT4DecalComponent> DecalComponentPtr;

#if WITH_EDITOR
	FT4DecalAction ActionCached;
#endif
};
