// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionNodeGraph.h"
#include "Public/Action/T4ActionCodeMinimal.h"
#include "Public/Action/T4ActionParameters.h"
#include "Public/T4Engine.h"
#include "T4Asset/Public/Action/T4ActionContiStructs.h"

/**
  *
 */
enum ET4ActionPlayState
{
	APS_Pending,
	APS_Ready, // #54 : APS_Playing_Offset, 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우!
	APS_Playing,
	APS_Stopping,
	APS_Stopped,
	APS_Finished,
};

enum ET4ActionLoadState
{
	ALS_NotUsed,
	ALS_Ready,
	ALS_Loading,
	ALS_Completed,
	ALS_Cancelled,
	ALS_Failed,
};

class AT4GameObject;
class IT4GameWorld;
class FT4ActionControl;
class FT4ActionNode : public IT4ActionNode
{
public:
	explicit FT4ActionNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionNode();

	// IT4ActionNode
	bool IsPlaying() const override; // #60 : IsPlaying 은 동작 상태를 뜻하고, Action 이 플레이 되었는지는 IsPlayed 를 사용할 것!
	bool IsLooping() const override { return (ET4LifecycleType::Looping == LifecycleType) ? true : false; }

	float GetElapsedTimeSec() const override { return ElapsedTimeSec; } // #102

	IT4ActionNode* GetParentNode() const override { return ParentNode; } // #23
	virtual const FName GetActionPoint() const override { return NAME_None; } // #63

	IT4ActionNode* AddChildNode(const FT4ActionStruct* InAction, float InOffsetTimeSec) override; // #23, #54
	bool RemoveChildNode(const FT4StopAction* InAction) override;

	uint32 NumChildActions() const override;

#if !UE_BUILD_SHIPPING
	bool IsDebugPaused() const override { return bDebugPaused; } // #56
	void SetDebugPause(bool bInPause); // #54
#endif

public:
	static FT4ActionKey FindActionKeyInParameter(const FT4ActionParameters* InParameters); // #100

	static FT4ActionNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParameters // #54
	);
	static FT4ActionNode* CreateNode(
		FT4ActionControl* InControl, 
		const ET4ActionType InActionType, // #56
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParameters // #54
	);
	static void DestroyNode(FT4ActionNode** InActionNode);

	bool OnCreate(const FT4ActionStruct* InAction);
	void OnDestroy();

	// #102 : 삭제 대기로 들어갈 경우 호출됨. 삭제 대기로 들어갈 경우 한프레임 늦게 삭제되기 때문에 일부 액션(Ex:TimeScale)
	//        의 원상복구가 되지 않는 문제가 있어 추가함
	void OnDestroying(); // #102

	void OnAdvance(const FT4UpdateTime& InUpdateTime);

	void OnPlay();
	void OnStop();

	void OnStartLoading();

	virtual const ET4ActionType GetType() const = 0;
	const FT4ActionKey& GetKey() const { return ActionKey; }

	bool IsDestroyable() const;
	bool IsPlayed() const { return (CheckPlayState(APS_Playing) || CheckPlayState(APS_Ready)) ? true : false; } // #60

	bool CheckFinished() const; // #56 : 하위 노드까지 검사하는 처리 추가
	bool CheckStopedAndFinished() const;
	   
	float GetPlayingTime() const { return FMath::Max(0.0f, ElapsedTimeSec - DelayTimeSec); }
	float GetPlayTimeLeft() const 
	{ 
		const float MaxPlayTimeSec = (0.0f < DurationSec) ? DurationSec : GetGlobalMaxPlayTimeSec();
		return FMath::Max(0.0f, MaxPlayTimeSec - GetPlayingTime()); 
	}
	float GetDesiredPlayTime() const 
	{ 
		return (0.0f < DurationSec) ? DelayTimeSec + DurationSec : GetGlobalMaxPlayTimeSec();
	}
	float GetOffsetTimeSec() const { return OffsetTimeSec; } // #54, #56

	float GetGlobalMaxPlayTimeSec() const; // #58
	float GetGlobalElapsedTimeSec() const { return GlobalElapsedTimeSec; } // #54

	void SetParentNode(FT4ActionNode* InParentNode) { ParentNode = InParentNode; }
	void SetActionParameters(TSharedPtr<const FT4ActionParameters> InActionParameterPtr); // #28
	void SetDelayTimeSec(float InDelayTimeSec) { DelayTimeSec = InDelayTimeSec; }
	void SetLifecycleType(ET4LifecycleType InLifecycleType, float InDurationSec);

	void SetOffsetTimeSec(float InOffsetTimeSec) 
	{ 
		// WARN : 가변값이기 때문에 FT4ActionParameters 로 Offset 값을 받아 생성 단계에서 값을 채우도록 처리됨.
		//        다른곳에서의 호출은 최대한 지양해야 함!
		OffsetTimeSec = FMath::Max(0.0f, InOffsetTimeSec - DelayTimeSec);  // #56 : Case-1
		ElapsedTimeSec = InOffsetTimeSec; // #54 : OffsetTime 만큼 Node 의 시간을 진행해주어야 한다.
	} 

	void SetGlobalPlayTimeSec(float InGlobalElapsedTimeSec, float InContiMaxPlayTimeSec)
	{
		// #54 : Action 중 명시적으로 Duration 을 사용하지 않고 Auto LifecycleType 를 사용할 경우 
		//       특정 리소스가 루핑으로 제작(Particle) 되면 무한 루핑 상태에 빠질 수 있어 
		//       ContiAsset 에 설정된 MaxPlayTimeSec 로 최대 시간을 제한하도록 추가
		//       MaxPlayTimeSec 는 Conti Editor 의 Timeline PlaybackEnd UI 와 연동되어 있음
		GlobalElapsedTimeSec = InGlobalElapsedTimeSec;
		ContiMaxPlayTimeSec = InContiMaxPlayTimeSec;
	}

	bool AddChildNodeInternal(const FT4ActionStruct* InAction, FT4ActionNode* InActionNode); // #23

