// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4EngineTypes.h"
#include "Public/T4EngineLayer.h"
#include "Public/T4EngineStructs.h"
#include "Public/Action/T4ActionKey.h"

#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4Asset/Public/Entity/T4EntityKey.h"

#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
class IT4GameWorld;
class IT4WorldActor;

struct FT4AnimNotifyMessage; // #111
struct FT4ActionCommand;
struct FT4StopAction;
struct FT4SpawnActorAction;
struct FT4ActionParameters; // #28

#if !UE_BUILD_SHIPPING
class IT4ActionReplayPlayer; // #68
class IT4ActionReplayRecorder; // #68
class IT4ActionReplaySystem; // #68
#endif

class UT4EntityAsset;

struct FWorldContext;
class UAnimSequence;
class AController;
class APlayerController; // #114
class APlayerCameraManager; // #100

class T4ENGINE_API IT4AnimState
{
public:
	virtual ~IT4AnimState() {}

	virtual FName GetName() const = 0;
	virtual ET4AnimStatePriority GetPriority() const = 0;

	virtual void OnEnter() = 0;
	virtual void OnUpdate(const FT4UpdateTime& InUpdateTime) = 0;
	virtual void OnLeave() = 0;
};

class T4ENGINE_API IT4AnimControl
{
public:
	virtual ~IT4AnimControl() {}

	// #47
	virtual const IT4AnimState* GetActiveAnimState() const = 0;
	virtual const IT4AnimState* GetPendingAnimState() const = 0;

	virtual bool TryChangeAnimState(
		const FName& InAnimStateName,
		bool bInCheckPriorityActiveState,
		bool bInCheckPriorityPendingActiveState
	) = 0;

	virtual void RegisterAnimState(const FName& InAnimStateName, IT4AnimState* InAnimState) = 0;
	virtual void UnregisterAnimState(const FName& InAnimStateName) = 0;
	// ~#47

	virtual bool HasSection(const FName& InAnimMontageName, const FName& InSectionName) = 0;
	virtual float GetDurationSec(const FName& InAnimMontageName, const FName& InSectionName) = 0;

	virtual bool IsPlayingAnimation(const FName& InAnimMontageName) = 0; // #116
	virtual bool IsPlayingAnimation(FT4AnimInstanceID InPlayInstanceID) = 0;
	virtual bool IsPlayingAndBlendOutStarted(FT4AnimInstanceID InPlayInstanceID) = 0; // #44

	virtual FT4AnimInstanceID PlayAnimation(const FT4AnimParameters& InAnimParameters) = 0; // #38

	virtual bool StopAnimation(const FName& InAnimMontageName, float InBlendOutTimeSec) = 0; // #38
	virtual bool StopAnimation(FT4AnimInstanceID InPlayInstanceID, float InBlendOutTimeSec) = 0; // #47

#if !UE_BUILD_SHIPPING
	virtual void DebugPauseAnimation(FT4AnimInstanceID InPlayInstanceID, bool bInPause) = 0; // #54
#endif

#if WITH_EDITOR
	virtual bool EditorPlayAnimation(
		UAnimSequence* InPlayAnimSequence,
		float InPlayRate = 1.0f,
		float InBlendInTimeSec = T4Const_DefaultAnimBlendTimeSec,
		float InBlendOutTimeSec = T4Const_DefaultAnimBlendTimeSec
	) = 0; // #111
#endif
};

class T4ENGINE_API IT4ActionNode // #23
{
public:
	virtual ~IT4ActionNode() {}

	virtual bool IsPlaying() const = 0;
	virtual bool IsLooping() const = 0;

	virtual float GetElapsedTimeSec() const = 0; // #102

	virtual IT4ActionNode* GetParentNode() const = 0;
	virtual const FName GetActionPoint() const = 0; // #63

	virtual IT4ActionNode* AddChildNode(const FT4ActionCommand* InAction, float InOffsetTimeSec) = 0; // #23, #54
	virtual bool RemoveChildNode(const FT4StopAction* InAction) = 0;

	virtual uint32 NumChildActions() const = 0;

#if !UE_BUILD_SHIPPING
	virtual bool IsDebugPaused() const = 0;
#endif
};

class T4ENGINE_API IT4ActionControl // #23
{
public:
	virtual ~IT4ActionControl() {}

	virtual bool HasAction(const FT4ActionKey& InActionKey) const = 0; // #102

	virtual bool IsPlaying(const FT4ActionKey& InActionKey) const = 0;
	virtual bool IsLooping(const FT4ActionKey& InActionKey) const = 0;

	virtual float GetElapsedTimeSec(const FT4ActionKey& InActionKey) const = 0; // #102

