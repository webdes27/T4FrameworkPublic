// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionControl.h"
#include "ManualNode/T4ManualNodeIncludes.h" // #76
#include "Public/T4Engine.h"

/**
  * #76
 */
class USkeletalMeshComponent;
class UT4CharacterEntityAsset;
struct FT4EntityCharacterReactionPhysicsStartData;
struct FT4EntityCharacterReactionPhysicsStopData;
class FT4ManualControl : public FT4ActionControl
{
public:
	explicit FT4ManualControl();
	virtual ~FT4ManualControl();

	void Reset() override;

	void Set(AT4GameObject* InGameObject);

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play(const FT4ActionStruct* InAction);

	bool StartPhysics(
		const FT4EntityCharacterReactionPhysicsStartData* InPhysicsData,
		const FVector& InShotDirection
	);
	bool EndPhysics();

	bool StopPhysics(const FT4EntityCharacterReactionPhysicsStopData* InPhysicsData);

	USkeletalMeshComponent* GetSkeletonMeshComponent() const;

#if WITH_EDITOR
	void EditorRestoreReaction(); // #76
#endif

private:
	bool bPhysicsStarted;
	FT4ManualDieNode ManualDieNode;
	FT4ManualResurrectNode ManualResurrectNode;
	FT4ManualHitNode ManualHitNode;
};
