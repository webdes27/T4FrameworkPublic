// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionContiNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h" // #24

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionContiNode::FT4ActionContiNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionCompositeNodeBase(InControl, InKey)
	, LoadingPolicy(ET4LoadingPolicy::Default) // #56
{
}

FT4ActionContiNode::~FT4ActionContiNode()
{
	check(AssetLoader.CheckReset());
}

FT4ActionContiNode* FT4ActionContiNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4ContiAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Conti == InAction.ActionType);
	
	// #54
	if (InAction.ContiAsset.IsNull())
	{
		T4_LOG(
			Error,
			TEXT("CreateNode Failed. Not set ContiAsset.")
		);
		return nullptr;
	}

	FT4ActionContiNode* NewNode = new FT4ActionContiNode(InControl, InAction.ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionContiNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Conti == InAction->ActionType);
	const FT4ContiAction& ConvAction = *(static_cast<const FT4ContiAction*>(InAction));
	LoadingPolicy = ConvAction.LoadingPolicy;
	if (ET4LoadingPolicy::Default == LoadingPolicy)
	{
		LoadingPolicy = ET4LoadingPolicy::Async;
	}
	AssetPath = ConvAction.ContiAsset.ToSoftObjectPath();
	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading(); // #8, #54 : 생성 시점에 무조건 비동기 로딩을 걸어준다.
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionContiNode::Destroy()
{
	AssetLoader.Reset();
}

void FT4ActionContiNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
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

void FT4ActionContiNode::Advance(const FT4UpdateTime& InUpdateTime)
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
}

bool FT4ActionContiNode::Play()
{
	check(!AssetLoader.IsBinded());

	// #56 : TDOO OffsetTinmeSec

	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(AssetPath.TryLoad());
		if (nullptr != ContiAsset)
		{
			ProcessCompositeData(
				ContiAsset->CompositeData, 
				ContiAsset->TotalPlayTimeSec, // #54
				GetOffsetTimeSec() // #54, #56 : OffsetTime
			); 
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
				TEXT("Failed to load ContiAsset '%s'."),
				*(AssetPath.ToString())
			);
			OnStop();
			return false;
		}
		else
		{
			// #56 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
			//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
			SetPlayState(APS_Ready);
			return false;
		}
	}
	else
	{
		check(false); // ??
	}

	return true;
}

void FT4ActionContiNode::Stop()
{
	AssetLoader.Reset();
}

void FT4ActionContiNode::StartLoading()
{
	AssetLoader.Load(AssetPath, false, TEXT("ActionContiNode"));
}

bool FT4ActionContiNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	return (!CheckLoadState(ALS_Loading)) ? true : false;
}

bool FT4ActionContiNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!AssetLoader.IsBinded());
	check(AssetLoader.IsLoadCompleted());
	UT4ContiAsset* ContiAsset = AssetLoader.GetContiAsset();
	check(nullptr != ContiAsset);
	ProcessCompositeData(
		ContiAsset->CompositeData, 
		ContiAsset->TotalPlayTimeSec, // #54
		InOffsetTimeSec
	);
	AssetLoader.SetBinded();
	SetPlayState(APS_Playing);
	return true;
}
