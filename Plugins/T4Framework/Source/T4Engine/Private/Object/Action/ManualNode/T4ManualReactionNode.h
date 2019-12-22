// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ManualNode.h"
#include "Public/T4Engine.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #76

/**
  * #76
 */
struct FT4ReactionPlayInfo
{
	FT4ReactionPlayInfo()
		: bPlayFailed(false)
		, bPhysicsStarted(false)
		, bPhysicsStopped(false)
		, bAnimationStarted(false)
		, TimeSec(0.0f)
		, ShotDirection(FVector::UpVector)
	{
	}
	bool bPlayFailed;
	bool bPhysicsStarted;
	bool bPhysicsStopped;
	bool bAnimationStarted;
	float TimeSec;
	FT4EntityCharacterReactionData Data;
	FVector ShotDirection;
};

class FT4ManualControl;
class USkeletalMeshComponent;
class UT4CharacterEntityAsset;
class FT4ManualReactionNode : public FT4ManualNode
{
public:
	explicit FT4ManualReactionNode(FT4ManualControl* InControl);
	virtual ~FT4ManualReactionNode();

	bool IsPlaying() const { return (0 < PlayInfos.Num()) ? true : false; }

	virtual void StopAll();

protected:
	virtual void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool AdvancePlayInfo(const FT4UpdateTime& InUpdateTime, FT4ReactionPlayInfo& InPlayInfo);

	bool TryPhysicsStart(FT4ReactionPlayInfo& InPlayInfo);
	bool TryPhysicsStop(FT4ReactionPlayInfo& InPlayInfo);
	bool TryAnimation(FT4ReactionPlayInfo& InPlayInfo);

	bool GetDataInEntity(
		const UT4CharacterEntityAsset* InEntity,
		const FName InReactionName,
		bool bInTransientPlay,
		FT4EntityCharacterReactionData& OutReactionData
	);

private:
	void AdvancePhysicsStart(const FT4UpdateTime& InUpdateTime, FT4ReactionPlayInfo& InPlayInfo);
	void AdvancePhysicsStop(const FT4UpdateTime& InUpdateTime, FT4ReactionPlayInfo& InPlayInfo);
	void AdvanceAnimation(const FT4UpdateTime& InUpdateTime, FT4ReactionPlayInfo& InPlayInfo);

protected:
	TArray<FT4ReactionPlayInfo> PlayInfos;
};
