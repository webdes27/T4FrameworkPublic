// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionControl.h"
#include "ActionNode/T4ActionNode.h"

#include "Public/Action/T4ActionCodeBase.h"
#include "Public/Action/T4ActionParameters.h" // #28

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionControl::FT4ActionControl()
	: OwnerGameObject(nullptr)
	, bPaused(false) // #63 : 새로 생성되는 Node 에게 전파해야 한다!!
	, TimeScale(1.0f) // #102 : 새로 생성되는 Node 에게 전파해야 한다!!
{
}

FT4ActionControl::~FT4ActionControl()
{
	Reset();
}

void FT4ActionControl::Reset()
{
	ActionNodeGraph.Reset();
	SyncActionStatusMap.Empty();
}

void FT4ActionControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	ActionNodeGraph.Advance(InUpdateTime);
}

bool FT4ActionControl::HasAction(const FT4ActionKey& InActionKey) const
{
	TArray<IT4ActionNode*> ChildNodes; // todo : optimizing
	bool bResult = ActionNodeGraph.GetChildNodes(InActionKey, ChildNodes);
	if (bResult)
	{
		return bResult;
	}
	// #23 : ActionNode 를 사용하지 않는 Action의 Key 관리
	bResult = SyncActionStatusMap.Contains(InActionKey);
	return bResult;
}

bool FT4ActionControl::IsPlaying(const FT4ActionKey& InActionKey) const
{
	TArray<IT4ActionNode*> ChildNodes; // todo : optimizing
	bool bResult = ActionNodeGraph.GetChildNodes(InActionKey, ChildNodes);
	if (bResult)
	{
		for (IT4ActionNode* ActionNode : ChildNodes)
		{
			check(nullptr != ActionNode);
			if (ActionNode->IsPlaying())
			{
				return true; // 하위 노드 모두를 체크한다.
			}
		}
	}
	// #23 : ActionNode 를 사용하지 않는 Action의 Key 관리
	bResult = SyncActionStatusMap.Contains(InActionKey);
	return bResult;
}

bool FT4ActionControl::IsLooping(const FT4ActionKey& InActionKey) const
{
	TArray<IT4ActionNode*> ChildNodes; // todo : optimizing
	bool bResult = ActionNodeGraph.GetChildNodes(InActionKey, ChildNodes);
	if (bResult)
	{
		for (IT4ActionNode* ActionNode : ChildNodes)
		{
			check(nullptr != ActionNode);
			if (ActionNode->IsLooping())
			{
				return true; // 하위 노드 모두를 체크한다.
			}
		}
	}
	return false;
}

float FT4ActionControl::GetElapsedTimeSec(const FT4ActionKey& InActionKey) const // #102
{
	TArray<IT4ActionNode*> ChildNodes; // todo : optimizing
	bool bResult = ActionNodeGraph.GetChildNodes(InActionKey, ChildNodes);
	if (bResult)
	{
		for (IT4ActionNode* ActionNode : ChildNodes)
		{
			check(nullptr != ActionNode);
			return ActionNode->GetElapsedTimeSec();
		}
	}
	return 0.0f;
}

IT4ActionNode* FT4ActionControl::GetChildNodeByPrimary(
	const FT4ActionKey& InPrimaryActionKey
) const
{
	return ActionNodeGraph.GetChilldNodeByPrimary(InPrimaryActionKey);
}

bool FT4ActionControl::GetChildNodes(
	const FT4ActionKey& InSameActionKey,
	TArray<IT4ActionNode*>& OutNodes
) const
{
	bool bResult = ActionNodeGraph.GetChildNodes(InSameActionKey, OutNodes);
	return bResult;
}

uint32 FT4ActionControl::NumChildActions() const
{
	return ActionNodeGraph.NumChildActions();
}

uint32 FT4ActionControl::NumChildActions(const FT4ActionKey& InActionKey) const // #54
{
	uint32 NumChildActions = 0;
	TArray<IT4ActionNode*> ChildNodes; // todo : optimizing
	bool bResult = ActionNodeGraph.GetChildNodes(InActionKey, ChildNodes);
	if (bResult)
	{
		for (IT4ActionNode* ActionNode : ChildNodes)
		{
			check(nullptr != ActionNode);
			NumChildActions += ActionNode->NumChildActions();
		}
	}
	return NumChildActions;
}

