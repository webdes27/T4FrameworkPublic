// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldTimeControl.h"
#include "T4WorldController.h"

#include "Public/T4EngineSettings.h" // #93
#include "Public/T4EngineConstants.h" // #93
#include "Public/T4EngineDefinitions.h" // #93
#include "Public/T4EngineUtility.h" // #93
#include "Public/T4Engine.h" // #93

#include "T4EngineInternal.h"

static const float SystemTimeSecPerHour = 60.0f * 60.0f;
static const int32 SystemNumberOfHoursPerDay = 24;

/**
  * #93
 */
FT4WorldTimeControl::FT4WorldTimeControl(FT4WorldController* InWorldController)
	: WorldControllerRef(InWorldController)
	, ConstantGameTimeHoursPerDay(SystemNumberOfHoursPerDay) // #93 : UT4EngineSettings::WorldimeZoneHours
	, ConstantGameTimeClockScale(1.0f) // #93 : 24 / UT4EngineSettings::WorldimeZoneHours
	, ConstantGameTimeSecPerHour(SystemTimeSecPerHour) // #93 : SystemTimeSecPerHour * ConstantGameTimeClockScale
	, ConstantGameTimeMaxSec(ConstantGameTimeHoursPerDay * ConstantGameTimeSecPerHour) // #93 : UT4EngineSettings::WorldimeZoneHours * SystemTimeSecPerHour
	, GameTimeDataRef(nullptr) // #93
	, GameTimeSec(0.0f) // #93
	, GameTimeRatio(0.0f) // #93
	, GameTimeRatioForNameRange(0.0f) // #93
	, GameTimeScale(1.0f) // #93
	, bPaused(false)
{
}

FT4WorldTimeControl::~FT4WorldTimeControl()
{
}

void FT4WorldTimeControl::Initialize(float InStartHour)
{
	UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
	check(nullptr != EngineSettings);
	ConstantGameTimeHoursPerDay = EngineSettings->GameTimeHoursPerDay;
	check(0 < ConstantGameTimeHoursPerDay);
	ConstantGameTimeClockScale = 1.0f;
	if (SystemNumberOfHoursPerDay != ConstantGameTimeHoursPerDay)
	{
		ConstantGameTimeClockScale = static_cast<float>(SystemNumberOfHoursPerDay / ConstantGameTimeHoursPerDay);
	}
	ConstantGameTimeSecPerHour = SystemTimeSecPerHour * ConstantGameTimeClockScale;
	ConstantGameTimeMaxSec = ConstantGameTimeHoursPerDay * ConstantGameTimeSecPerHour;
	{
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfDay, T4WorldEivronmentTimeTagNameOfAtNoon);
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfAtNoon, T4WorldEivronmentTimeTagNameOfSunset); // #97 : 정오, 한밤 추가!
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfSunset, T4WorldEivronmentTimeTagNameOfNight);
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfNight, T4WorldEivronmentTimeTagNameOfMidnight); // #97 : 정오, 한밤 추가!
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfMidnight, T4WorldEivronmentTimeTagNameOfSunrise);
		AddConstantGameTimeData(T4WorldEivronmentTimeTagNameOfSunrise, T4WorldEivronmentTimeTagNameOfDay);
	}
	GameTimeSec = InStartHour * ConstantGameTimeSecPerHour;
	Process(0.0f);
}

void FT4WorldTimeControl::Reset(float InStartHour)
{
	GameTimeSec = InStartHour * ConstantGameTimeSecPerHour;
	GameTimeDataRef = nullptr;
	GameTimeRatio = 0.0f; // #93
	GameTimeRatioForNameRange = 0.0f; // #93
	GameTimeScale = 1.0f; // #93
}

void FT4WorldTimeControl::Process(float InDeltaTime) // #93
{
	if (bPaused) // #93
	{
		return; // 툴 또는 게임 환경에서 시간 업데이트를 멈출 경우!
	}
	GameTimeSec += (InDeltaTime * ConstantGameTimeClockScale) * GameTimeScale;
	if (GameTimeSec >= ConstantGameTimeMaxSec)
	{
		GameTimeSec = GameTimeSec - ConstantGameTimeMaxSec;
	}
	GameTimeRatio = FMath::Clamp(GameTimeSec / ConstantGameTimeMaxSec, 0.0f, 1.0f);
	for (const FT4GameTimeData& TimeData : ConstantGameTimeTables)
	{
		if (TimeData.TimeSecRange.Contains(GameTimeSec))
		{
			if (nullptr == GameTimeDataRef ||
			    (nullptr != GameTimeDataRef && GameTimeDataRef->TimeTagName != TimeData.TimeTagName))
			{
				check(nullptr != WorldControllerRef);
				FT4EngineDelegates::OnGameWorldTimeTransition.Broadcast(
					WorldControllerRef->GetGameWorld(), 
					TimeData.TimeTagName
				);
			}
			float GameLocalTimeSec = GameTimeSec - TimeData.TimeSecRange.GetLowerBoundValue();
			GameTimeRatioForNameRange = FMath::Clamp(
				(TimeData.TimeLocalAddSec + GameLocalTimeSec) / TimeData.TimeLocalMaxSec,
				0.0f, 
				1.0f
			);
			GameTimeDataRef = &TimeData;
			break;
		}
	}
}

FName FT4WorldTimeControl::GetTimeTagName() const
{
	return (nullptr != GameTimeDataRef) ? GameTimeDataRef->TimeTagName : NAME_None;
}

