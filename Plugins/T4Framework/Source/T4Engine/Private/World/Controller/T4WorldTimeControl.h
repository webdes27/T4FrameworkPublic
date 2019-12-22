// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #93
 */
class FT4WorldController;
class FT4WorldTimeControl
{
public:
	explicit FT4WorldTimeControl(FT4WorldController* InWorldController);
	virtual ~FT4WorldTimeControl();

	void Initialize(float InStartHour);
	void Reset(float InStartHour);

	void Process(float InDeltaTime);

	float GetRatio() const { return GameTimeRatio; } // 0.0f ~ 1.0f
	float GetRatioForNameRange() const { return GameTimeRatioForNameRange; } // 0.0f ~ 1.0f for GetGameTimeTagName
	
	FName GetTimeTagName() const;
	FName GetPrevTimeTagName() const;
	FName GetNextTimeTagName() const;

	void SetPause(bool bInPause) { bPaused = bInPause; }
	bool IsPaused() const { return bPaused; } // #94

	void SetTimeHour(float InHour);
	float GetTimeHour() const;

	void SetTimeScale(float InScale) { GameTimeScale = InScale; }
	float GetTimeScale() const { return GameTimeScale; }

	FString GetDisplayString();

private:
	void AddConstantGameTimeData(const FName InStartTimeName, const FName InEndTimeName); // #93

private:
	FT4WorldController* WorldControllerRef;

	struct FT4GameTimeData // #93
	{
		FName TimeTagName;
		FVector2D TimeHourDisplayRange; // hour
		FVector2D TimeHourRange; // hour
		TRange<float> TimeSecRange; // sec

		float TimeLocalAddSec; // For Local
		float TimeLocalMaxSec; // For Local
	};

	TArray<FT4GameTimeData> ConstantGameTimeTables;
	int32 ConstantGameTimeHoursPerDay; // #93 : UT4EngineSettings::WorldimeZoneHours
	float ConstantGameTimeClockScale; // #93 : 24 / UT4EngineSettings::WorldimeZoneHours
	float ConstantGameTimeSecPerHour; // #93 : SystemTimeSecPerHour * ConstantGameTimeClockScale
	float ConstantGameTimeMaxSec; // #93 : UT4EngineSettings::WorldimeZoneHours * SystemTimeSecPerHour

	const FT4GameTimeData* GameTimeDataRef;
	float GameTimeSec;
	float GameTimeRatio;
	float GameTimeRatioForNameRange;
	float GameTimeScale;

	bool bPaused;
};