FT4ActionNode* FT4ActionControl::CreateRootNode(
	const FT4ActionStruct* InAction,
	const FT4ActionParameters* InActionParam
)
{
	FT4ActionKey NewActionKey = FT4ActionKey::EmptyActionKey;
	if (nullptr != InActionParam)
	{
		if (InActionParam->CheckBits(ET4DefaultParamBits::ActionKeyBit)) // #32 : override
		{
			NewActionKey = InActionParam->GetDefaultParams().ActionKey;
		}
	}
	if (!NewActionKey.IsValid() && ET4ActionStructType::Code == InAction->GetActionStructType())
	{
		NewActionKey = (static_cast<const FT4CodeActionStruct*>(InAction))->ActionKey;
	}

	if (NewActionKey.IsPrimaryKey() && IsPlaying(NewActionKey))
	{
		if (!NewActionKey.bOverrideExisting) // #44
		{
			UE_LOG(
				LogT4Engine,
				Verbose,
				TEXT("FT4ActionControl : Primary ActionKey '%s' is already exists!"),
				*(NewActionKey.ToString())
			);
			return nullptr;
		}
		ActionNodeGraph.RemoveChildNode(NewActionKey, 0.0f, false); // #44
	}

	float ApplyOffsetTimesSec = 0.0f; // #56

	TSharedPtr<const FT4ActionParameters> ActionParameterPtr;
	if (nullptr != InActionParam)
	{
		ActionParameterPtr = MakeShareable(new FT4ActionParameters(*InActionParam));  // #28

		if (ActionParameterPtr->CheckBits(ET4TimeParamBits::OffsetTimeBit)) // #56
		{
			ApplyOffsetTimesSec = ActionParameterPtr->GetTimeParams().OffsetTimeSec;
		}
	}

	if (ET4LifecycleType::Duration == InAction->LifecycleType)
	{
		// #56 : 만약, Duration 을 사용하는 Action 이 OffsetTime 보다 작다면 플레이 할 필요가 없을 것이다!
		//       단, 몇가지 액션은 옵션으로 1회성 플레이를 보장해주어야 할 수도 있다.
		//       FT4ActionControl::CreateNode 와 FT4ActionNode::AddChildNode, FT4ActionCompositeNode::AddChildActionNode 에서 체크하고 있다.
		if ((InAction->StartTimeSec + InAction->DurationSec) <= ApplyOffsetTimesSec)
		{
			UE_LOG(
				LogT4Engine,
				VeryVerbose,
				TEXT("FT4ActionControl::CreateNode '%s' skiped. (DurationSec <= ApplyOffsetTimesSec)"),
				*(InAction->ToString())
			);
			return nullptr;
		}
	}

	FT4ActionNode* NewActionNode = FT4ActionNode::CreateNode(
		this, 
		InAction, 
		ActionParameterPtr.Get()
	);
	if (nullptr == NewActionNode)
	{
		return nullptr;
	}

	NewActionNode->SetParentNode(nullptr); // set root
	NewActionNode->SetActionParameters(ActionParameterPtr);  // #28
	NewActionNode->SetOffsetTimeSec(ApplyOffsetTimesSec); // #56

	bool bResult = NewActionNode->OnCreate(InAction);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			VeryVerbose,
			TEXT("FT4ActionControl::CreateNode '%s' OnCreate failed."),
			*(InAction->ToString())
		);
		FT4ActionNode::DestroyNode(&NewActionNode);
		return nullptr;
	}

	ActionNodeGraph.AddChildNode(NewActionKey, NewActionNode);
	return NewActionNode;
}

bool FT4ActionControl::DestroyNode(
	const FT4ActionKey& InActionKey,
	float InDelayTimeSec,
	bool bInSameKeyDestroyAll
)
{
	bool bResult = ActionNodeGraph.RemoveChildNode(
		InActionKey, 
		InDelayTimeSec, 
		bInSameKeyDestroyAll
	);
	return bResult;
}

bool FT4ActionControl::AddSyncActionStatus(
	const FT4ActionKey& InActionKey,
	const ET4ActionType InActionType
)
{
	if (!InActionKey.IsValid())
	{
		return false;
	}
	if (InActionKey.IsPrimaryKey() && SyncActionStatusMap.Contains(InActionKey))
	{
		UE_LOG(
			LogT4Engine,
			Verbose,
			TEXT("AddSyncAction : Primary ActionKey '%s' is already exists!"),
			*(InActionKey.ToString())
		);
		return false;
	}
	SyncActionStatusMap.Add(InActionKey, InActionType);
	return true;
}

void FT4ActionControl::RemoveSyncActionStatus(const FT4ActionKey& InActionKey)
{
	if (!InActionKey.IsValid())
	{
		return;
	}
	if (!SyncActionStatusMap.Contains(InActionKey))
	{
		return;
	}
	SyncActionStatusMap.Remove(InActionKey);
}

IT4GameWorld* FT4ActionControl::GetGameWorld() const // #100
{
	if (nullptr == OwnerGameObject)
	{
		return nullptr;
	}
	return OwnerGameObject->GetGameWorld();
}

#if !UE_BUILD_SHIPPING
void FT4ActionControl::SetDebugPause(bool bInPause) // #68
{
	ActionNodeGraph.SetDebugPause(bInPause);
}
#endif
