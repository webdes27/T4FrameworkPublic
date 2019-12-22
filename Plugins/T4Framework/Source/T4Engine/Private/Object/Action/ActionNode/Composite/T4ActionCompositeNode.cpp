// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionCompositeNode.h"
#include "Object/Action/ActionNode/T4ActionNodeIncludes.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/Action/T4ActionParameters.h" // #28

#include "T4EngineInternal.h"

/**
  * #24
 */
FT4ActionCompositeNode::FT4ActionCompositeNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
{
}

FT4ActionCompositeNode::~FT4ActionCompositeNode()
{
}

inline const FT4ContiActionStruct* GetContiActionBase(
	const FT4ActionCompositeData& InCompositeData,
	const FT4ActionHeaderInfo& InHierarchyInfo
)
{
	// #T4_ADD_ACTION_TAG_CONTI

	switch (InHierarchyInfo.ActionType)
	{
		case ET4ActionType::Branch: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.BranchActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.BranchActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::SpecialMove: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.SpecialMoveActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.SpecialMoveActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Animation: // #17
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.AnimationActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.AnimationActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Particle: // #20
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ParticleActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.ParticleActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Decal: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.DecalActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.DecalActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Projectile: // #63
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ProjectileActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.ProjectileActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Reaction: // #76
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ReactionActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.ReactionActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::LayerSet: // #81
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.LayerSetActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.LayerSetActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::TimeScale: // #102
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.TimeScaleActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.TimeScaleActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::CameraWork: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.CameraWorkActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.CameraWorkActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::CameraShake: // #101
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.CameraShakeActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.CameraShakeActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::PostProcess: // #100
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.PostProcessActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.PostProcessActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		case ET4ActionType::Environment: // #99
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.EnvironmentActions.Num());
				return static_cast<const FT4ContiActionStruct*>(&InCompositeData.EnvironmentActions[InHierarchyInfo.ActionArrayIndex]);
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("GetContiActionBase: Unknown action type '%u'"),
					uint8(InHierarchyInfo.ActionType)
				);
				check(false);
			}
			break;
	}
	return nullptr;
}

inline uint32 GetActionSortOrder(
	const FT4ActionCompositeData& InCompositeData,
	const FT4ActionHeaderInfo& InHierarchyInfo
)
{
	const FT4ContiActionStruct* ContiActionBase = GetContiActionBase(InCompositeData, InHierarchyInfo);
	if (nullptr != ContiActionBase)
	{
		return ContiActionBase->SortOrder;
	}
	else
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("GetActionSortOrder: Unknown action type '%u'"),
			uint8(InHierarchyInfo.ActionType)
		);
		check(false);
	}
	return INDEX_NONE;
}

