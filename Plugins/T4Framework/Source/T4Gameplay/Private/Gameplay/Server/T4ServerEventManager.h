// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/GameTable/T4GameTableDataTypes.h" // #48

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
#if (WITH_EDITOR || WITH_SERVER_CODE)

struct FEffectDamageData // #63 : temp
{
	FName EventName;
	FT4ObjectID TargetObjectID;
	FVector TargetLocation;
	FT4ObjectID AttackerObjectID;
	FT4GameEffectDataID EffectDataID;
};

struct FEffectDamageInfo // #63 : temp
{
	float Time;
	FEffectDamageData Data;
};

class IT4GameObject;
class IT4ObjectController;
class IT4EditorGameplayHandler;
class FT4ServerEventManager
{
public:
	explicit FT4ServerEventManager();
	~FT4ServerEventManager();

	bool Initialize(ET4LayerType InLayerType);
	void Finalize();

	void Process(float InDeltaTime);

	void AddEffectDamage(
		float InProcessTimeSec, 
		const FEffectDamageData& InEffectData
	); // #63 : temp

private:
	void ProcessEffectDamage(const FEffectDamageData& InEffectData); // #50, #63

	bool GetTargetObjects(
		const FVector& InCenterLocation,
		const float InAreaRange,
		TArray<IT4GameObject*>& OutTargetObjects
	);

	IT4ObjectController* GetObjectController(const FT4ObjectID& InObjectID); // #63
	IT4EditorGameplayHandler* GetEditorGameplayCustomHandler() const; // #60, #63

private:
	ET4LayerType LayerType;

	TArray<FEffectDamageInfo> EffectDamages;
};

#endif