protected:
	virtual bool Create(const FT4ActionStruct* InAction) { return true; }
	virtual void Destroy() {}

	virtual void Destroying() {} // #102

	virtual void AdvanceLoading(const FT4UpdateTime& InUpdateTime) {}
	virtual void Advance(const FT4UpdateTime& InUpdateTime) {}

	virtual bool Play() { return true; }
	virtual void Stop() {}

	virtual void StartLoading() {}
	virtual bool IsAutoFinished() const { return true; } // 기본은 모두 

	virtual bool PlayInternal(float InOffsetTimeSec) = 0; // #54, #56 : OffsetTime 의 중요성을 강조하기 위해 추상 인터페이스화 함!!

	AT4GameObject* GetGameObject() const;
	IT4GameWorld* GetGameWorld() const;

	ET4ActionPlayState GetPlayState() const { return PlayState; }
	ET4ActionLoadState GetLoadState() const { return LoadState; }

	bool CheckPlayState(ET4ActionPlayState InState) const { return (InState == PlayState) ? true : false; }
	bool CheckLoadState(ET4ActionLoadState InState) const { return (InState == LoadState) ? true : false; }

	void SetPlayState(ET4ActionPlayState InState) { PlayState = InState; }
	void SetLoadState(ET4ActionLoadState InState) { LoadState = InState; }

	// #54
	AT4GameObject* NewClientObject(
		ET4ObjectType InWorldObjectType, // #63 : Only World Object
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	);
	void DeleteClientObject();

	AT4GameObject* GetClientObject() const;
	// #54

	void AddOffsetTimeSec(float InOffsetTimeSec) { OffsetTimeSec += InOffsetTimeSec; } // #54 : Case-2

	bool CheckAbsoluteMaxPlayTime() const
	{ 
		if (0.0f < GetGlobalMaxPlayTimeSec())
		{
			return (GetGlobalMaxPlayTimeSec() <= GetGlobalElapsedTimeSec()); // #54, #58
		}
		return true;
	}

	bool FindTargetObjectInParameter(IT4GameObject** OutTargetObject, const TCHAR* InDebugString = nullptr); // #28
	bool FindTargetLocationInParameter(FVector& OutTargetLocation, const TCHAR* InDebugString = nullptr); // #28
	bool FindTargetDirectionInParameter(FVector& OutTargetDirection, const TCHAR* InDebugString = nullptr); // #28

