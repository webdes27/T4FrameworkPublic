// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "Public/T4EngineStructs.h"
#include "Public/Action/T4ActionKey.h"

/**
  *
 */
class IT4ActionNode;
class FT4ActionNodeBase;
struct FPendingyDestroyInfo
{
	float LeftTimeSec;
	FT4ActionNodeBase* ActionNode;
};

typedef TMultiMap<FT4ActionKey, FT4ActionNodeBase*> T4ACTION_MULTI_MAP;

// #23
class FT4ActionNodeBaseGraph
{
public:
	explicit FT4ActionNodeBaseGraph();
	virtual ~FT4ActionNodeBaseGraph();

	void Advance(const FT4UpdateTime& InUpdateTime);

	void Reset();

	void AddChildNode(const FT4ActionKey& InActionKey, FT4ActionNodeBase* InNewNode);
	bool RemoveChildNode(
		const FT4ActionKey& InActionKey,
		float InDelayTimeSec,
		bool bInSameKeyDestroyAll // Primary 와 관계없이 FT4ActionKey:Value 와 같으면 모두 삭제
	);

	IT4ActionNode* GetChilldNodeByPrimary(const FT4ActionKey& InPrimaryActionKey) const;
	bool GetChildNodes(const FT4ActionKey& InSameActionKey, TArray<IT4ActionNode*>& OutNodes) const;

	TArray<FT4ActionNodeBase*>& GetChildNodes() { return ChildNodes; }
	T4ACTION_MULTI_MAP& GetChildMultiMap() { return ChildMultiMap; }

	bool IsPlaying() const;
	bool IsLooping() const;
	
	void OnStopAllChildren(); // #56

	bool CheckFinished() const; // #56 : 하위 노드까지 검사하는 처리 추가
	bool CheckStopedAndFinished() const;

	uint32 NumChildActions() const;

#if !UE_BUILD_SHIPPING
	bool IsDebugPaused() const; // #56
	void SetDebugPause(bool bInPause); // #54
#endif

protected:
	TArray<FT4ActionNodeBase*> ChildNodes;
	T4ACTION_MULTI_MAP ChildMultiMap;
	TArray<FPendingyDestroyInfo> DestroyPendingChildInfos;
	TArray<FT4ActionNodeBase*> DestoryBeforeFrameNodes;
};
