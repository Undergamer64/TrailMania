// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrailManiaWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UTrailManiaWheelRear::UTrailManiaWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}