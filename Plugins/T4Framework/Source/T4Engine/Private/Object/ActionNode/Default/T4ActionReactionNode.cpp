// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReactionNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionReactionNode::FT4ActionReactionNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, ReactionName(NAME_None)
{
}

FT4ActionReactionNode::~FT4ActionReactionNode()
{
}

FT4ActionReactionNode* FT4ActionReactionNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4ReactionAction& InAction,
	const FT4ActionParameters* InParameters
)
{
	check(ET4ActionType::Reaction == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionReactionNode* NewNode = new FT4ActionReactionNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionReactionNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Reaction == InAction->ActionType);
	const FT4ReactionAction& ConvAction = *(static_cast<const FT4ReactionAction*>(InAction));

	ReactionName = ConvAction.ReactionName;

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionReactionNode::Destroy()
{
}

void FT4ActionReactionNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionReactionNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("Failed to Reaction '%s'"),
			*(ReactionName.ToString())
		);
		OnStop();
	}
	return bResult;
}

void FT4ActionReactionNode::Stop()
{
}

bool FT4ActionReactionNode::IsAutoFinished() const // #104 : 플레이 되었다면 종료 조건이 되어야 한다.
{
	return IsPlayed();
}

bool FT4ActionReactionNode::PlayInternal(float InOffsetTimeSec)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	FVector ShotDirection = FVector::UpVector;
	IT4GameObject* TargetObject = nullptr;
	bool bResult = FindTargetObjectInParameter(&TargetObject, TEXT("Reaction TargetObject"));
	if (bResult)
	{
		ShotDirection = OwnerObject->GetNavPoint() - TargetObject->GetNavPoint();
		ShotDirection.Normalize();
	}

	ET4EntityReactionType ReactionType = GetReactionType(ReactionName);
	switch (ReactionType)
	{
		case ET4EntityReactionType::Die:
			{
				FT4DieAction NewAction;
				NewAction.ReactionName = ReactionName;
				NewAction.ShotDirection = ShotDirection;
				OwnerObject->OnExecuteAction(&NewAction, nullptr);
			}
			break;

		case ET4EntityReactionType::Resurrect:
			{
				FT4ResurrectAction NewAction;
				NewAction.ReactionName = ReactionName;
				OwnerObject->OnExecuteAction(&NewAction, nullptr);
			}
			break;

		case ET4EntityReactionType::Hit:
			{
				FT4HitAction NewAction;
				NewAction.ReactionName = ReactionName;
				NewAction.ShotDirection = ShotDirection;
				OwnerObject->OnExecuteAction(&NewAction, nullptr);
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Type (%u)"),
					uint8(ReactionType)
				);
				return false;
			}
			break;
	}
	return true;
}

ET4EntityReactionType FT4ActionReactionNode::GetReactionType(FName InReactionName)
{
	AT4GameObject* OwnerObject = GetGameObject();
	if (nullptr == OwnerObject)
	{
		return ET4EntityReactionType::None;
	}
	const UT4EntityAsset* EntityAsset = OwnerObject->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return ET4EntityReactionType::None;
	}
	if (ET4EntityType::Character != EntityAsset->GetEntityType())
	{
		return ET4EntityReactionType::None;
	}
	const UT4CharacterEntityAsset* CharacterEntity = Cast<const UT4CharacterEntityAsset>(EntityAsset);
	if (nullptr == CharacterEntity)
	{
		return ET4EntityReactionType::None;
	}
	const FT4EntityCharacterReactionSetData& ReactionSetData = CharacterEntity->ReactionSetData;
	if (!ReactionSetData.ReactionMap.Contains(InReactionName))
	{
		return ET4EntityReactionType::None;
	}
	return ReactionSetData.ReactionMap[InReactionName].ReactionType;
}