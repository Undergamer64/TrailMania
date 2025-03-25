// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrailManiaWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UTrailManiaWheelFront::UTrailManiaWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}