	virtual IT4ActionNode* GetChildNodeByPrimary(const FT4ActionKey& InPrimaryActionKey) const = 0;
	virtual bool GetChildNodes(const FT4ActionKey& InSameActionKey, TArray<IT4ActionNode*>& OutNodes) const = 0;

	virtual uint32 NumChildActions() const = 0;
	virtual uint32 NumChildActions(const FT4ActionKey& InActionKey) const = 0; // #54
};

class T4ENGINE_API IT4WorldActor
{
public:
	virtual ~IT4WorldActor() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual ET4ObjectType GetObjectType() const = 0;

	virtual const FT4ActorID& GetActorID() const = 0;
	virtual const FName& GetName() const = 0;

	virtual const FT4EntityKey& GetEntityKey() const = 0; // #35
	virtual const UT4EntityAsset* GetEntityAsset() = 0; // #39

	virtual const FT4ObjectID& GetOwnerID() const = 0; // #114 : GameObject ID
	virtual void SetOwnerID(const FT4ObjectID& InObjectID) = 0; // #114
	virtual void ClearOwnerID() = 0; // #114

	virtual const FName GetStanceName() const = 0; // #73 : StanceNameTable
	virtual const FName GetSubStanceName() const = 0; // #106 : SubStanceNameTable

	virtual const FName& GetGameDataIDName() const = 0;

	virtual bool IsLoaded() const = 0; // #57 : 모든 로딩이 완료 된 상태
	virtual bool HasPlayer() const = 0;

	virtual APawn* GetPawn() = 0;
	virtual IT4GameWorld* GetGameWorld() const = 0; // #100

	virtual void OnAnimNotifyMessage(const FT4AnimNotifyMessage* InMessage) = 0; // #111

	virtual bool OnExecuteAction(const FT4ActionCommand* InAction, const FT4ActionParameters* InParam = nullptr) = 0; // #76

	virtual IT4AnimControl* GetAnimControl() const = 0; // #14
	virtual IT4ActionControl* GetActionControl() = 0; // #20, #76 : Action Public Manager
	virtual const FT4WorldActorProperty& GetPropertyConst() const = 0; // #34

	virtual float GetLifeTimeSec() const = 0; // #102
	virtual float GetTimeScale() const = 0; // #102

	virtual bool IsLockOn() const = 0; // #33
	virtual bool IsFalling() const = 0;
	virtual bool IsFlying() const = 0;
	virtual bool IsRolling() const = 0; // #46
	virtual bool IsTurning() const = 0; // #46
	virtual bool IsCombat() const = 0; // #109
	virtual bool IsAiming() const = 0; // #113

	virtual bool HasPlayingAnimState(const FName& InAnimStateName) const = 0; // #47

	virtual bool HasAction(const FT4ActionKey& InActionKey) const = 0; // #102 : 존재만 해도 true 리턴
	virtual bool IsPlayingAction(const FT4ActionKey& InActionKey) const = 0; // #20, #76 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	virtual const FVector GetCOMLocation() const = 0; // #18 : WARN : Center of mass 캐릭터의 경우 Coll Capsule 의 중점이다.
	virtual const FVector GetRootLocation() const = 0;
	virtual const FVector GetNavPoint() const = 0; // #52

	virtual const FRotator GetRotation() const = 0;
	virtual const FVector GetFrontVector() const = 0; // #38
	virtual const FVector GetRightVector() const = 0; // #38

	virtual const FVector GetMovementVelocity() const = 0;
	virtual const float GetMovementSpeed() const = 0;

	virtual bool GetVisible() const = 0; // #117
	virtual const float GetOpacity() const = 0; // #78

	virtual bool HasReaction(const FName& InReactionName) const = 0; // #73
	virtual bool HasPlayTag(const FName& InPlayTagName, ET4PlayTagType InPlayTagType) const = 0; // #81
	virtual bool HasActionPoint(const FName& InActionPoint) const = 0; // #57 : ActionPoint = Socket or Bone or VirtualBone
	virtual bool HasEquipment(const FT4ActionKey& InActionKey) const = 0; // #111

	virtual bool GetSocketLocation(const FName& InSocketName, FVector& OutLocation) const = 0; // #18
	virtual bool GetSocketRotation(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FRotator& OutRotation) const = 0; // #18
	virtual bool GetSocketScale(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FVector& OutScale) const = 0; // #54
	virtual bool GetSocketTransform(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FTransform& OutTransform) const = 0; // #58

	virtual void SetHeightOffset(float InOffset) = 0; // #18
	virtual void SetOutline(bool bInUse) = 0; // #115
	virtual void SetNameplateText(const TCHAR* InText, float InHeightOffset, const FColor& InTextColor, float InScaleXY) = 0; // #119 : InText == nullptr Hide

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual FT4ServerWorldActorDelegates& GetServerDelegates() = 0; // #49

