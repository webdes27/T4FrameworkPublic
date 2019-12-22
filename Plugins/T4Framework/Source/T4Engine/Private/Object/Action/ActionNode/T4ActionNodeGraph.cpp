// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionNodeGraph.h"
#include "Object/Action/ActionNode/T4ActionNode.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionNodeGraph::FT4ActionNodeGraph()
{
}

FT4ActionNodeGraph::~FT4ActionNodeGraph()
{
	check(0 == DestoryBeforeFrameNodes.Num());
	check(0 == DestroyPendingChildInfos.Num());
	check(0 == ChildNodes.Num());
}

void FT4ActionNodeGraph::Reset()
{
#if 0
	for (FT4ActionNode* ActionNode : DestoryBeforeFrameNodes)
	{
		if (ChildNodes.Contains(ActionNode))
		{
			// #20 : FDelayDestroyInfo 에서 왔다면 ActionRootNodes 에 없음으로 예외 처리해준다.
			ChildNodes.Remove(ActionNode);
		}
		ActionNode->OnDestroy();
		delete ActionNode;
	}
#endif
	// WARN : ChildNodes 에서 삭제될 것이기 때문에 위의 처리는 주석처리해둔다.
	DestoryBeforeFrameNodes.Empty();

	for (const FPendingyDestroyInfo& DestroyInfo : DestroyPendingChildInfos)
	{
		check(nullptr != DestroyInfo.ActionNode);
		DestroyInfo.ActionNode->OnDestroy();
		delete DestroyInfo.ActionNode;
	}
	DestroyPendingChildInfos.Empty();

	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		ActionNode->OnDestroy();
		delete ActionNode;
	}
	ChildNodes.Empty();
	ChildMultiMap.Empty();
}

void FT4ActionNodeGraph::Advance(const FT4UpdateTime& InUpdateTime)
{
	// #20 : 안전한 삭제를 위해 다음 프레임에 처리한다.
	for (FT4ActionNode* ActionNode : DestoryBeforeFrameNodes)
	{
		if (ChildNodes.Contains(ActionNode))
		{
			// #20 : FDelayDestroyInfo 에서 왔다면 ActionRootNodes 에 없음으로 예외 처리해준다.
			ChildNodes.Remove(ActionNode);
		}
		ActionNode->OnDestroy();
		delete ActionNode;
	}
	DestoryBeforeFrameNodes.Empty();

	// Stop 을 통해 삭제 예정인 액션을 정리한다.
	for (TArray<FPendingyDestroyInfo>::TIterator It(DestroyPendingChildInfos); It; ++It)
	{
		FPendingyDestroyInfo& DestroyInfo = *It;
		check(nullptr != DestroyInfo.ActionNode);
		DestroyInfo.ActionNode->OnAdvance(InUpdateTime);
		DestroyInfo.LeftTimeSec -= InUpdateTime.ScaledTimeSec;
		if (0.0f >= DestroyInfo.LeftTimeSec || DestroyInfo.ActionNode->IsDestroyable())
		{
			// #102 : 삭제 대기로 들어갈 경우 호출됨. 삭제 대기로 들어갈 경우 한프레임 늦게 삭제되기 때문에 일부 액션(Ex:TimeScale)
			//        의 원상복구가 되지 않는 문제가 있어 추가함
			DestroyInfo.ActionNode->OnDestroying();
			DestoryBeforeFrameNodes.Add(DestroyInfo.ActionNode);
			DestroyPendingChildInfos.RemoveAt(It.GetIndex());
		}
	}

	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		ActionNode->OnAdvance(InUpdateTime);
		if (ActionNode->IsDestroyable())
		{
			const FT4ActionKey& ActionKey = ActionNode->GetKey();
			int32 NumRemoved = ChildMultiMap.Remove(ActionKey, ActionNode);
			// check(0 < NumRemoved); // #100 : Conti 외 Conti 내 Action 을 코드로 부를경우 assert 조건이 되지 않아 주석 처리
			DestoryBeforeFrameNodes.Add(ActionNode);
		}
	}
}

void FT4ActionNodeGraph::AddChildNode(
	const FT4ActionKey& InActionKey, 
	FT4ActionNode* InNewNode
)
{
	ChildNodes.Add(InNewNode);
	ChildMultiMap.Add(InActionKey, InNewNode);
}

