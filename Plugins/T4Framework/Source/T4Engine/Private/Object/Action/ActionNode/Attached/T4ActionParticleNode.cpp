// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionParticleNode.h"

#include "Object/Component/T4ParticleSystemComponent.h"
#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionParticleNode::FT4ActionParticleNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionAttachedNode(InControl, InKey)
	, Scale(FVector::OneVector) // #54
	, PlayRate(1.0f)
	, bAutoFinished(false) //#54
	, bParticleLooping(false) // #54 : 에셋에서 루핑 사용중인지 체크
{
	WorldObjectType = ET4ObjectType::World_Default; // #54, #63
}

FT4ActionParticleNode::~FT4ActionParticleNode()
{
	check(AssetLoader.CheckReset());
}

FT4ActionParticleNode* FT4ActionParticleNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4ParticleAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Particle == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionParticleNode* NewNode = new FT4ActionParticleNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionParticleNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::Particle == InAction->ActionType);
	const FT4ParticleAction& ConvAction = *(static_cast<const FT4ParticleAction*>(InAction));
	Scale = ConvAction.Scale; // #54
	PlayRate = ConvAction.PlayRate; // #56
	SetAttachInfo(
		ConvAction.AttachParent, // #54
		ConvAction.bParentInheritPoint, // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
		ConvAction.ActionPoint, // #57 : BoneOrSocketName
		ConvAction.ParticleAsset.ToSoftObjectPath(),
		ConvAction.LoadingPolicy
	);
	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading();
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionParticleNode::Destroy()
{
	ensure(!ParticleSystemComponentPtr.IsValid());
	AssetLoader.Reset();
}

void FT4ActionParticleNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
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

void FT4ActionParticleNode::Advance(const FT4UpdateTime& InUpdateTime)
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
		if (ParticleSystemComponentPtr.IsValid())
		{
			ParticleSystemComponentPtr->SetPlayRate(InUpdateTime.TimeScale); // #102
		}
		if (!bAutoFinished)
		{
			if (ParticleSystemComponentPtr.IsValid())
			{
				if (ET4LifecycleType::Auto == LifecycleType && bParticleLooping)
				{
					// #54 : Conti 로 플레이 할 경우 설정되는 최대 플레이시간 설정
					//       Duration 을 사용하지 않을 경우 리소스에 따라 무한대 플레이가 될 수 있어
					//       ContiAsset 에 TotalPlayTimeSec 를 설정하도록 처리하였음.
					//       이 시간은 Conti 플레이 시간부터의 제한 시간임으로 ContiPlayTimeSec 로 시작 시간을 알려준다.
					bAutoFinished = CheckAbsoluteMaxPlayTime();
				}
				if (!bAutoFinished)
				{
					// #56 : ParticleSystemComponentPtr->bAutoDestroy = false; 끄고 종료 여부를 쿼리하는 것으로 변경
					bAutoFinished = ParticleSystemComponentPtr->IsReadyForOwnerToAutoDestroy();
				}
			}
			else
			{
				bAutoFinished = true;
			}
		}
	}
}

bool FT4ActionParticleNode::Play()
{
	check(!AssetLoader.IsBinded());
	check(!ParticleSystemComponentPtr.IsValid());
	ParticleSystemComponentPtr = NewComponentTemplate<UT4ParticleSystemComponent>(true);
	{
		ParticleSystemComponentPtr->SetWorldScale3D(Scale); // #54
		ParticleSystemComponentPtr->bAutoActivate = false; // #56  :로딩 완료 후 ActivateSystem 를 직접 호출한다.
		//ParticleSystemComponentPtr->bAllowRecycling; // #56 : Looping ?
		//ParticleSystemComponentPtr->bAutoDestroy = false;
	}
	OnAttachToParent(Cast<USceneComponent>(ParticleSystemComponentPtr.Get()), false);
	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UParticleSystem* ParticleSystem = Cast<UParticleSystem>(AssetPath.TryLoad());
		if (nullptr != ParticleSystem)
		{
			ParticleSystemComponentPtr->SetTemplate(ParticleSystem);
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
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("FT4ActionParticleNode::Play : Failed to load ParticleSystem '%s'"),
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

void FT4ActionParticleNode::Stop()
{
	AssetLoader.Reset();
	if (!ParticleSystemComponentPtr.IsValid())
	{
		return;
	}
	OnDetachFromParent(Cast<USceneComponent>(ParticleSystemComponentPtr.Get()), false);
	ParticleSystemComponentPtr->DestroyComponent();
	ParticleSystemComponentPtr.Reset();
}

void FT4ActionParticleNode::StartLoading()
{
	AssetLoader.Load(AssetPath, false, TEXT("ParticleNode"));
}

bool FT4ActionParticleNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	return bAutoFinished; // #54
}

bool FT4ActionParticleNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!AssetLoader.IsBinded());
	check(AssetLoader.IsLoadCompleted());
	check(ParticleSystemComponentPtr.IsValid());
	UParticleSystem* ParticleSystem = AssetLoader.GetParticleSystem();
	check(nullptr != ParticleSystem);
	ParticleSystemComponentPtr->CustomTimeDilation = PlayRate; // #56
	ParticleSystem->WarmupTime = InOffsetTimeSec; // #56 : Apply OffsetTime!!
	ParticleSystemComponentPtr->SetTemplate(ParticleSystem);
	ParticleSystemComponentPtr->ActivateSystem(); // #56
	bParticleLooping = ParticleSystemComponentPtr->Template->IsLooping(); // #54 : 에셋에서 루핑 사용중인지 체크
	AssetLoader.SetBinded();
	SetPlayState(APS_Playing);
	return true;
}
