// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionDecalNode.h"

#include "Object/Component/T4DecalComponent.h"
#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

static FRotator DefaultDecalRelativeRotaiton(-90.0f, 0.0f, 0.0f); // #54, #104 : decal 기본 축을 +X 축으로 맞춰준다.

/**
  *
 */
FT4ActionDecalNode::FT4ActionDecalNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionAttachedNodeBase(InControl, InKey)
	, Scale(FVector::OneVector)
	, DecalSortOrder(0)
	, DecalSize(FVector::ZeroVector)
	, FadeInTimeSec(0.0f)
	, FadeOutTimeSec(0.0f)
{
	WorldObjectType = ET4ObjectType::World_Default; // #54, #63
}

FT4ActionDecalNode::~FT4ActionDecalNode()
{
	check(AssetLoader.CheckReset());
}

FT4ActionDecalNode* FT4ActionDecalNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4DecalAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Decal == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionDecalNode* NewNode = new FT4ActionDecalNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionDecalNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Decal == InAction->ActionType);
	const FT4DecalAction& ConvAction = *(static_cast<const FT4DecalAction*>(InAction));
	Scale = ConvAction.Scale;
	DecalSortOrder = ConvAction.DecalSortOrder;
	DecalSize = ConvAction.DecalSize;
	FadeInTimeSec = ConvAction.FadeInTimeSec;
	FadeOutTimeSec = ConvAction.FadeOutTimeSec;
	SetAttachInfo(
		ConvAction.AttachParent, // #54
		ConvAction.bParentInheritPoint, // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
		ConvAction.ActionPoint, // #57 : BoneOrSocketName
		ConvAction.DecalMaterial.ToSoftObjectPath(),
		ConvAction.LoadingPolicy
	);
	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading();
	}
	if (ET4AttachParent::World == ConvAction.AttachParent)
	{
		bInheritRotation = false; // #54 : 회전은 무시한다.
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionDecalNode::Destroy()
{
	ensure(!DecalComponentPtr.IsValid());
	AssetLoader.Reset();
}

void FT4ActionDecalNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
{
	if (AssetLoader.IsLoadFailed()) // Failed 먼저 체크!!
	{
		SetLoadState(ALS_Failed);
	}
	else if (AssetLoader.IsLoadCompleted())
	{
		SetLoadState(ALS_Completed);
	}
}

void FT4ActionDecalNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (CheckPlayState(APS_Ready))
	{
		// #56
		check(ET4LoadingPolicy::Async == LoadingPolicy);
		AddOffsetTimeSec(InUpdateTime.ScaledTimeSec); // #54 : Case-2
		if (CheckLoadState(ALS_Failed))
		{
			SetPlayState(APS_Stopped);
		}
		else if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
	}
	if (IsPlayed())
	{
		if (DecalComponentPtr.IsValid())
		{
			DecalComponentPtr->SetPlayRate(InUpdateTime.TimeScale); // #102

			if (!IsLooping()) // #117 : TODO : 루핑 종료 인터럽트 발생시 FadeOut 처리 필요!
			{
				if (0.0f < FadeOutTimeSec && GetPlayTimeLeft() <= FadeOutTimeSec)
				{
					DecalComponentPtr->SetFadeOut(0.0f, GetPlayTimeLeft(), false);
					FadeOutTimeSec = 0.0f;
				}
			}
		}
	}
}

bool FT4ActionDecalNode::Play()
{
	check(!AssetLoader.IsBinded());

	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);

	check(!DecalComponentPtr.IsValid());
	DecalComponentPtr = NewComponentTemplate<UT4DecalComponent>(OwnerGameObject->GetPawn(), true);
	{
		DecalComponentPtr->bDestroyOwnerAfterFade = false; // Parent 를 죽이는 매우 요상한 코드 동작!
		DecalComponentPtr->DecalSize = DecalSize;
		DecalComponentPtr->SetVisibility(false); // #60 : 로딩 전까지는 안보이도록 처리!
		DecalComponentPtr->SetWorldScale3D(Scale);

		WorldInheritRotation = DefaultDecalRelativeRotaiton; // #54, #104 : decal 기본 축을 +X 축으로 맞춰준다.
		DecalComponentPtr->SetRelativeRotation(DefaultDecalRelativeRotaiton);
	}
	OnAttachToParent(Cast<USceneComponent>(DecalComponentPtr.Get()), false);
	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(AssetPath.TryLoad());
		if (nullptr != MaterialInterface)
		{
			DecalComponentPtr->SetDecalMaterial(MaterialInterface);
		}
	}
	else if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
		else if (CheckLoadState(ALS_Failed))
		{
			T4_LOG(
				Error,
				TEXT("Play Failed to load DecalMaterial '%s'."),
				*(AssetPath.ToString())
			);
			OnStop();
			return false;
		}
		else
		{
			// #54 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
			//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
			SetPlayState(APS_Ready);
			return false;
		}
	}
	return true;
}

void FT4ActionDecalNode::Stop()
{
	AssetLoader.Reset();
	if (!DecalComponentPtr.IsValid())
	{
		return;
	}
	OnDetachFromParent(Cast<USceneComponent>(DecalComponentPtr.Get()), false);
	DecalComponentPtr->DestroyComponent();
	DecalComponentPtr.Reset();
}

void FT4ActionDecalNode::StartLoading()
{
	AssetLoader.Load(AssetPath, false, TEXT("DecalNode"));
}

bool FT4ActionDecalNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionDecalNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!AssetLoader.IsBinded());
	check(AssetLoader.IsLoadCompleted());
	check(DecalComponentPtr.IsValid());
	UMaterialInterface* MaterialInterface = AssetLoader.GetMaterialInterface();
	check(nullptr != MaterialInterface);
	DecalComponentPtr->SetDecalMaterial(MaterialInterface);
	DecalComponentPtr->SetVisibility(true); // #60 : 로딩 전까지는 안보이도록 처리!
	DecalComponentPtr->SetSortOrder(DecalSortOrder);
	{
		if (0.0f < FadeInTimeSec && InOffsetTimeSec < FadeInTimeSec)
		{
			DecalComponentPtr->SetFadeIn(0.0f, FMath::Max(0.0f, FadeInTimeSec - InOffsetTimeSec));
		}
	}
	AssetLoader.SetBinded();
	SetPlayState(APS_Playing);
	return true;
}
