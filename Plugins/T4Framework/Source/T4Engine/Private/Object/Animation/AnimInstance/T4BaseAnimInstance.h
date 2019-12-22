// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineTypes.h"
#include "Public/T4EngineStructs.h"
#include "Animation/AnimInstance.h"
#include "T4BaseAnimInstance.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
class UAnimMontage;
class UBlendSpaceBase;
class UT4AnimSetAsset;
struct FT4MovementAnimVariables;
struct FT4IKAnimVariables;
UCLASS()
class UT4BaseAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	virtual ~UT4BaseAnimInstance();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	DECLARE_DELEGATE_OneParam(FT4OnAnimNotify, FName); // #38

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	bool HandleNotify(const FAnimNotifyEvent& InAnimNotifyEvent) override; // #38

public:
	void OnReset(); // #38

	virtual const ET4AnimInstance GetAnimInstanceType() const { return ET4AnimInstance::None; }

	virtual FT4MovementAnimVariables* GetMovementAnimVariables() { return nullptr; }
	virtual FT4IKAnimVariables* GetIKAnimVariables() { return nullptr; }

	bool IsValidSection(const FName& InAnimMontageName, const FName& InSectionName);
	float GetDurationSec(const FName& InAnimMontageName, const FName& InSectionName);

	bool IsPlaying(FT4AnimInstanceID InPlayInstanceID);
	bool IsPlayingAndBlendOutStarted(FT4AnimInstanceID InPlayInstanceID); // #44

	FT4AnimInstanceID PlayAnimation(const FT4AnimParameters& InAnimParameters); // #38

	bool StopAnimation(const FName& InAnimMontageName, float InBlendOutTimeSec); // #38
	bool StopAnimation(FT4AnimInstanceID InPlayInstanceID, float InBlendOutTimeSec); // #47

	void SetPause(bool bInPause); // #63
	void SetTimeScale(float InTimeScale); // #102

#if WITH_EDITOR
	bool PlayAnimation(
		UAnimSequence* InPlayAnimSequence,
		float InPlayRate,
		float InBlendInTimeSec,
		float InBlendOutTimeSec
	); // #39
#endif

#if !UE_BUILD_SHIPPING
	void DebugPauseAnimation(FT4AnimInstanceID InPlayInstanceID, bool bPause); // #54
#endif

	void SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset); // #39, #38
	void AddAnimMontage(const FName& InName, UAnimMontage* InAnimMontage); // #39, #38, #69
	void AddBlendSpace(const FName& InName, UBlendSpaceBase* InBlendSpace); // #39, #38

	FT4OnAnimNotify& GetOnAnimNotify() { return OnAnimNotify; } // #38

protected:
	virtual void Reset() {} // #38

	virtual void PreUpdateAnimation(float DeltaSeconds) override;

	FT4AnimInstanceID PlayAnimationInternal(
		UAnimMontage* InMontageToPlay, 
		const FT4AnimParameters& InAnimParameters,
		const TCHAR* InDebugString
	);

	bool GetSectionLength(UAnimMontage* InMontage, const FName& InSectionName, float& OutLength);

	UFUNCTION()
	void HandleOnAnimMontageEnded(UAnimMontage* InMontage, bool bInterrupted);

protected:
	UPROPERTY()
	UT4AnimSetAsset* AnimSetAsset; // #39

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Montage, Meta = (AllowPrivateAccess = true))
	TMap<FName, UAnimMontage*> AnimMontages;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Blandspace, Meta = (AllowPrivateAccess = true))
	TMap<FName, UBlendSpaceBase*> BlendSpaces;

private:
	FT4OnAnimNotify OnAnimNotify;

	bool bPaused; // #63
	float TimeScale; // #102

	struct FT4PlayAnimInstanceInfo // #102
	{
		FT4AnimInstanceID AnimInstanceID;
		float PlayRate;
	};
	TMap<FT4AnimInstanceID, FT4PlayAnimInstanceInfo> PlayAnimInstanceInfoMap; // #102

#if WITH_EDITOR
	UAnimMontage* EditorDynamicMontage; // #71
#endif
};
