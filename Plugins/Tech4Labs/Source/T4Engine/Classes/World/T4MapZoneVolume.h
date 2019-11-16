// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/PostProcessVolume.h"

#include "T4MapZoneVolume.generated.h"

/**
  * #92
 */
class UT4MapEnvironmentAsset;
UCLASS(ClassGroup = Tech4Labs, Category = "Tech4Labs")
class T4ENGINE_API AT4MapZoneVolume : public APostProcessVolume
{
	GENERATED_UCLASS_BODY()

public:
	virtual void PostUnregisterAllComponents();

public:
	//~ Begin UObject interface
	virtual void BeginPlay() override;
	virtual void PostRegisterAllComponents() override;

	virtual void Serialize(FArchive& Ar) override;

public:
	void Update(float InDeltaTime);

	void Enter();
	void Leave();

	bool IsEntered() const { return bEntered; }
	bool IsGlobalZone() const;

	int32 GetBlendPriority() const { return (IsGlobalZone()) ? -1 : BlendPriority; }
	float GetBlendWeight() const;

	FColor GetPaintColor() const { return DebugColor; } // #92 : WorldMap 에서 사용하는 Color 값, 보통 Alpha 를 사용한다.

#if WITH_EDITOR
	virtual bool IsSelectable() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const UProperty* InProperty) const override;

	virtual FColor GetWireColor() const override;

	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; } // #92
#endif

public:
	UPROPERTY(VisibleAnywhere)
	FName MapZoneName;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4MapEnvironmentAsset> MapEnvironmentAsset; // #90, #92

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0", UIMax = "5"))
	int32 BlendPriority; // #92

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float BlendInTimeSec; // #92

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float BlendOutTimeSec; // #92

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FColor DebugColor; // #92
#endif

public:
	bool bEntered;
	bool bBlendStart;
	float BlendTimeLeft;

private:
	FT4OnPropertiesChanged OnPropertiesChangedDelegate; // #92
};