bool FT4ActionNodeGraph::RemoveChildNode(
	const FT4ActionKey& InActionKey,
	float InDelayTimeSec,
	bool bInSameKeyDestroyAll
)
{
	if (bInSameKeyDestroyAll && InActionKey.IsPrimaryKey())
	{
		FT4ActionKey SameActionKey = InActionKey;
		SameActionKey.bPrimary = false;
		RemoveChildNode(SameActionKey, InDelayTimeSec, bInSameKeyDestroyAll);
	}
	if (!ChildMultiMap.Contains(InActionKey))
	{
		return false;
	}
	TArray<FT4ActionNode*> FoundActionRootNodes;
	ChildMultiMap.MultiFind(InActionKey, FoundActionRootNodes);
	for (FT4ActionNode* ActionNode : FoundActionRootNodes)
	{
		check(nullptr != ActionNode);
		FPendingyDestroyInfo NewDelayDestroyInfo;
		NewDelayDestroyInfo.LeftTimeSec = InDelayTimeSec;
		NewDelayDestroyInfo.ActionNode = ActionNode;
		DestroyPendingChildInfos.Add(NewDelayDestroyInfo);
		ChildNodes.Remove(ActionNode);
	}
	ChildMultiMap.Remove(InActionKey);
	return true;
}

IT4ActionNode* FT4ActionNodeGraph::GetChilldNodeByPrimary(
	const FT4ActionKey& InPrimaryActionKey
) const
{
	if (!InPrimaryActionKey.IsPrimaryKey())
	{
		return nullptr;
	}
	if (!ChildMultiMap.Contains(InPrimaryActionKey))
	{
		return nullptr;
	}
	TArray<FT4ActionNode*> FoundActionRootNodes;
	ChildMultiMap.MultiFind(InPrimaryActionKey, FoundActionRootNodes);
	uint32 NumFoundActionRootNodes = FoundActionRootNodes.Num();
	check(1 == NumFoundActionRootNodes);
	return static_cast<IT4ActionNode*>(FoundActionRootNodes[0]);
}

bool FT4ActionNodeGraph::GetChildNodes(
	const FT4ActionKey& InSameActionKey,
	TArray<IT4ActionNode*>& OutNodes
) const
{
	if (!ChildMultiMap.Contains(InSameActionKey))
	{
		return false;
	}
	TArray<FT4ActionNode*> FoundActionRootNodes;
	ChildMultiMap.MultiFind(InSameActionKey, FoundActionRootNodes);
	uint32 NumFoundActionRootNodes = FoundActionRootNodes.Num();
	if (0 >= NumFoundActionRootNodes)
	{
		return false;
	}
	OutNodes.SetNum(NumFoundActionRootNodes);
	int32 Index = 0;
	for (FT4ActionNode* ActionNode : FoundActionRootNodes)
	{
		check(nullptr != ActionNode);
		OutNodes[Index++] = static_cast<IT4ActionNode*>(ActionNode);
	}
	return true;
}

bool FT4ActionNodeGraph::IsPlaying() const
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		if (ActionNode->IsPlaying())
		{
			return true;
		}
	}
	return false;
}

bool FT4ActionNodeGraph::IsLooping() const
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		if (ActionNode->IsLooping())
		{
			return true;
		}
	}
	return false;
}

void FT4ActionNodeGraph::OnStopAllChildren() // #63
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		ActionNode->OnStop();
	}
}

bool FT4ActionNodeGraph::CheckFinished() const // #56 : 하위 노드까지 검사하는 처리 추가
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		if (!ActionNode->CheckFinished())
		{
			return false;
		}
	}
	return true;
}

bool FT4ActionNodeGraph::CheckStopedAndFinished() const
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		if (!ActionNode->CheckStopedAndFinished())
		{
			return false;
		}
	}
	return true;
}

uint32 FT4ActionNodeGraph::NumChildActions() const
{
	uint32 NumChildActions = ChildNodes.Num();
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		NumChildActions += ActionNode->NumChildActions();
	}
	return NumChildActions;
}

#if !UE_BUILD_SHIPPING
bool FT4ActionNodeGraph::IsDebugPaused() const
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		if (ActionNode->IsDebugPaused())
		{
			return true;
		}
	}
	return false;
}

void FT4ActionNodeGraph::SetDebugPause(bool bPause) // #54
{
	for (FT4ActionNode* ActionNode : ChildNodes)
	{
		check(nullptr != ActionNode);
		ActionNode->SetDebugPause(bPause);
	}
}
#endif
