// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayHUD.h"

#include "Gameplay/T4GameplayInstance.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "Engine/Canvas.h"

#include "T4GameplayInternal.h"

/**
  * // #68
 */
FT4GameplayHUD::FT4GameplayHUD(ET4LayerType InLayerType)
	: LayerType(InLayerType)
{
}

FT4GameplayHUD::~FT4GameplayHUD()
{
}

bool FT4GameplayHUD::Initialize()
{
	check(LayerType < ET4LayerType::Max);
	return true;
}

void FT4GameplayHUD::Finalize()
{
}

void FT4GameplayHUD::Draw(
	FViewport* InViewport, 
	FCanvas* InCanvas, 
	FT4HUDDrawInfo& InOutDrawInfo
) // #68
{
	check(nullptr != InCanvas);

#if WITH_EDITOR
	float HeightOffset = 4.0f;

	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		// #68
		IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
		check(nullptr != GameWorld);
		if (nullptr != GameWorld->GetActionPlaybackPlayer())
		{
			IT4ActionPlaybackPlayer* ActionPlaybackPlayer = GameWorld->GetActionPlaybackPlayer();
			check(nullptr != ActionPlaybackPlayer);
			FString DrawString = FString::Printf(
				TEXT("* Action Playback [%s : %.1f / %.1f Sec] %s"),
				(ActionPlaybackPlayer->IsPaused()) ? TEXT("PAUSED") : TEXT("PLAY"),
				ActionPlaybackPlayer->GetPlayTimeSec(),
				ActionPlaybackPlayer->GetMaxPlayTimeSec(),
				ActionPlaybackPlayer->GetPlayFile()
			);
			DrawText(
				InViewport, 
				InCanvas, 
				DrawString, 
				(ActionPlaybackPlayer->IsPaused()) ? FLinearColor::Red : FLinearColor::Green,
				InOutDrawInfo
			);
		}
		if (nullptr != GameWorld->GetActionPlaybackRecorder())
		{
			IT4ActionPlaybackRecorder* ActionPlaybackRecorder = GameWorld->GetActionPlaybackRecorder();
			check(nullptr != ActionPlaybackRecorder);
			FString DrawString = FString::Printf(
				TEXT("* Action Playback [REC : %.1f Sec] %s"),
				ActionPlaybackRecorder->GetRecTimeSec(),
				ActionPlaybackRecorder->GetRecFile()
			);
			DrawText(InViewport, InCanvas, DrawString, FLinearColor::Red, InOutDrawInfo);
		}
	}
#endif
}

void FT4GameplayHUD::DrawText(
	FViewport* InViewport,
	FCanvas* InCanvas,
	const FString& InMessages,
	const FLinearColor& InColor,
	FT4HUDDrawInfo& InOutDrawInfo
)
{
#if WITH_EDITOR
	int32 XL;
	int32 YL;
	StringSize(GEngine->GetLargeFont(), XL, YL, *InMessages);
	const float DrawX = FMath::FloorToFloat(InViewport->GetSizeXY().X - XL - 15.0f);
	const float DrawY = InOutDrawInfo.NoticeLineOffset + YL;
	InCanvas->DrawShadowedString(DrawX, DrawY, *InMessages, GEngine->GetLargeFont(), InColor);
	InOutDrawInfo.NoticeLineOffset += YL + 2.0f;
#endif
}

IT4PlayerController* FT4GameplayHUD::GetPlayerController() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetPlayerController();
}

IT4GameFrame* FT4GameplayHUD::GetGameFrame() const
{
	check(ET4LayerType::Max > LayerType);
	return T4FrameGet(LayerType);
}

FT4GameplayInstance* FT4GameplayHUD::GetGameplayInstance() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	return GameplayInstance;
}