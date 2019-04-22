// Fill out your copyright notice in the Description page of Project Settings.

#include "MLCharacter.h"
#include "SensorComponent.h"
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
#include <Runtime/Engine/Public/EngineUtils.h >

// Sets default values
AMLCharacter::AMLCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance
    // if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesoh"));
    attached_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
    camera_spring_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    FrontLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("FrontSensorComponent"));
    FrontRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("BehindSensorComponent"));
    BackLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("LeftSensorComponent"));
    BackRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("RightSensorComponent"));

    // Network
    network = MLGenome(8, 2);
    fitness = 0;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Meshes/CarMesh"));
    if (MeshAsset.Succeeded())
    {
        UStaticMesh* Asset = MeshAsset.Object;
        StaticMesh->SetStaticMesh(Asset);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    }
    FrontRightSensorSocket = FName("FrontRightSensor");
    FrontLeftSensorSocket = FName("FrontLeftSensor");
    BackLeftSensorSocket = FName("BackLeftSensor");
    BackRightSensorSocket = FName("BackRightSensor");

    camera_spring_arm->SetupAttachment(RootComponent);
    attached_camera->SetupAttachment(camera_spring_arm, USpringArmComponent::SocketName);

    FrontLeftSensor->SetupAttachment(StaticMesh, FrontRightSensorSocket);
    FrontRightSensor->SetupAttachment(StaticMesh, FrontLeftSensorSocket);
    BackRightSensor->SetupAttachment(StaticMesh, BackLeftSensorSocket);
    BackLeftSensor->SetupAttachment(StaticMesh, BackRightSensorSocket);

    FrontRightSensor->SetRayCastDirections(true, false, true);
    FrontLeftSensor->SetRayCastDirections(true, false, false, true);
    BackLeftSensor->SetRayCastDirections(false, true, false, true);
    BackRightSensor->SetRayCastDirections(false, true, true);

    camera_spring_arm->TargetArmLength = 400.f;
    camera_spring_arm->bEnableCameraLag = true;
    camera_spring_arm->CameraLagSpeed = 3.0f;
    StaticMesh->SetMobility(EComponentMobility::Movable);
    StaticMesh->SetVisibility(true);
    StaticMesh->SetRelativeRotationExact(FRotator(0.0f, 90.0f, 0.0f));
    UpdateEditorProperties();
    Tags.Add(FName("MLTrigger"));
    RootComponent->SetMobility(EComponentMobility::Movable);
    StaticMesh->SetGenerateOverlapEvents(false);
    StaticMesh->SetCollisionObjectType(ECC_WorldDynamic);
    StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    StaticMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldDynamic);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StaticMesh->SetupAttachment(RootComponent);
}

void
EditorInit()
{
}

#if WITH_EDITOR
void
AMLCharacter::PostEditMove(bool bFinished)
{
}

void
AMLCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UpdateEditorProperties();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// Called when the game starts or when spawned
void
AMLCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("We are using MLCharacter."));
    }
    UpdateEditorProperties();
}

