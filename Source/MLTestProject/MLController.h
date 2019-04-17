// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include <Camera/CameraComponent.h>
#include <DrawDebugHelpers.h>
#include "MLController.generated.h"

UCLASS()
class MLTESTPROJECT_API AMLController : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMLController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void MoveXAxis(float AxisValue);
	void MoveYAxis(float AxisValue);
	void UpdateEditorProperties();

	//Input variables
	UCameraComponent* AttachedCamera;
	FVector CurrentVelocity;
	bool bFlying;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float FlightSpeed = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float MovementSpeed = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
		FVector CameraRelativeLocation;

	UPROPERTY(EditAnywhere, Category = "Camera")
		FRotator CameraRelativeRotation;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMesh;
};
