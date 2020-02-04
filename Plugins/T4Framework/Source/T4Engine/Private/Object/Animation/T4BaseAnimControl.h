// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineDefinitions.h"
#include "Public/T4Engine.h"

/**
  *
 */
class UT4AnimSetAsset;
class UT4BaseAnimInstance;
class AT4GameObject;
class IT4GameWorld;
struct FT4StateAnimVariables;
struct FT4MovementAnimVariables;
class FT4BaseAnimControl : public IT4AnimControl
{
public:
	explicit FT4BaseAnimControl(AT4GameObject* InGameObject);
	virtual ~FT4BaseAnimControl();

	// IT4AnimControl 
	// #47
	const IT4AnimState* GetActiveAnimState() const override;
	const IT4AnimState* GetPendingAnimState() const override;

	bool TryChangeAnimState(const FName& InAnimStateName, bool bInCheckPriorityActiveState, bool bInCheckPriorityPendingActiveState) override;
	void RegisterAnimState(const FName& InAnimStateName, IT4AnimState* InAnimState) override;
	void UnregisterAnimState(const FName& InAnimStateName) override;
	// ~#47

	bool HasSection(const FName& InAnimMontageName, const FName& InSectionName) override;
	float GetDurationSec(const FName& InAnimMontageName, const FName& InSectionName) override;

	bool IsPlayingAnimation(const FName& InAnimMontageName) override; // #116
	bool IsPlayingAnimation(FT4AnimInstanceID InPlayInstanceID) override;
	bool IsPlayingAndBlendOutStarted(FT4AnimInstanceID InPlayInstanceID) override; // #44

	FT4AnimInstanceID PlayAnimation(const FT4AnimParameters& InAnimParameters) override; // #38

	bool StopAnimation(const FName& InAnimMontageName, float InBlendOutTimeSec) override; // #38
	bool StopAnimation(FT4AnimInstanceID InPlayInstanceID, float InBlendOutTimeSec) override; // #38

#if !UE_BUILD_SHIPPING
	void DebugPauseAnimation(FT4AnimInstanceID InPlayInstanceID, bool bPause) override; // #54
#endif

#if WITH_EDITOR
	bool EditorPlayAnimation(
		UAnimSequence* InPlayAnimSequence,
		float InPlayRate,
		float InBlendInTimeSec,
		float InBlendOutTimeSec
	) override;
#endif

public:
	static FT4BaseAnimControl* CreateNewControl(AT4GameObject* InGameObject, ET4AnimInstance InAnimInstanceType);

	virtual void BeginPlay() {} // #18 : 모델 로딩이 완료된 시점에 1회 불림

	void OnReset(); // #38

	void OnAdvance(const FT4UpdateTime& InUpdateTime);

	virtual void SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset) {} // #38
	virtual void SetEnableFootIK(bool bInEnable) {}

	virtual void SetGoalRotation(const FRotator& InRotation) {} // #44

	virtual bool DoJump(const FVector& InVelocity) { return false; } // #46
	virtual bool DoRoll(const FVector& InVelocity) { return false; } // #46
	virtual bool DoTurn(const FRotator& InRotation) { return false; } // #47
	virtual bool DoVoid() { return false; } // #76

	AT4GameObject* GetOwnerObject() { return OwnerObjectPtr.Get(); }
	UT4BaseAnimInstance* GetAnimInstance();

	FT4StateAnimVariables* GetStateAnimVariables();
	FT4MovementAnimVariables* GetMovementAnimVariables();

	const FName GetActiveAnimStateName() const;

	// #47
	void OnAutoRegisterAnimStates();

	bool ChangeNextAnimState(const FName& InAnimStateName); // Current Frame
	// ~#47

protected:
	virtual void Reset() {} // #38
	virtual void Advance(const FT4UpdateTime& InUpdateTime) {}

	virtual void AdvanceAnimState(const FT4UpdateTime& InUpdateTime); // #47

	virtual void AutoRegisterAnimStates(); // #47

	virtual void NotifyPlayAnimation(const FT4AnimParameters& InAnimParameters) {} // #47

	bool IsLoaded() const;

	bool HasAnimState(const FName& InAnimStateName) const; // #48

	bool QueryIKLineTrace(
		const FVector& InStartLocation,
		const FVector& InEndLocation,
		const FCollisionQueryParams& InCollisionQueryParams,
		FVector& OutLocation
	);

	IT4GameWorld* GetGameWorld() const;

	// #47
	IT4AnimState* GetActiveAnimState();
	IT4AnimState* GetPendingAnimState();
	IT4AnimState* GetNextAnimState();
	// ~#47

private:
	void RemoveAnimStates(); // #47
	void ProcessPendingAnimState(); // #47

protected:
	ET4LayerType LayerType;
	bool bBegunPlay;
	TWeakObjectPtr<AT4GameObject> OwnerObjectPtr;

	// #47
	FName ActiveAnimStateName;
	FName NextAnimStateName;
	FName PendingAnimStateName;
	TMap<FName, IT4AnimState*> AnimStates;
	// ~#47
};