FName FT4WorldTimeControl::GetPrevTimeTagName() const
{
	FName CurrentTimeTagName = GetTimeTagName();
	return T4EngineUtility::GetPrevTimeTagName(CurrentTimeTagName);
}

FName FT4WorldTimeControl::GetNextTimeTagName() const
{
	FName CurrentTimeTagName = GetTimeTagName();
	return T4EngineUtility::GetNextTimeTagName(CurrentTimeTagName);
}

void FT4WorldTimeControl::SetTimeHour(float InHour)
{
	GameTimeSec = (InHour * ConstantGameTimeClockScale) * ConstantGameTimeSecPerHour;
	Process(0.0f);
}

float FT4WorldTimeControl::GetTimeHour() const
{
	return GameTimeSec / ConstantGameTimeSecPerHour;
}

void FT4WorldTimeControl::AddConstantGameTimeData(
	const FName InStartTimeName,
	const FName InEndTimeName
) // #93
{
	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	const FT4ConstantDataRow& StartTimeDataRow = EngineConstants->GetConstantData(
		ET4EngineConstantType::TimeTag,
		InStartTimeName
	);
	const FT4ConstantDataRow& EndTimeDataRow = EngineConstants->GetConstantData(
		ET4EngineConstantType::TimeTag,
		InEndTimeName
	);

	float StartHour = StartTimeDataRow.FloatValue;
	float EndHour = EndTimeDataRow.FloatValue;
	float LocalMaxHour = (EndHour - StartHour) * ConstantGameTimeClockScale;
	float LocalAddHour = 0.0f;
	if (StartHour > EndHour)
	{
		// WARN : Night 와 같이 Start=20 ~ End=7 와 같을 경우 계산 편의를 위해 
		//        20 ~ 24시, 0 ~ 7시 기준으로 끊어 두개의 TimeData 로 등록해준다.
		FT4GameTimeData& NewGameTimeData = ConstantGameTimeTables.AddDefaulted_GetRef();
		NewGameTimeData.TimeTagName = StartTimeDataRow.Name;
		NewGameTimeData.TimeHourDisplayRange = FVector2D(
			StartTimeDataRow.FloatValue,
			EndTimeDataRow.FloatValue
		);
		NewGameTimeData.TimeHourRange = FVector2D(
			StartHour * ConstantGameTimeClockScale,
			ConstantGameTimeHoursPerDay
		);
		NewGameTimeData.TimeSecRange = TRange<float>(
			NewGameTimeData.TimeHourRange.X * ConstantGameTimeSecPerHour,
			NewGameTimeData.TimeHourRange.Y * ConstantGameTimeSecPerHour
		);

		// WARN : LocalTimeRatio 계산시 0 ~ 7 TimeData 에는 20 ~ 24 까지의 시간을 더하기 위해 미리 계산해둔다.
		LocalAddHour = ConstantGameTimeHoursPerDay - (StartHour * ConstantGameTimeClockScale);
		
		// WARN : Night 류의 LocalTimeRatio 계산시에는 20 ~ 7 과 관계없이 전체 시간 (11시간) 으로 LocalRatio 를 계산한다.
		LocalMaxHour = LocalAddHour + (EndTimeDataRow.FloatValue * ConstantGameTimeClockScale);

		NewGameTimeData.TimeLocalMaxSec = LocalMaxHour * ConstantGameTimeSecPerHour;
		NewGameTimeData.TimeLocalAddSec = 0.0f;
		StartHour = 0.0f;
	}
	FT4GameTimeData& NewGameTimeData = ConstantGameTimeTables.AddDefaulted_GetRef();
	NewGameTimeData.TimeTagName = StartTimeDataRow.Name;
	NewGameTimeData.TimeHourDisplayRange = FVector2D(
		StartTimeDataRow.FloatValue,
		EndTimeDataRow.FloatValue
	);
	NewGameTimeData.TimeHourRange = FVector2D(
		StartHour * ConstantGameTimeClockScale,
		EndHour * ConstantGameTimeClockScale
	);
	NewGameTimeData.TimeSecRange = TRange<float>(
		NewGameTimeData.TimeHourRange.X * ConstantGameTimeSecPerHour,
		NewGameTimeData.TimeHourRange.Y * ConstantGameTimeSecPerHour
	);
	NewGameTimeData.TimeLocalMaxSec = LocalMaxHour * ConstantGameTimeSecPerHour;
	NewGameTimeData.TimeLocalAddSec = LocalAddHour * ConstantGameTimeSecPerHour;
}

FString FT4WorldTimeControl::GetDisplayString()
{
	float CurrentHour = GameTimeSec / ConstantGameTimeSecPerHour;
	return FString::Printf(
		TEXT("TOD : %s => %s (%.0f ~ %.0f) '%.2f% / %.0f%' Hours (x%.1f) %s"),
		*(GetTimeTagName().ToString()),
		*(GetNextTimeTagName().ToString()),
		(nullptr != GameTimeDataRef) ? GameTimeDataRef->TimeHourDisplayRange.X : 0.0f,
		(nullptr != GameTimeDataRef) ? GameTimeDataRef->TimeHourDisplayRange.Y : 0.0f,
		CurrentHour,
		(nullptr != GameTimeDataRef) ? GameTimeDataRef->TimeHourRange.Y : 24.0f,
		GameTimeScale,
		(bPaused) ? TEXT("/ Paused") : TEXT("")
	);
}