	virtual void BeginWeaponHitOverlapEvent(const FName& InHitOverlapEventName) = 0; // $49
	virtual void EndWeaponHitOverlapEvent() = 0; // #49
#endif

#if !UE_BUILD_SHIPPING
	virtual FT4WorldActorDebugInfo& GetDebugInfo() = 0; // #76

	virtual void SetDebugPause(bool bInPause) = 0; // #102
#endif

#if WITH_EDITOR
	virtual void EditorSetAimTarget(bool bEnable, const FVector& InAimTarget) = 0; // #111
	virtual bool EditorPlayAnimation(
		UAnimSequence* InPlayAnimSequence,
		FName InSectionName,
		float InPlayRate = 1.0f,
		float InBlendInTimeSec = T4Const_DefaultAnimBlendTimeSec,
		float InBlendOutTimeSec = T4Const_DefaultAnimBlendTimeSec
	) = 0; // #111
#endif
};

class T4ENGINE_API IT4WorldContainer // #87
{
public:
	virtual ~IT4WorldContainer() {}

	virtual uint32 GetNumWorldActors() const = 0;
	virtual bool GetWorldActors(ET4SpawnMode InSpawnType, TArray<IT4WorldActor*>& OutWorldActors) = 0; // #68

	virtual bool HasWorldActor(const FT4ActorID& InActorID) const = 0;;
	virtual IT4WorldActor* FindWorldActor(const FT4ActorID& InActorID) const = 0;

	virtual bool QueryNearestWorldActors(
		const FVector& InOriginLocation,
		const float InMaxDistance,
		TArray<IT4WorldActor*>& OutActors
	) = 0; // #34

	// #54 : 현재는 ClientOnly
	virtual IT4WorldActor* PlayWorldExtraActor(
		ET4ObjectType InWorldActorType,
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	) = 0; // #68 : 소멸 조건이 되면 스스로 소멸한다.

	virtual IT4WorldActor* CreateWorldExtraActor(
		ET4ObjectType InWorldActorType, // #63 : Only World Object
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	) = 0;
	virtual bool DestroyWorldExtraActor(const FT4ActorID& InActorID) = 0;
	// ~#54 : 현재는 ClientOnly
};

class T4ENGINE_API IT4WorldCollisionSystem // #87
{
public:
	virtual ~IT4WorldCollisionSystem() {}

	virtual bool QueryLineTraceSingle(
		ET4CollisionChannel InCollisionChannel,
		const FVector& InStartLocation,
		const FVector& InEndLocation,
		const FCollisionQueryParams& InCollisionQueryParams,
		FT4HitSingleResult& OutHitResult
	) = 0;

	virtual bool QueryLineTraceSingle(
		ET4CollisionChannel InCollisionChannel,
		const FVector& InStartLocation,
		const FVector& InStartDirection,
		const float InMaxDistance,
		const FCollisionQueryParams& InCollisionQueryParams,
		FT4HitSingleResult& OutHitResult
	) = 0;
};

class T4ENGINE_API IT4WorldNavigationSystem // #87
{
public:
	virtual ~IT4WorldNavigationSystem() {}

	virtual bool ProjectPoint(const FVector& InGoal, const FVector& InExtent, FVector& OutLocation) = 0; // #31 // INVALID_NAVEXTENT, FVector::ZeroVector

	virtual bool HasReached(const FVector& InStartLocation, const FVector& InEndLocation) = 0; // #52

	virtual bool GetRandomLocation(FVector& OutLocation) = 0; // #87
	virtual bool GetRandomLocation(const FVector& InOrigin, float InMaxRadius, FVector& OutLocation) = 0; // #31
};

class T4ENGINE_API IT4WorldController // #87
{
public:
	virtual ~IT4WorldController() {}

	virtual ET4GameWorldType GetGameWorldType() const = 0; // #87

	virtual bool CheckLevelLoadComplated() = 0; // #87

	virtual UWorld* GetWorld() const = 0;

	// #93
	virtual FName GetGameTimeTagName() const = 0;
	virtual FString GetGameTimeString() = 0;

	virtual void SetGameTimeHour(float InHour) = 0;
	virtual float GetGameTimeHour() const = 0;

	virtual void SetGameTimeScale(float InScale) = 0;
	virtual float GetGameTimeScale() const = 0;
	// ~#93

#if WITH_EDITOR
	virtual bool IsPreviewScene() const = 0; // #87
#endif
};

