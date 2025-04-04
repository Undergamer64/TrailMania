// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrailManiaPawn.h"
#include "TrailManiaWheelFront.h"
#include "TrailManiaWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TrailManiaPlayerController.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ATrailManiaPawn::ATrailManiaPawn()
{
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(GetMesh());
	Arrow->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());;
}

void ATrailManiaPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ATrailManiaPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ATrailManiaPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ATrailManiaPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ATrailManiaPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATrailManiaPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATrailManiaPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::ResetVehicle);
		
		EnhancedInputComponent->BindAction(FullResetVehicleAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::FullResetVehicle);
		
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Triggered, this, &ATrailManiaPawn::Quit);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATrailManiaPawn::Quit()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void ATrailManiaPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	if (bIsRacing)
	{
		CurrentTimer += Delta;
	}

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 0.5f);

	BackSpringArm->SetRelativeRotation(FRotator(-10.0f, CameraYaw, 0.0f));

	CheckNewGravity();
	
	if (Arrow != nullptr)
	{
		Arrow->SetWorldRotation(gravity.Rotation());
	}
	
	if (bIsCentralGravity)
	{
		GetMesh()->SetPhysicsLinearVelocity((CentralGravity - GetActorLocation()).GetSafeNormal() * 4 * (GravityScale * 981) * Delta, true);
	}
	else
	{
		GetMesh()->SetPhysicsLinearVelocity(gravity * (GravityScale * 981) * Delta, true);
	}
}

void ATrailManiaPawn::CheckNewGravity()
{
	FHitResult HitResult;
	FVector Start = GetActorLocation() + GetActorUpVector() * 5;
	FVector End = Start + -GetActorUpVector() * 200;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor == nullptr)
		{
			return;
		}
		
		if (HitActor->ActorHasTag("Gravity"))
		{
			bIsCentralGravity = false;
			gravity = -HitResult.ImpactNormal;
		}
		else if (HitActor->ActorHasTag("Gravity2"))
		{
			bIsCentralGravity = true;
			CentralGravity = HitActor->GetActorLocation();
		}
	}
}


void ATrailManiaPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);

	bIsRacing = true;
}

void ATrailManiaPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);

	bIsRacing = true;
}

void ATrailManiaPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);

	bIsRacing = true;
}

void ATrailManiaPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);

	bIsRacing = true;
}

void ATrailManiaPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);

	bIsRacing = true;
}

void ATrailManiaPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);

	bIsRacing = true;
}

void ATrailManiaPawn::StopHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);

	bIsRacing = true;
}

void ATrailManiaPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ATrailManiaPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ATrailManiaPawn::ResetVehicle()
{
	if (CurrentCheckpoint != nullptr)
	{
		CurrentCheckpoint->RespawnPlayer(this);
	}
	else
	{
		FullResetVehicle();
	}
	
	BackSpringArm->SetRelativeRotation(FRotator(-10.0f, BackSpringArm->GetRelativeRotation().Yaw, 0.0f));
	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ATrailManiaPawn::FullResetVehicle()
{
	// reset the vehicle to the start
	SetActorTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	gravity = FVector::DownVector;
	CentralGravity = FVector::Zero();
	bIsCentralGravity = false;
	bIsRacing = false;
	CurrentTimer = 0.0f;

	for (ACheckpoint* Checkpoint : CheckpointsPassed)
	{
		if (Checkpoint == nullptr)
		{
			continue;
		}
		Checkpoint->ResetCheckpoint();
	}
	CheckpointsPassed.Empty();
	CurrentCheckpoint = nullptr;
}

void ATrailManiaPawn::SetCheckpoint(ACheckpoint* Checkpoint)
{
	CurrentCheckpoint = Checkpoint;
	CheckpointsPassed.Add(Checkpoint);
}

void ATrailManiaPawn::FinishRace()
{
	if (BestTime == NULL || CurrentTimer < BestTime)
	{
		if (Controller != nullptr && Controller->IsA(ATrailManiaPlayerController::StaticClass()))
		{
			ATrailManiaPlayerController* TrailManiaPlayerController = Cast<ATrailManiaPlayerController>(Controller);
			BestTime = CurrentTimer;
			TrailManiaPlayerController->SetNewRaceTime(BestTime);
		}
	}
	
	FullResetVehicle();
}


#undef LOCTEXT_NAMESPACE