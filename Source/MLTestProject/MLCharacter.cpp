// Fill out your copyright notice in the Description page of Project Settings.

#include "MLCharacter.h"
#include <Components/InputComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Components/CapsuleComponent.h>
#include <Runtime/Engine/Classes/GameFramework/SpringArmComponent.h>
#include <Runtime/Engine/Classes/Camera/CameraComponent.h>
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/Engine/Classes/GameFramework/Controller.h>
#include <Runtime/Engine/Classes/Engine/StaticMesh.h>
#include <Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>
#include <Runtime/Engine/Public/LevelUtils.h>
// Sets default values
AMLCharacter::AMLCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ray_distance = 100.0f;
	ray_width = 10.0f;

	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	static_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesoh"));
	attached_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	camera_spring_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	
	static_mesh->SetupAttachment(RootComponent);
	camera_spring_arm->SetupAttachment(RootComponent);
	attached_camera->SetupAttachment(camera_spring_arm, USpringArmComponent::SocketName);

	camera_spring_arm->TargetArmLength = 400.f;
	camera_spring_arm->bEnableCameraLag = true;
	camera_spring_arm->CameraLagSpeed = 3.0f;
	static_mesh->SetMobility(EComponentMobility::Movable);
	static_mesh->SetVisibility(true);

	//UpdateEditorProperties();
}

void EditorInit()
{

}

#if WITH_EDITOR
void AMLCharacter::PostEditMove(bool bFinished)
{
}


void AMLCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//UpdateEditorProperties();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// Called when the game starts or when spawned
void AMLCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("We are using MLCharacter."));
	}
	UpdateEditorProperties();
}

// Called every frame
void AMLCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CastRay();

	if (!CameraInput.IsZero())
	{
		FRotator NewRotation =camera_spring_arm->GetTargetRotation();
		NewRotation.Yaw += CameraInput.X;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -88.0f, 30.0f);
		camera_spring_arm->SetWorldRotation(NewRotation);
	}

	if (!MovementInput.IsZero())
	{
		//Scale our movement input axis values by 100 units per second
		MovementInput = MovementInput.GetSafeNormal() * 400.0f;
		FVector NewLocation = GetActorLocation();
		NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
		NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
		SetActorLocation(NewLocation);
	}
}

// Called to bind functionality to input
void AMLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up "movement" bindings.
	PlayerInputComponent->BindAxis("MoveForward", this, &AMLCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMLCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MouseX", this, &AMLCharacter::YawCamera);
	PlayerInputComponent->BindAxis("MouseY", this, &AMLCharacter::PitchCamera);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMLCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMLCharacter::StopJump);
	PlayerInputComponent->BindAxis("MouseWheel", this, &AMLCharacter::CameraZoom);
}

void AMLCharacter::MoveForward(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
void AMLCharacter::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMLCharacter::StartJump()
{
	bPressedJump = true;
}

void AMLCharacter::StopJump()
{
	bPressedJump = false;
}

void  AMLCharacter::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogTemp, Warning, TEXT("PostLoad"));
}

void AMLCharacter::PostActorCreated()
{
	Super::PostActorCreated();
	UE_LOG(LogTemp, Warning, TEXT("PostActorCreated"));
	UpdateEditorProperties();
}

void AMLCharacter::UpdateComponentLocations()
{
	UE_LOG(LogTemp, Warning, TEXT("UPP"));

	FVector root_world_location = RootComponent->GetComponentLocation();
	SetActorLocation(root_world_location);
	root_world_location = RootComponent->GetComponentLocation();
	GetCapsuleComponent()->SetWorldLocation(root_world_location);
	if (static_mesh) {
		static_mesh->SetWorldLocation(root_world_location);
	}
	if (camera_spring_arm)	{
		camera_spring_arm->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator(-60.0f, 0.f, 0.0f));
		camera_spring_arm->SSetWorldLocation(root_world_location);
	}
	if (attached_camera) {
		FVector socket_location;
		FRotator socket_rotation;
		camera_spring_arm->GetSocketWorldLocationAndRotation(USpringArmComponent::SocketName, socket_location, socket_rotation);
		attached_camera->SetWorldLocationAndRotation(socket_location, socket_rotation);
	}
}

void AMLCharacter::HandleAttachments()
{
	if (attached_camera) {
		/*FDetachmentTransformRules detachmenRule(EDetachmentRule::KeepRelative, false);
		attached_camera->DetachFromComponent(detachmenRule);
		FVector socket_location;
		FRotator socket_rotation;
		camera_spring_arm->GetSocketWorldLocationAndRotation(USpringArmComponent::SocketName, socket_location, socket_rotation);
		attached_camera->SetWorldLocationAndRotation(socket_location, socket_rotation);
		attached_camera->SetupAttachment(camera_spring_arm, USpringArmComponent::SocketName);
		UE_LOG(LogTemp, Warning, TEXT("Attached Camera"));*/
	}
}

void AMLCharacter::UpdateEditorProperties()
{
	UpdateComponentLocations();
	HandleAttachments();
}

void AMLCharacter::CastRay()
{
	FHitResult hit_result;
	FVector ray_start = GetActorLocation();
	FVector forward_vec = GetActorForwardVector();
	//TODO_OGUZ: FIND A BETTER WAY OF AVOIDING SELF HIT
	ray_start += forward_vec * 100.0f;
	FVector ray_end = (forward_vec * ray_distance) + ray_start;

	FColor debug_line_color = FColor::Red;
	if (GetWorld()->LineTraceSingleByChannel(hit_result, ray_start, ray_end, ECC_WorldStatic))
	{
		debug_line_color = FColor::Green;
	}

	DrawDebugLine(GetWorld(), ray_start, ray_end, debug_line_color, false, -1.0f, '\000', ray_width);
}

//Input functions
void AMLCharacter::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}
void AMLCharacter::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void AMLCharacter::CameraZoom(float AxisValue)
{
	if (AxisValue != 0)
	{
		//attached_camera->FieldOfView += ::Lerp<float>(90.0f, 60.0f, ZoomFactor * AxisValue);
		camera_spring_arm->TargetArmLength += ZoomFactor * AxisValue;
	}
}
