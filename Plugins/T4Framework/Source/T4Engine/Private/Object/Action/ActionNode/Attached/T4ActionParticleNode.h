// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionAttachedNode.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  *
 */
// #20
class FT4ActionControl;
class UT4ParticleSystemComponent;
class FT4ActionParticleNode : public FT4ActionAttachedNode
{
public:
	explicit FT4ActionParticleNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionParticleNode();

	static FT4ActionParticleNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4ParticleAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Particle; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
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
	float PlayRate; // #56
	FT4ParticleSystemLoader AssetLoader;
	TWeakObjectPtr<UT4ParticleSystemComponent> ParticleSystemComponentPtr;

	bool bAutoFinished; // #54
	bool bParticleLooping; // #54 : 에셋에서 루핑 사용중인지 체크

#if WITH_EDITOR
	FT4ParticleAction ActionCached;
#endif
};