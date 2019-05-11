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
#if SIMULATE_ML
    PrimaryActorTick.bCanEverTick = false;
#else
    PrimaryActorTick.bCanEverTick = true;
#endif
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesoh"));
    FrontLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("FrontSensorComponent"));
    FrontRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("BehindSensorComponent"));
    BackLeftSensor = CreateDefaultSubobject<USensorComponent>(TEXT("LeftSensorComponent"));
    BackRightSensor = CreateDefaultSubobject<USensorComponent>(TEXT("RightSensorComponent"));
    GetCapsuleComponent()->SetupAttachment(RootComponent);

    // Network
    ML_input_count = ML_ray_count + 4; // sensors + two 2D vectors
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

    FrontLeftSensor->SetupAttachment(StaticMesh, FrontRightSensorSocket);
    FrontRightSensor->SetupAttachment(StaticMesh, FrontLeftSensorSocket);
    /*BackRightSensor->SetupAttachment(StaticMesh, BackLeftSensorSocket);
    BackLeftSensor->SetupAttachment(StaticMesh, BackRightSensorSocket);*/

    FrontRightSensor->SetRayCastDirections(true, true, false);
    FrontLeftSensor->SetRayCastDirections(true, false, true);
    /*BackLeftSensor->SetRayCastDirections(false, true, false, true);
    BackRightSensor->SetRayCastDirections(false, true, true);*/

    StaticMesh->SetMobility(EComponentMobility::Movable);
    StaticMesh->SetVisibility(true);
    StaticMesh->SetRelativeRotationExact(FRotator(0.0f, 90.0f, 0.0f));
    update_editor_properties();
    Tags.Add(FName("MLTrigger"));
    RootComponent->SetMobility(EComponentMobility::Movable);
    StaticMesh->SetGenerateOverlapEvents(false);
    StaticMesh->SetCollisionObjectType(ECC_WorldStatic);
    StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldStatic);
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    StaticMesh->SetupAttachment(RootComponent);

    network_inputs.SetNum(ML_input_count);
    sensor_outputs.SetNum(ML_ray_count);
    network_inputs.SetNum(ML_input_count);
    max_sensor_input = FrontRightSensor->RayTravelDistance;
    velocity_vector = FVector(0.f, 0.f, 0.f);
    acceleration_vector = FVector(0.f, 0.f, 0.f);
    index = 0;
    update_actor_vectors();
    car_color_simple_red = FVector(0.318, 0.0f, 0.0f);
    car_color_orange = FVector(1.0f, 0.275f, 0.0f);
    car_color_elite = FVector(0.615f, 0.0f, 0.963f);
    UMaterialInterface* Material1 = StaticMesh->GetMaterial(0);
    material1 = StaticMesh->CreateDynamicMaterialInstance(1, Material1);
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

#if !SIMULATE_ML
void
AMLCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    update_actor_vectors();
    tick_sensors();

    update_char_w_network_output(DeltaTime);
}
#endif
void
AMLCharacter::update(float DeltaTime)
{
#if SIMULATE_ML
    {
        if (!has_crashed)
        {
            pushed_frame_count++;

            Super::Tick(DeltaTime);
            update_actor_vectors();
            tick_sensors();
            update_network_inputs();
            update_char_w_network_output(DeltaTime);
            handle_char_camera();
            kill_if_stale(DeltaTime);
            kill_if_checkpoint();
            distance_traveled += velocity_vector.Size() * DeltaTime;
            alive_time += DeltaTime;
        }
    }
#endif
}

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
    sensor_penalty *= sensor_penalty_mult;
    score = check_point_score + distance_traveled - sensor_penalty;
	if (score <0)
	{
            score = 0;
	}
    // score = check_point_score;
    fitness = score;
}

void
AMLCharacter::update_pos(float dt, float acceleration_input)
{

    lateral_velocity = actor_right * FVector::DotProduct(velocity_vector, actor_right);
    lateral_friction = -lateral_velocity * lateral_friction_const;
    backward_friction = -velocity_vector * backward_friction_const;

    velocity_vector += (backward_friction + lateral_friction) * dt;

    acceleration_vector = actor_forward * acceleration_input * thrust;
    current_speed = velocity_vector.Size();
    // if (FVector::DotProduct(velocity_vector, actor_forward) < 0)
    //{
    //    current_speed = -current_speed;
    //}
    if (FVector::DotProduct(velocity_vector, actor_forward) < 0)
    {
        velocity_vector = FVector::ZeroVector;
    }

    if (current_speed < max_speed)
    {
        velocity_vector += acceleration_vector * dt;
    }
    actor_new_position = actor_position + velocity_vector;
}

void
AMLCharacter::update_rotation(float dt, float steering_input)
{
    float angle = steering_input * rotation_speed * dt * (current_speed / max_speed);
    actor_new_rotation = GetActorRotation().Add(0, angle, 0);
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
    handle_single_attachment(StaticMesh, FrontLeftSensor, FrontRightSensorSocket);
    handle_single_attachment(StaticMesh, FrontRightSensor, FrontLeftSensorSocket);
    handle_single_attachment(StaticMesh, BackRightSensor, BackLeftSensorSocket);
    handle_single_attachment(StaticMesh, BackLeftSensor, BackRightSensorSocket);
}

