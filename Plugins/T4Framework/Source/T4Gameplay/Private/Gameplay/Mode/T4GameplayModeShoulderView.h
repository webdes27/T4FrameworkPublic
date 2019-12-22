// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayModeBase.h"

/**
  * #40
 */
class IT4PlayerController;
class IT4PacketHandlerCS;
class FT4GameplayModeShoulderView : public FT4GameplayModeBase
{
public:
	explicit FT4GameplayModeShoulderView(ET4LayerType InLayerType);
	~FT4GameplayModeShoulderView();

	ET4GameplayGameModeType GetModeType() const override { return ET4GameplayGameModeType::ShoulderView; }

protected:
	void Enter() override;
	void Leave() override;

	void Process(float InDeltaTime) override;

private:
	void ProcessMovement(float InDeltaTime);
	void ProcessLockOn(float InDeltaTime);

private:
	bool bLockOnStart; // #40
	float LockOnClearTimeLeft; // #40
};
