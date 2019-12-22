// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayModeBase.h"

/**
  * #40
 */

class FT4GameplayModeTPS : public FT4GameplayModeBase
{
public:
	explicit FT4GameplayModeTPS(ET4LayerType InLayerType);
	~FT4GameplayModeTPS();

	ET4GameplayGameModeType GetModeType() const override { return ET4GameplayGameModeType::TPS; }

	bool DoLockOnPressed() override;
	bool DoLockOnReleased() override;

protected:
	void Enter() override;
	void Leave() override;

	void Process(float InDeltaTime) override;

private:
	void ProcessMovement(float InDeltaTime);
	void ProcessTurn(float InDeltaTime);
};
