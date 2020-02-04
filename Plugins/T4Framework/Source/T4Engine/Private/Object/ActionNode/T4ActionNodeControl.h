// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionNodeGraph.h"
#include "Public/T4Engine.h"

/**
  *
 */
class AT4GameObject;
class FT4ActionNodeBase;

struct FDelayDestroyInfo
{
	float LeftTimeSec;
	FT4ActionNodeBase* ActionNode;
};

class FT4ActionNodeControl : public IT4ActionControl
{
public:
	explicit FT4ActionNodeControl();
	virtual ~FT4ActionNodeControl();

	// IT4ActionControl
	bool HasAction(const FT4ActionKey& InActionKey) const override; // #102

	bool IsPlaying(const FT4ActionKey& InActionKey) const override;
	bool IsLooping(const FT4ActionKey& InActionKey) const override;

	float GetElapsedTimeSec(const FT4ActionKey& InActionKey) const override; // #102

	IT4ActionNode* GetChildNodeByPrimary(const FT4ActionKey& InPrimaryActionKey) const override;
	bool GetChildNodes(const FT4ActionKey& InSameActionKey, TArray<IT4ActionNode*>& OutNodes) const override;

	uint32 NumChildActions() const override;
	uint32 NumChildActions(const FT4ActionKey& InActionKey) const override; // #54

#if !UE_BUILD_SHIPPING
	void SetDebugPause(bool bInPause); // #68
#endif

public:
	virtual void Reset();

	virtual void Set(AT4GameObject* InGameObject)
	{
		OwnerGameObject = InGameObject;
	}

	virtual void Advance(const FT4UpdateTime& InUpdateTime);

	FT4ActionNodeBase* CreateRootNode(
		const FT4ActionCommand* InAction,
		const FT4ActionParameters* InActionParam
	);

	bool DestroyNode(
		const FT4ActionKey& InActionKey,
		float InDelayTimeSec,
		bool bSameKeyNameRemoveAll
	);

	bool AddSyncActionStatus(const FT4ActionKey& InActionKey, const ET4ActionType InActionType); // #23
	void RemoveSyncActionStatus(const FT4ActionKey& InActionKey); // #23

	AT4GameObject* GetGameObject() const { return OwnerGameObject; }
	IT4GameWorld* GetGameWorld() const; // #100

protected:
	friend class FT4ActionNodeBase;
	friend class FT4ActionAttachedNodeBase; // #48

protected:
	AT4GameObject* OwnerGameObject;

	FT4ActionNodeBaseGraph ActionNodeGraph;
	TMultiMap<FT4ActionKey, ET4ActionType> SyncActionStatusMap; // #23 : ActionNode 를 사용하지 않는 Action의 Key 관리

	bool bPaused; // #63 : 새로 생성되는 Node 에게 전파해야 한다!!
	float TimeScale; // #102 : 새로 생성되는 Node 에게 전파해야 한다!!
};
