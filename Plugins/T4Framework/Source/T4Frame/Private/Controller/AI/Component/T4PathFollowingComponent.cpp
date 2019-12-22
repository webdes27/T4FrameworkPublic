// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/Component/T4PathFollowingComponent.h"

#include "T4Engine/Public/T4Engine.h"

#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Navigation/MetaNavMeshPath.h"
#include "NavLinkCustomInterface.h"

#include "T4FrameInternal.h"

/**
  *
 */
UT4PathFollowingComponent::UT4PathFollowingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
{
	bIsDecelerating = false;
}

void UT4PathFollowingComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4PathFollowingComponent::BeginPlay()
{
	Super::BeginPlay();

	check(ET4LayerType::Max == LayerType);
	LayerType = T4EngineLayer::Get(GetWorld()); // #12 : Support Multiple LayerType
	check(ET4LayerType::Max != LayerType);
}

void UT4PathFollowingComponent::UpdatePathSegment()
{
	Super::UpdatePathSegment();
}

void UT4PathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	if (!Path.IsValid() || MovementComp == nullptr)
	{
		return;
	}
	
	if (!TargetObjectID.IsValid())
	{
		return;
	}
	check(ET4LayerType::Max != LayerType);

	// refer : UPathFollowingComponent::FollowPathSegment

	IT4GameObject* TargetObject = GetGameObject();
	check(nullptr != TargetObject);

	const FVector NavLocation = TargetObject->GetNavPoint();
	FVector CurrentTarget = GetCurrentTargetLocation();
	CurrentTarget.Z = NavLocation.Z;

	// #52 : C/S 레이턴시로 인해 오차가 발생할 수 밖에 없음으로 StoC 에서는 TargetLocation 을 받고,
	//       Velocity 로 변환해 처리하도록 수정
	//FVector MoveVelocity = (CurrentTarget - CurrentNavLocation) / DeltaTime;
	FVector MoveVelocity = CurrentTarget - NavLocation;
	if (!MoveVelocity.IsNearlyZero())
	{
		const int32 LastSegmentStartIndex = Path->GetPathPoints().Num() - 2;
		const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);
		GetOnCallbackMoveTo().ExecuteIfBound(MoveVelocity, bNotFollowingLastSegment); // #34, #50, #52
	}
}

void UT4PathFollowingComponent::OnPathFinished(const FPathFollowingResult& Result) // #52
{
	bool bStopMovementOnFinishCached = bStopMovementOnFinish;
	{
		// #52 : MovementComp->StopMovementKeepPathing(); 호출을 우회하기 위한 처리 Hack!!
		bStopMovementOnFinish = false;
		Super::OnPathFinished(Result);
		GetOnCallbackMoveStop().ExecuteIfBound(); // #52 : Stop 패킷 전송!
	}
	bStopMovementOnFinish = bStopMovementOnFinishCached;
}

void UT4PathFollowingComponent::OnSegmentFinished() // #52
{
	Super::OnSegmentFinished();
}

void UT4PathFollowingComponent::OnPathUpdated() // #52
{
	Super::OnPathUpdated();
}

IT4GameObject* UT4PathFollowingComponent::GetGameObject() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	return GameWorld->GetContainer()->FindGameObject(TargetObjectID);
}