void FT4ActionCompositeNode::ProcessCompositeData(
	const FT4ActionCompositeData& InCompositeData,
	const float InMaxPlayTimeSec, // #54
	const float InOffsetTimeSec // #56
)
{
	// #56 : SortOrder 순으로 정렬해 처리한다.
	struct FT4CompositeActionInfo
	{
		uint32 SortOrder;
		int32 HeaderIndex;
		ET4ActionType ActionType; // #58
	};
	TArray<FT4CompositeActionInfo> SortedCompositeActionInfos;
	for (TMap<uint32, FT4ActionHeaderInfo>::TConstIterator It(InCompositeData.HeaderInfoMap); It; ++It)
	{
		const FT4ActionHeaderInfo& HierarchyInfo = It.Value();
		FT4CompositeActionInfo& NewInfo = SortedCompositeActionInfos.AddDefaulted_GetRef();
		NewInfo.SortOrder = GetActionSortOrder(InCompositeData, HierarchyInfo);
		NewInfo.HeaderIndex = It.Key();
		NewInfo.ActionType = HierarchyInfo.ActionType; // #58
	}
	SortedCompositeActionInfos.Sort([](const FT4CompositeActionInfo& A, const FT4CompositeActionInfo&B)
	{
		return A.SortOrder < B.SortOrder;
	});

	TMap<uint32, FT4ActionNode*> TempActionMap;
	for (FT4CompositeActionInfo& CompositeActionInfo : SortedCompositeActionInfos)
	{
		if (!InCompositeData.HeaderInfoMap.Contains(CompositeActionInfo.HeaderIndex))
		{
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("ProcessCompositeData failed : Action Header Info '%u' not found."),
				CompositeActionInfo.HeaderIndex
			);
			continue;
		}

		const FT4ActionHeaderInfo& HierarchyInfo = InCompositeData.HeaderInfoMap[CompositeActionInfo.HeaderIndex];

		FT4ActionNode* FoundParentNode = this;
		FT4ActionKey ChildActionKey;
		FT4ActionNode* NewChildNode = nullptr;

#if WITH_EDITOR
		if (ActionParameterPtr.IsValid())
		{
			// #56 : Conti Editor 에서 Invisible or Isolate 로 출력을 제어할 때 더미용으로 사용(delay, duration 동작 보장)
			const FT4EditorParameters& EditorParam = ActionParameterPtr->EditorParams;

			bool bUseDummyNode = false;
			if (0 < EditorParam.IsolationActionSet.Num())
			{
				bUseDummyNode = true;
				// #56 : 하나라도 Isolate 가 사용되면 포함된 것만 출력하고 나머지는 모두 Hide 한다.
				if (EditorParam.IsolationActionSet.Contains(CompositeActionInfo.HeaderIndex))
				{
					bUseDummyNode = false;
				}
			}
			if (!bUseDummyNode)
			{
				// #56 : Invisible 에 포함되면 출력하지 않는다.
				if (ET4ActionType::CameraWork != CompositeActionInfo.ActionType && // #58 : CameraWork 의 Invisible 은 카메라 모델 출력으로 대체
					EditorParam.InvisibleActionSet.Contains(CompositeActionInfo.HeaderIndex))
				{
					bUseDummyNode = true;
				}
			}
			if (bUseDummyNode)
			{
				const FT4ContiActionStruct* BaesAction = GetActionStruct(HierarchyInfo, InCompositeData);
				NewChildNode = NewChildActionNode(
					FoundParentNode,
					ET4ActionType::Dummy,
					BaesAction,
					InMaxPlayTimeSec, // #54
					InOffsetTimeSec, // #53
					ChildActionKey,
					TempActionMap
				);
				continue;
			}
		}
#endif

		const FT4ContiActionStruct* ContiActionBase = GetContiActionBase(InCompositeData, HierarchyInfo);
		if (nullptr != ContiActionBase)
		{
			NewChildNode = NewChildActionNode(
				FoundParentNode,
				ContiActionBase,
				InMaxPlayTimeSec, // #54
				InOffsetTimeSec, // #53
				ChildActionKey,
				TempActionMap
			);
		}
		else
		{
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("FT4ActionCompositeNode::LoadCompositeData : Unknown action type '%u'"),
				uint8(HierarchyInfo.ActionType)
			);
			check(false);
		}
	}

	TempActionMap.Empty();
}