// Called every frame
void
AMLCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickSensors();
    if (!CameraInput.IsZero())
    {
        FRotator NewRotation = camera_spring_arm->GetTargetRotation();
        NewRotation.Yaw += CameraInput.X;
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -88.0f, 30.0f);
        camera_spring_arm->SetWorldRotation(NewRotation);
    }

    if (!MovementInput.IsZero())
    {
        // Scale our movement input axis values by 100 units per second
        MovementInput = MovementInput.GetSafeNormal() * 400.0f;
        FVector NewLocation = GetActorLocation();
        NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
        NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

// Called to bind functionality to input
void
AMLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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

void
AMLCharacter::MoveForward(float AxisValue)
{
    MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
void
AMLCharacter::MoveRight(float AxisValue)
{
    MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void
AMLCharacter::StartJump()
{
    bPressedJump = true;
}

void
AMLCharacter::StopJump()
{
    bPressedJump = false;
}

void
AMLCharacter::PostLoad()
{
    Super::PostLoad();
    UE_LOG(LogTemp, Warning, TEXT("PostLoad"));
}

void
AMLCharacter::PostActorCreated()
{
    Super::PostActorCreated();
    UE_LOG(LogTemp, Warning, TEXT("PostActorCreated"));
    UpdateEditorProperties();
}

void
AMLCharacter::calculate_fitness()
{
    fitness = 0;
}

void
AMLCharacter::UpdateComponentLocations()
{
    UE_LOG(LogTemp, Warning, TEXT("UPP"));

    FVector root_world_location = RootComponent->GetComponentLocation();
    SetActorLocation(root_world_location);
    root_world_location = RootComponent->GetComponentLocation();
    GetCapsuleComponent()->SetWorldLocation(root_world_location);
    if (StaticMesh)
    {
        StaticMesh->SetWorldLocation(root_world_location);
    }
    if (camera_spring_arm)
    {
        camera_spring_arm->SetRelativeLocationAndRotation(FVector::ZeroVector,
                                                          FRotator(-60.0f, 0.f, 0.0f));
        camera_spring_arm->SetWorldLocation(root_world_location);
    }
    if (attached_camera)
    {
        FVector socket_location;
        FRotator socket_rotation;
        camera_spring_arm->GetSocketWorldLocationAndRotation(
          USpringArmComponent::SocketName, socket_location, socket_rotation);
        attached_camera->SetWorldLocationAndRotation(socket_location, socket_rotation);
    }
    HandleSingleAttachment(StaticMesh, FrontLeftSensor, FrontRightSensorSocket);
    HandleSingleAttachment(StaticMesh, FrontRightSensor, FrontLeftSensorSocket);
    HandleSingleAttachment(StaticMesh, BackRightSensor, BackLeftSensorSocket);
    HandleSingleAttachment(StaticMesh, BackLeftSensor, BackRightSensorSocket);
}

void
AMLCharacter::HandleAttachments()
{
    if (attached_camera)
    {
        /*FDetachmentTransformRules detachmenRule(EDetachmentRule::KeepRelative, false);
        attached_camera->DetachFromComponent(detachmenRule);
        FVector socket_location;
        FRotator socket_rotation;
        camera_spring_arm->GetSocketWorldLocationAndRotation(USpringArmComponent::SocketName,
        socket_location, socket_rotation);
        attached_camera->SetWorldLocationAndRotation(socket_location, socket_rotation);
        attached_camera->SetupAttachment(camera_spring_arm, USpringArmComponent::SocketName);
        UE_LOG(LogTemp, Warning, TEXT("Attached Camera"));*/
    }
}

void
AMLCharacter::UpdateEditorProperties()
{
    UpdateComponentLocations();
    HandleAttachments();
}

void
AMLCharacter::HandleSingleAttachment(USceneComponent* ParentComponent,
                                     USceneComponent* ComponentToAttach,
                                     FName& SocketName)
{
    if (ComponentToAttach && ParentComponent)
    {
        FVector socket_location;
        FRotator socket_rotation;
        ParentComponent->GetSocketWorldLocationAndRotation(
          SocketName, socket_location, socket_rotation);
        ComponentToAttach->SetWorldLocationAndRotation(socket_location, socket_rotation);
    }
}

void
AMLCharacter::TickSensors()
{
    FrontLeftSensor->RayCast(GetActorLocation(), GetActorForwardVector(), GetActorRightVector());
    FrontRightSensor->RayCast(GetActorLocation(), GetActorForwardVector(), GetActorRightVector());
    BackRightSensor->RayCast(GetActorLocation(), GetActorForwardVector(), GetActorRightVector());
    BackLeftSensor->RayCast(GetActorLocation(), GetActorForwardVector(), GetActorRightVector());
}

void
AMLCharacter::CheckPointTest(void* ptr)
{
    UE_LOG(LogTemp, Warning, TEXT("GOOD JOB M8"));
}

// Input functions
void
AMLCharacter::PitchCamera(float AxisValue)
{
    CameraInput.Y = AxisValue;
}
void
AMLCharacter::YawCamera(float AxisValue)
{
    CameraInput.X = AxisValue;
}

void
AMLCharacter::CameraZoom(float AxisValue)
{
    if (AxisValue != 0)
    {
        // attached_camera->FieldOfView += ::Lerp<float>(90.0f, 60.0f, ZoomFactor * AxisValue);
        camera_spring_arm->TargetArmLength += ZoomFactor * AxisValue;
    }
}

void
AMLCharacter::reset_player()
{
    fitness = 0;
    checkpoint_count = 0;
    // TODO_OGUZ RESET LOCATION HERE TO RESPAWN
}
