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
#define SIMULATE_ML 1

// Sets default values
AMLCharacter::AMLCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance
    // if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesoh"));
    attached_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
    camera_spring_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    FrontLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("FrontSensorComponent"));
    FrontRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("BehindSensorComponent"));
    BackLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("LeftSensorComponent"));
    BackRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("RightSensorComponent"));

    // Network
    network.create_empty_genome(ML_input_count, ML_output_count, false);

    fitness = 0;
    score = 0;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Meshes/CarMesh"));
    if (MeshAsset.Succeeded())
    {
        UStaticMesh* Asset = MeshAsset.Object;
        StaticMesh->SetStaticMesh(Asset);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("COULD NOT FIND THE ASSET FOR THE ML CAR!"));
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
    update_editor_properties();
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

    sensor_outputs.SetNum(ML_input_count);
    max_sensor_input = FrontRightSensor->RayTravelDistance;
    velocity_vector = FVector(0.f, 0.f, 0.f);
    acceleration_vector = FVector(0.f, 0.f, 0.f);
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
    update_editor_properties();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// Called when the game starts or when spawned
void
AMLCharacter::BeginPlay()
{
    Super::BeginPlay();
    update_editor_properties();
}

void
AMLCharacter::update(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (has_crashed)
    {
        return;
    }
    tick_sensors();
    normalize(current_speed, 0, max_speed);

    sensor_outputs[velocity_input_index] = current_speed;
    for (int i = 0; i < ML_input_count - 1; i++)
    {
        if (sensor_outputs[i] <= destruction_distance)
        {
            has_crashed = true;
        }
        normalize(sensor_outputs[i], 0, max_sensor_input);
    }
    TArray<float> output = network.feed_forward(sensor_outputs);

    if (!CameraInput.IsZero())
    {
        FRotator NewRotation = camera_spring_arm->GetTargetRotation();
        NewRotation.Yaw += CameraInput.X;
        NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -88.0f, 30.0f);
        camera_spring_arm->SetWorldRotation(NewRotation);
    }

#if SIMULATE_ML
    {
        update_pos(DeltaTime, output[0]);
        update_rotation(DeltaTime, output[1]);
    }
#else
    {
        update_pos(DeltaTime, MovementInput.X);
        update_rotation(DeltaTime, MovementInput.Y);
    }
#endif

    if (current_speed == 0)
    {
		stale_timer+= DeltaTime;
        if (stale_timer > stale_limit)
        {
			has_crashed = true;
		}
	}
}

// Called every frame

// Called to bind functionality to input
void
AMLCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Set up "movement" bindings.
    PlayerInputComponent->BindAxis("MoveForward", this, &AMLCharacter::move_forward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMLCharacter::move_right);
    PlayerInputComponent->BindAxis("MouseX", this, &AMLCharacter::yaw_camera);
    PlayerInputComponent->BindAxis("MouseY", this, &AMLCharacter::pitch_camera);
    PlayerInputComponent->BindAxis("MouseWheel", this, &AMLCharacter::camera_zoom);
}

void
AMLCharacter::move_forward(float AxisValue)
{
    MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
void
AMLCharacter::move_right(float AxisValue)
{
    MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void
AMLCharacter::PostLoad()
{
    Super::PostLoad();
}

void
AMLCharacter::PostActorCreated()
{
    Super::PostActorCreated();
    update_editor_properties();
}

void
AMLCharacter::calculate_score()
{
    // TODO_OGUZ: Tune this
    score = check_point_score + alive_time;
    fitness = score;
}

void
AMLCharacter::update_pos(float dt, float acceleration_input)
{
    FVector actor_right = GetActorRightVector();
    FVector actor_forward = GetActorForwardVector();
    lateral_velocity = actor_right * FVector::DotProduct(velocity_vector, actor_right);
    lateral_friction = -lateral_velocity * lateral_friction_const;
    backward_friction = -velocity_vector * backward_friction_const;
    acceleration_vector = actor_forward * acceleration_input * thrust;
    velocity_vector += (acceleration_vector * dt) + ((backward_friction + lateral_friction) * dt);
    current_speed = velocity_vector.Size();
    FMath::Clamp(current_speed, 0.0f, max_speed);
    if (FVector::DotProduct(velocity_vector, actor_forward) < 0)
    {
        velocity_vector = FVector::ZeroVector;
		current_speed = 0;
    }
    SetActorLocation(GetActorLocation() + velocity_vector * dt);
}

void
AMLCharacter::update_rotation(float dt, float steering_input)
{
    float angle = steering_input * rotation_speed * dt * (current_speed / max_speed);
    SetActorRotation(GetActorRotation().Add(0, angle, 0));
}

void
AMLCharacter::update_component_locations()
{

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
    handle_single_attachment(StaticMesh, FrontLeftSensor, FrontRightSensorSocket);
    handle_single_attachment(StaticMesh, FrontRightSensor, FrontLeftSensorSocket);
    handle_single_attachment(StaticMesh, BackRightSensor, BackLeftSensorSocket);
    handle_single_attachment(StaticMesh, BackLeftSensor, BackRightSensorSocket);
}

void
AMLCharacter::handle_attachments()
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
AMLCharacter::update_editor_properties()
{
    update_component_locations();
    handle_attachments();
}

void
AMLCharacter::handle_single_attachment(USceneComponent* ParentComponent,
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
AMLCharacter::handle_collision()
{
    has_crashed = true;
}

void
AMLCharacter::tick_sensors()
{
    FrontLeftSensor->RayCast(
      GetActorLocation(), GetActorForwardVector(), GetActorRightVector(), sensor_outputs, 0);
    FrontRightSensor->RayCast(
      GetActorLocation(), GetActorForwardVector(), GetActorRightVector(), sensor_outputs, 2);
    BackRightSensor->RayCast(
      GetActorLocation(), GetActorForwardVector(), GetActorRightVector(), sensor_outputs, 4);
    BackLeftSensor->RayCast(
      GetActorLocation(), GetActorForwardVector(), GetActorRightVector(), sensor_outputs, 6);
}

void
AMLCharacter::check_point_update(void* ptr)
{
    prev_check_point = ptr;
    checkpoint_count++;
    check_point_score += checkpoint_count / (alive_time - last_check_point_time);
    last_check_point_time = alive_time;
}

// Input functions
void
AMLCharacter::pitch_camera(float AxisValue)
{
    CameraInput.Y = AxisValue;
}
void
AMLCharacter::yaw_camera(float AxisValue)
{
    CameraInput.X = AxisValue;
}

void
AMLCharacter::camera_zoom(float AxisValue)
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

void
AMLCharacter::normalize(float& val, float min, float max)
{
    val = (2 * (val - min) / (max - min)) - 1;
}
