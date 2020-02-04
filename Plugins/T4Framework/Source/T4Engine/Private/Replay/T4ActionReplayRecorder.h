// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"
#include "T4ActionReplayBase.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4ActionReplayRecorder : public FT4ActionReplayBase, public IT4ActionReplayRecorder
{
public:
	explicit FT4ActionReplayRecorder(ET4LayerType InLayerType);
	virtual ~FT4ActionReplayRecorder();

	// IT4ActionReplayRecorder
	bool IsRecording() const override { return bRecording; }

	const TCHAR* GetRecFile() const override { return *RecFullPath; }

	float GetRecTimeSec() const override { return RecTimeSec; }

	bool RecWorldAction(
		const FT4ActionCommand* InAction,
		const FT4ActionParameters* InActionParam
	) override;
	bool RecObjectAction(
		const FT4ObjectID& InObjectID,
		const FT4ActionCommand* InAction,
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