#if !UE_BUILD_SHIPPING
	// #54, #102 : 디버깅용으로 Pause 가 되었을 경우 외부 오브젝트(Ex:Proj) 제어를 위한 호출
	//			   일반적인 상황에서는 ScaledTimeSec = 0 으로 Pause 를 구현하고 있다.
	virtual void NotifyDebugPaused(bool bInPause) {}
#endif

private:
	float GetContiMaxPlayTimeSec() const { return ContiMaxPlayTimeSec; } // #54, #58 : GetMaxPlayTimeSec 로 대체됨!

protected:
	FT4ActionKey ActionKey;

	FT4ActionControl* ActionControlRef;
	IT4ActionNode* ParentNode; // #23
	FT4ActionNodeGraph ActionNodeGraph; // #23

	TSharedPtr<const FT4ActionParameters> ActionParameterPtr; // #28

	ET4ActionPlayState PlayState;
	ET4ActionLoadState LoadState;
	
	bool bDebugPaused; // #56

	FT4ObjectID WorldObjectID; // #54 : ET4AttachParent::World 일 경우만 사용됨

	ET4LifecycleType LifecycleType;

	float ElapsedTimeSec;
	float DelayTimeSec;
	float DurationSec;

private:
	// * ActionNode 의 OffsetTimeSec 는 다음 두가지 경우에 시간이 설정된다.
	// Case-1 : ActionNode 생성 단계에서 OffsetTimeSec 이 넘어오는 경우 (#56)
	//          1) 로직단에서 패킷 지연으로 ActionNode 가 늦게 Execute 가 될 경우 타이밍을 맞추기 위해 OffsetTime 을 설정한다.
	//          2) 또는 T4RehearsalConti Editor 의 Timeline 에서 GlobalTime 조정 후 Play 를 할 경우 OffsetTime = GloablTime 이 된다.
	//          3) 이 경우 ActionNode 가 이미 DelayTimeSec 가 있다면 OffsetTimeSec - DelayTimeSec 로 초기 값을 설정해주어야 한다. (DelayTime 은 대기 시간이기 때문)
	//          4) Public:SetOffsetTimeSec 호출 (단, ActionParameter 로만 전달되어 NodeCreate 단계에서 설정됨)
	// Case-2 : 어셋 비동기 로딩으로 인해 Play 는 되었으나, 아직 리소스가 준비가 안되어 렌더링이 불가할 경우 (#54)
	//          1) 파티클을 플레이 시 즉시 출력되어야 하나 리소스 로딩이 안되었을 경우 APS_Ready Flag 를 켜고 로딩 완료까지 OffsetTime 을 설정한다.
	//          2) 만약, Case-1 의 OffsetTime 이 있다면 값을 더해서 설정해주어야 한다.
	//          3) Protected::AddOffsetTimeSec 호출
	float OffsetTimeSec; // #56 : ActionNode 에 따라 OffsetTimeSec 를 처리하는 방법이 틀릴 것임으로 유의할 것!

	// #54 : Action 중 명시적으로 Duration 을 사용하지 않고 Auto LifecycleType 를 사용할 경우 
	//       특정 리소스가 루핑으로 제작(Particle) 되면 무한 루핑 상태에 빠질 수 있어 
	//       ContiAsset 에 설정된 MaxPlayTimeSec 로 최대 시간을 제한하도록 추가
	//       MaxPlayTimeSec 는 Conti Editor 의 Timeline PlaybackEnd UI 와 연동되어 있음
	float GlobalElapsedTimeSec;
	float ContiMaxPlayTimeSec; // #58 : GetGlobalMaxPlayTimeSec() 에서 선택하도록 변경됨. 이 값은 콘티에 설정된 값!
};
