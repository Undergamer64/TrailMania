// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Checkpoint.generated.h"

UCLASS()
class TRAILMANIA_API ACheckpoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACheckpoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void RespawnPlayer(AActor* Actor);
	
	TObjectPtr<UBoxComponent> BoxComponent;

	FTransform RespawnTransform;
	FVector RespawnVelocity;
	FVector RespawnGravity;
};
