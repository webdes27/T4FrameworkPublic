// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionAttachedNodeBase.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  *
 */
// #20
class FT4ActionNodeControl;
class UT4StaticMeshComponent;
class FT4ActionMeshNode : public FT4ActionAttachedNodeBase
{
public:
	explicit FT4ActionMeshNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionMeshNode();

	static FT4ActionMeshNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4MeshAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Mesh; }

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
	FT4StaticMeshLoader AssetLoader;
	TWeakObjectPtr<UT4StaticMeshComponent> MeshComponentPtr;

#if WITH_EDITOR
	FT4MeshAction ActionCached;
#endif
};