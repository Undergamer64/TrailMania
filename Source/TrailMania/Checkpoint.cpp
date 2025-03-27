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
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlapBegin);
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
	UE_LOG(LogTemplateVehicle, Error, TEXT("Checkpoint !"));
	if (OtherActor->IsA(ATrailManiaPawn::StaticClass()))
	{
		ATrailManiaPawn* Pawn = Cast<ATrailManiaPawn>(OtherActor);
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