class AT4EditorCameraActor; // #58
class T4ENGINE_API IT4GameWorld
{
public:
	virtual ~IT4GameWorld() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual ET4WorldType GetType() const = 0;

	virtual void OnReset() = 0;

	virtual void OnProcessPre(float InDeltaTime) = 0; // #34 : OnWorldPreActorTick
	virtual void OnProcessPost(float InDeltaTime) = 0; // #34 : OnWorldPostActorTick

	virtual bool OnExecuteAction(const FT4ActionCommand* InAction, const FT4ActionParameters* InActionParam = nullptr) = 0;

	virtual ET4GameWorldType GetGameWorldType() const = 0; // #87
	virtual const FName GetEntityKeyName() const = 0; // #100 : 현재 로딩된 Entity KeyName 만약, 없다면 NAME_None (preview or Level 을 직접) 로 리턴됨

	virtual UWorld* GetWorld() const = 0;
	virtual const FSoftObjectPath& GetWorldActorPath() const = 0; // #104

	virtual IT4WorldController* GetController() = 0; // #87
	virtual IT4WorldContainer* GetContainer() = 0; // #87
	virtual IT4WorldCollisionSystem* GetCollisionSystem() = 0; // #87
	virtual IT4WorldNavigationSystem* GetNavigationSystem() = 0; // #87

	// Client Only
	virtual bool SetPlayerInfo(const FT4ObjectID& InPlayerObjectID, APlayerController* InPlayerController) = 0; // AT4PlayerController
	virtual bool HasPlayerController() const = 0; // #115 : PC가 스폰되어야 렌더링이 가능하다.
	virtual const FT4ObjectID GetPlayerObjectID() const = 0;

	virtual APlayerController* GetPlayerController() const = 0; // #114
	virtual APlayerCameraManager* GetPlayerCameraManager() const = 0; // #114

	virtual bool HasPlayerActor() const = 0;
	virtual IT4WorldActor* GetPlayerActor() const = 0;
	virtual bool ComparePlayerActor(const FT4ActorID& InActorID) const = 0;
	virtual bool ComparePlayerActor(IT4WorldActor* InWorldActor) const = 0;

	virtual bool SetMPCGlobalParameterScalar(FName InParameterName, const float InScalar) = 0; // #115
	virtual bool SetMPCGlobalParameterColor(FName InParameterName, const FLinearColor& InValue) = 0; // #115

	virtual FVector GetCameraLocation() const = 0;
	virtual FRotator GetCameraRotation() const = 0;

	virtual IT4WorldActor* GetIndicatorObject() = 0; // #117

#if !UE_BUILD_SHIPPING
	virtual IT4ActionReplayPlayer* GetActionReplayPlayer() const = 0; // #68
	virtual IT4ActionReplayRecorder* GetActionReplayRecorder() const = 0;
	virtual IT4ActionReplaySystem* GetActionReplaySystem() = 0;
#endif

#if WITH_EDITOR
	virtual AT4EditorCameraActor* FindOrCreateEditorCameraActor(
		uint32 InKey, 
		bool bInCreate,
		bool bInEmulMode
	) = 0; // #58 : Only Client
	virtual void DestroyEditorCameraActor(uint32 InKey) = 0; // #58 : Only Client

	virtual bool IsDisabledLevelStreaming() const = 0; // #86, #104
	virtual void SetDisableLevelStreaming(bool bInDisable) = 0; // #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	virtual void SetDisableEnvironmentUpdating(bool bInDisable) = 0; // #92 : Map Environemnt Update 제어 옵션 처리
	virtual void SetDisableTimelapse(bool bInDisable) = 0; // #93 : 시간 경과 옵션 처리
	virtual bool GetTimelapseDisabled() const = 0; // #94
#endif
};

// #87
DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnGameWorldTravel, IT4GameWorld*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FT4OnGameWorldTimeTransition, IT4GameWorld*, const FName); // #93
class T4ENGINE_API FT4EngineDelegates
{
public:
	static FT4OnGameWorldTravel OnGameWorldTravelPre; // #87 : 월드 이동 ActionReplay 지원
	static FT4OnGameWorldTravel OnGameWorldTravelPost; // #87 : 월드 이동 ActionReplay 지원

	static FT4OnGameWorldTimeTransition OnGameWorldTimeTransition; // #93 : 월드 TimeName 변경 알림

private:
	FT4EngineDelegates() {}
};

T4ENGINE_API IT4GameWorld* T4EngineWorldCreate(
	ET4WorldType InWorldType,
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
);
T4ENGINE_API void T4EngineWorldDestroy(IT4GameWorld* InGameWorld);

T4ENGINE_API IT4GameWorld* T4EngineWorldGet(ET4LayerType InLayerType);
