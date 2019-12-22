// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCameraNode.h"
#include "Public/T4Engine.h"
#include "T4Asset/Public/Action/T4ActionContiStructs.h" // #58

/**
  * #58
 */
struct FT4PlayCameraWorkSectionKeyData
{
	float StartTimeSec;
	float EndTimeSec;
	float RangeTimeSec;
	int32 NextKeyIndex;
	FT4CameraWorkSectionKeyData KeyData;
};

class FT4ActionControl;
class FT4ActionCameraWorkNode : public FT4ActionCameraNode
{
public:
	explicit FT4ActionCameraWorkNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCameraWorkNode();

	static FT4ActionCameraWorkNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4CameraWorkAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::CameraWork; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override;

private:
	TArray<FT4PlayCameraWorkSectionKeyData> PlayKeyDatas; // #58

#if WITH_EDITOR
	bool bShowEditorCameraActor;
	FT4CameraWorkAction ActionCached;
#endif
};
