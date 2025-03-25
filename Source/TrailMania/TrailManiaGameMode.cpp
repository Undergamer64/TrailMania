// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrailManiaGameMode.h"
#include "TrailManiaPlayerController.h"

ATrailManiaGameMode::ATrailManiaGameMode()
{
	PlayerControllerClass = ATrailManiaPlayerController::StaticClass();
}