FT4ActionNode* FT4ActionCompositeNode::NewChildActionNode(
	FT4ActionNode* InParentNode,
	const ET4ActionType InActionType, // #56
	const FT4ContiActionStruct* InAction,
	const float InMaxPlayTimeSec, // #54
	const float InOffsetTimeSec, // #56
	FT4ActionKey& OutChildActionKey,
	TMap<uint32, FT4ActionNode*>& OutTempActionMap
)
{
	check(nullptr != ActionControlRef);
	if (ET4LifecycleType::Duration == InAction->LifecycleType)
	{
		// #56 : 만약, Duration 을 사용하는 Action 이 OffsetTime 보다 작다면 플레이 할 필요가 없을 것이다!
		//       단, 몇가지 액션은 옵션으로 1회성 플레이를 보장해주어야 할 수도 있다.
		//       FT4ActionControl::CreateNode 와 FT4ActionNode::AddChildNode, FT4ActionCompositeNode::NewChildActionNode 에서 체크하고 있다.
		if ((InAction->StartTimeSec + InAction->DurationSec) <= InOffsetTimeSec)
		{
			UE_LOG(
				LogT4Engine,
				VeryVerbose,
				TEXT("FT4ActionCompositeNode::NewChildActionNode '%s' skiped. (DurationSec <= InOffsetTimeSec)"),
				*(InAction->ToString())
			);
			return nullptr;
		}
	}
	FT4ActionNode* NewChildNode = FT4ActionNode::CreateNode(
		ActionControlRef, 
		InActionType,
		InAction,
		ActionParameterPtr.Get() // #54
	);
	if (nullptr == NewChildNode)
	{
		return nullptr;
	}
	check(!OutTempActionMap.Contains(InAction->HeaderKey));
	OutTempActionMap.Add(InAction->HeaderKey, NewChildNode);
	check(nullptr != InParentNode);
	// #54, #56 : AddChild 전에 호출해야 Conti 류의 CompositeData 생성에 적용이 된다.
	NewChildNode->SetOffsetTimeSec(InOffsetTimeSec);
	if (0.0f < InMaxPlayTimeSec)
	{
		// #54 : Conti 로 플레이 할 경우 설정되는 최대 플레이시간 설정 대응
		NewChildNode->SetGlobalPlayTimeSec(
			(InParentNode == this) ? GetElapsedTimeSec() : InParentNode->GetGlobalElapsedTimeSec(),
			InMaxPlayTimeSec
		);
	}
	if (!InParentNode->AddChildNodeInternal(InAction, NewChildNode))
	{
		FT4ActionNode::DestroyNode(&NewChildNode);
		return nullptr;
	}
	return NewChildNode;
}

FT4ActionNode* FT4ActionCompositeNode::NewChildActionNode(
	FT4ActionNode* InParentNode,
	const FT4ContiActionStruct* InAction,
	const float InMaxPlayTimeSec, // #54
	const float InOffsetTimeSec, // #56
	FT4ActionKey& OutChildActionKey,
	TMap<uint32, FT4ActionNode*>& OutTempActionMap
)
{
	return NewChildActionNode(
		InParentNode,
		InAction->ActionType,
		InAction,
		InMaxPlayTimeSec,
		InOffsetTimeSec, // #56
		OutChildActionKey,
		OutTempActionMap
	);
}

#if WITH_EDITOR
const FT4ContiActionStruct* FT4ActionCompositeNode::GetActionStruct(
	const FT4ActionHeaderInfo& InHierarchyInfo,
	const FT4ActionCompositeData& InCompositeData
) const // #56
{
	// #T4_ADD_ACTION_TAG_CONTI

	switch (InHierarchyInfo.ActionType)
	{
		case ET4ActionType::Branch: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.BranchActions.Num());
				return &InCompositeData.BranchActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::SpecialMove: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.SpecialMoveActions.Num());
				return &InCompositeData.SpecialMoveActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Animation: // #17
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.AnimationActions.Num());
				return &InCompositeData.AnimationActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Particle: // #20
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ParticleActions.Num());
				return &InCompositeData.ParticleActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Decal: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.DecalActions.Num());
				return &InCompositeData.DecalActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Projectile: // #63
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ProjectileActions.Num());
				return &InCompositeData.ProjectileActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Reaction: // #76
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.ReactionActions.Num());
				return &InCompositeData.ReactionActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::LayerSet: // #81
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.LayerSetActions.Num());
				return &InCompositeData.LayerSetActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::TimeScale: // #102
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.TimeScaleActions.Num());
				return &InCompositeData.TimeScaleActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::CameraWork: // #54
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.CameraWorkActions.Num());
				return &InCompositeData.CameraWorkActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::CameraShake: // #101
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.CameraShakeActions.Num());
				return &InCompositeData.CameraShakeActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::PostProcess: // #99
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.PostProcessActions.Num());
				return &InCompositeData.PostProcessActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		case ET4ActionType::Environment: // #99
			{
				check(InHierarchyInfo.ActionArrayIndex < InCompositeData.EnvironmentActions.Num());
				return &InCompositeData.EnvironmentActions[InHierarchyInfo.ActionArrayIndex];
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("FT4ActionCompositeNode::GetActionStruct : Unknown action type '%u'"),
					uint8(InHierarchyInfo.ActionType)
				);
				check(false);
			}
			break;
	}
	return nullptr;
}
#endif