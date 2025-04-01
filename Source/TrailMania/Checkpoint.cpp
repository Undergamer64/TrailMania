// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint.h"

#include "TrailManiaPawn.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	BoxComponent->SetRelativeScale3D(FVector(0.1, 2, 2));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlapBegin);

	ResetCheckpoint();
}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsEnd)
	{
		SetEnd(OtherActor);
	}
	else
	{
		SetCheckpoint(OtherActor);
	}
}

void ACheckpoint::SetEnd(AActor* OtherActor)
{
	UE_LOG(LogTemplateVehicle, Error, TEXT("End !"));
	if (OtherActor->IsA(ATrailManiaPawn::StaticClass()))
	{
		ATrailManiaPawn* Pawn = Cast<ATrailManiaPawn>(OtherActor);
		Pawn->FinishRace();
	}
}

void ACheckpoint::SetCheckpoint(AActor* OtherActor)
{
	UE_LOG(LogTemplateVehicle, Error, TEXT("Checkpoint !"));
	if (OtherActor->IsA(ATrailManiaPawn::StaticClass()))
	{
		ATrailManiaPawn* Pawn = Cast<ATrailManiaPawn>(OtherActor);
		if (Pawn == nullptr) return;
		Pawn->SetCheckpoint(this);

		RespawnTransform = Pawn->GetTransform();
		RespawnVelocity = Pawn->GetMesh()->GetPhysicsLinearVelocity();
		RespawnGravity = Pawn->gravity;

		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ACheckpoint::RespawnPlayer(AActor* Actor)
{
	if (Actor->IsA(ATrailManiaPawn::StaticClass()))
	{
		ATrailManiaPawn* Pawn = Cast<ATrailManiaPawn>(Actor);
		Pawn->gravity = RespawnGravity;
		Pawn->SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		Pawn->GetMesh()->SetPhysicsLinearVelocity(RespawnVelocity);
		Pawn->GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ACheckpoint::ResetCheckpoint()
{
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	RespawnGravity = FVector::DownVector;
	RespawnTransform = FTransform::Identity;
	RespawnVelocity = FVector::ZeroVector;
}