void
AMLCharacter::handle_attachments()
{
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
AMLCharacter::tick_sensors()
{
    FrontRightSensor->RayCastRight(actor_forward, actor_right, sensor_outputs, 2);
    FrontLeftSensor->RayCastLeft(actor_forward, actor_right, sensor_outputs, 0);
    /*  BackRightSensor->RayCast(actor_forward, actor_right, actor_up, sensor_outputs, 4);
      BackLeftSensor->RayCast(actor_forward, actor_right, actor_up, sensor_outputs, 6);*/
}

void
AMLCharacter::update_actor_vectors()
{
    actor_position = GetActorLocation();
    actor_forward = GetActorForwardVector();
    actor_right = GetActorRightVector();
    actor_up = GetActorUpVector();
}

void
AMLCharacter::check_point_update(void* ptr)
{
    if (ptr == prev_check_point || ptr == prev_prev_check_point)
    {
        has_crashed = true;
        return;
    }
    if (first_check_point == ptr)
    {
        lap_count++;
    }
    if (checkpoint_count == 0)
    {
        first_check_point = ptr;
    }
    checkpoint_count++;

 //   if (pushed_frame_count != 0)
 //   {
 //       sensor_penalty += sensor_score_to_add / pushed_frame_count;
 //   }
 //   else
 //   {
 //       sensor_penalty += sensor_score_to_add;
	//		
	//}
    sensor_penalty += sensor_penalty_to_add;
	sensor_penalty_to_add = 0;
    pushed_frame_count = 0;
    check_point_score +=
      check_point_score_mult * checkpoint_count / (alive_time - last_check_point_time);
    // check_point_score += check_point_score_mult * checkpoint_count;
    last_check_point_time = alive_time;

    if (lap_count == max_lap_limit)
    {
        has_crashed = true;
        return;
    }
    prev_prev_check_point = prev_check_point;
    prev_check_point = ptr;
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
    }
}

void
AMLCharacter::reset_player(const FVector& start_point_location,
                           const FRotator& start_point_rotation)
{
    fitness = 0;
    checkpoint_count = 0;
    sensor_penalty_to_add = 0;
    sensor_penalty = 0;
    pushed_frame_count = 0;
    lap_count = 0;
    score = 0;
    has_crashed = false;
    alive_time = 0;
    last_check_point_time = 0;
    check_point_score = 0;
    current_speed = 0;
    prev_check_point = NULL;
    prev_prev_check_point = NULL;
    acceleration_vector = FVector::ZeroVector;
    velocity_vector = FVector::ZeroVector;
    lateral_velocity = FVector::ZeroVector;
    lateral_friction = FVector::ZeroVector;
    backward_friction = FVector::ZeroVector;
    SetActorLocationAndRotation(start_point_location, start_point_rotation);
    // DEBUG
    index = 0;
    inputs.Empty();
    outputs.Empty();
    distance_traveled = 0;
    dead_set = false;
    set_car_color(car_color_orange);
}

float
AMLCharacter::normalize(float val, float min, float max)
{
    return (2 * (val - min) / (max - min)) - 1;
}

void
AMLCharacter::update_network_inputs()
{	
   sensor_penalty_to_add +=FMath::Abs(sensor_outputs[1]-sensor_outputs[3]); // left and right rays

    for (int i = 0; i < ML_ray_count; i++)
    {
        if (sensor_outputs[i] <= destruction_distance)
        {
            has_crashed = true;
        }
        network_inputs[i] = (normalize(sensor_outputs[i], 0, max_sensor_input));
    }
    network_inputs[ML_ray_count] = normalize(velocity_vector[0], -max_speed, max_speed);
    network_inputs[ML_ray_count + 1] = normalize(velocity_vector[1], -max_speed, max_speed);
    network_inputs[ML_ray_count + 2] = actor_forward.X;
    network_inputs[ML_ray_count + 3] = actor_forward.Y;
}

void
AMLCharacter::update_char_w_network_output(float DeltaTime)
{
    network_output = network.feed_forward(network_inputs);
#if SIMULATE_ML
    {
        update_pos(DeltaTime, network_output[0]);
        update_rotation(DeltaTime, network_output[1]);
    }
#else
    {
        update_pos(DeltaTime, MovementInput.X);
        update_rotation(DeltaTime, MovementInput.Y);
    }
#endif
}

void
AMLCharacter::handle_char_camera()
{
}

void
AMLCharacter::kill_if_stale(float DeltaTime)
{
    if (current_speed == 0)
    {
        stale_timer += DeltaTime;
        if (stale_timer > stale_limit)
        {
            has_crashed = true;
            stale_timer = 0;
        }
    }
}

void
AMLCharacter::kill_if_checkpoint()
{
    if (alive_time - last_check_point_time > checkpoint_deadline)
    {
        has_crashed = true;
    }
}

void
AMLCharacter::set_car_color(const FVector& color)
{
    material1->SetVectorParameterValue(FName(TEXT("CarColor")), color);
    StaticMesh->SetMaterial(0, material1);
    StaticMesh->SetMaterial(1, material1);
}
