// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionTaskIncludes.h" // #76
#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Public/T4Engine.h"

/**
  * #76
 */
class USkeletalMeshComponent;
class UT4CharacterEntityAsset;
struct FT4EntityCharacterReactionPhysicsStartData;
struct FT4EntityCharacterReactionPhysicsStopData;
class FT4ActionTaskControl : public FT4ActionNodeControl
{
public:
	explicit FT4ActionTaskControl();
	virtual ~FT4ActionTaskControl();

	void Reset() override;

	void Set(AT4GameObject* InGameObject);

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool IsPending(ET4ActionType InActionType);
	void Flush(ET4ActionType InActionType); // #111
	bool Play(const FT4ActionCommand* InAction);

	bool PhysicsStart(const FT4EntityCharacterReactionPhysicsStartData* InPhysicsData, const FVector& InShotDirection);
	bool PhysicsEnd();
	bool PhysicsStop(const FT4EntityCharacterReactionPhysicsStopData* InPhysicsData);

	USkeletalMeshComponent* GetSkeletonMeshComponent() const;

#if WITH_EDITOR
	void EditorRestoreReaction(); // #76
#endif

private:
	bool bPhysicsStarted;

	FT4ActionDieTask ActionDieTask;
	FT4ActionResurrectTask ActionResurrectTask;
	FT4ActionHitTask ActionHitTask;

	FT4ActionStanceTask ActionStanceTask; // #111
	FT4ActionSubStanceTask ActionSubStanceTask; // #111
};
