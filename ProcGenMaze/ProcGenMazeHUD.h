// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ProcGenMazeHUD.generated.h"

UCLASS()
class AProcGenMazeHUD : public AHUD
{
	GENERATED_BODY()

public:
	AProcGenMazeHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

