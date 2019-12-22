// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"
#include "T4ActionPlaybackBase.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4ActionPlaybackRecorder : public FT4ActionPlaybackBase, public IT4ActionPlaybackRecorder
{
public:
	explicit FT4ActionPlaybackRecorder(ET4LayerType InLayerType);
	virtual ~FT4ActionPlaybackRecorder();

	// IT4ActionPlaybackRecorder
	bool IsRecording() const override { return bRecording; }

	const TCHAR* GetRecFile() const override { return *RecFullPath; }

	float GetRecTimeSec() const override { return RecTimeSec; }

	bool RecWorldAction(
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InActionParam
	) override;
	bool RecObjectAction(
		const FT4ObjectID& InObjectID,
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InActionParam
	) override;

public:
	bool DoRec(const FSoftObjectPath& InRecPath);
	bool DoRec(const FString& InRecAssetName, const FString& InFolderName);

	void ProcessRecording(float InDeltaTime);

protected:
	void Reset() override;
	void Stop() override;

private:
	bool Rec(const FString& InAssetName, const FString& InPackagePath);

private:
	bool bRecording;
	float RecTimeSec;

	FString RecAssetName;
	FString RecPackagePath;

	FString RecFullPath;
};

#endif