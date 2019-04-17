// Fill out your copyright notice in the Description page of Project Settings.

#include "MLController.h"

// Sets default values
AMLController::AMLController()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// Create a dummy root component we can attach things to.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	AttachedCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("AttachedCameraComponent"));
	// Create a camera and a visible object
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	// Attach our camera and visible object to our root component. Offset and rotate the camera.
	StaticMesh->SetupAttachment(RootComponent);
	AttachedCamera->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMLController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMLController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector location = GetActorLocation();
	if (bFlying)
		location.Z += FlightSpeed * DeltaTime;
	location += CurrentVelocity * DeltaTime * MovementSpeed;
	SetActorLocation(location);
}

// Called to bind functionality to input
void AMLController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Respond every frame to the values of our two movement axes, "MoveX" and "MoveY".
	PlayerInputComponent->BindAxis("MoveX", this, &AMLController::MoveXAxis);
	PlayerInputComponent->BindAxis("MoveY", this, &AMLController::MoveYAxis);
}

#if WITH_EDITOR
void AMLController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UpdateEditorProperties();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AMLController::MoveXAxis(float AxisValue)
{
	// Move at 100 units per second forward or backward
	CurrentVelocity.X = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
}

void AMLController::MoveYAxis(float AxisValue)
{
	// Move at 100 units per second right or left
	CurrentVelocity.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
}

void AMLController::UpdateEditorProperties()
{
	AttachedCamera->SetRelativeLocation(CameraRelativeLocation);
	AttachedCamera->SetRelativeRotation(CameraRelativeRotation);
}

