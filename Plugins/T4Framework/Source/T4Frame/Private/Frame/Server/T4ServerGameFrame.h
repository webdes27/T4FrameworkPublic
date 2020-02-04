// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "Frame/T4GameFrame.h"
#endif

/**
  *
  */
#if (WITH_EDITOR || WITH_SERVER_CODE)

class AAIController;
class FT4ServerGameFrame : public FT4GameFrame
{
public:
	explicit FT4ServerGameFrame();
	~FT4ServerGameFrame();

	// IT4Frame
	virtual ET4FrameType GetType() const override { return ET4FrameType::Frame_Server; }

	uint32 GenerateNetIDForServer() override { return ++UniqueNetIDIncr; }
	FT4ObjectID GenerateObjectIDForServer() override;

	bool RegisterGameAIController(const FT4NetID& InUniqueID, IT4GameAIController* InAIController) override; // #31
	void UnregisterGameAIController(const FT4NetID& InUniqueID) override; // #31

	IT4GameAIController* FindGameAIController(const FT4NetID& InUniqueID) const override; // #31

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;

	virtual void ResetPre() override;
	virtual void ResetPost() override;

	virtual void StartPlay() override;

	virtual void ProcessPre(float InDeltaTime) override;
	virtual void ProcessPost(float InDeltaTime) override;

private:
	uint32 UniqueNetIDIncr; // #15
	FT4ObjectID UniqueObjectIDIncr;

	TMap<FT4NetID, IT4GameAIController*> GameAIControllerRefs; // #15
};

#endif