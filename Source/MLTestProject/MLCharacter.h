// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLGenome.h"
#include "GameFramework/Character.h"
#include "MLCharacter.generated.h"

UCLASS()
class MLTESTPROJECT_API AMLCharacter : public ACharacter
{
    GENERATED_BODY()

  public:
    // Sets default values for this character's properties
    AMLCharacter();

  protected:
    // Override functions
    virtual void BeginPlay() override;

  public:
    // Override functions
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
#endif
    virtual void PostLoad() override;
    virtual void PostActorCreated() override;

    // Unreal functions
    void update_component_locations();
    void handle_attachments();
    void update_editor_properties();
    void handle_single_attachment(USceneComponent* ParentComponent,
                                  USceneComponent* ComponentToAttach,
                                  FName& SocketName);
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void handle_collision();
    void update(float DeltaTime);

    // Input functions
    void pitch_camera(float AxisValue);
    void yaw_camera(float AxisValue);
    void camera_zoom(float AxisValue);
    void move_forward(float Value);
    void move_right(float Value);

    // ML Functions
    void calculate_score();
    void update_pos(float dt, float steering_input);
    void update_rotation(float dt, float acceleration_input);
    void check_point_update(void* ptr);
    void tick_sensors();
    void reset_player(const FVector& start_point_location, const FRotator& start_point_rotation);
    void normalize(float& val, float min, float max);
    inline float fvector_lenght(const FVector& vec)
    {
        return FMath::Sqrt(vec.X * vec.X + vec.Y * vec.Y + vec.Z * vec.Z);
    }

    void* prev_check_point = NULL;
    void* prev_prev_check_point = NULL;

    class UStaticMeshComponent* StaticMesh = nullptr;
    class USpringArmComponent* camera_spring_arm = nullptr;
    class UCameraComponent* attached_camera = nullptr;

    class USensorComponent* FrontLeftSensor = nullptr;
    class USensorComponent* FrontRightSensor = nullptr;
    class USensorComponent* BackRightSensor = nullptr;
    class USensorComponent* BackLeftSensor = nullptr;
    FName FrontLeftSensorSocket;
    FName FrontRightSensorSocket;
    FName BackRightSensorSocket;
    FName BackLeftSensorSocket;

    // Input variables
    FVector2D MovementInput;
    FVector2D CameraInput;
    float ZoomFactor = 10.0f;
    bool bZoomingIn;

    MLGenome network;
    float fitness;
    float score;
    float check_point_score;
    int checkpoint_count;
    float last_check_point_time;
    float alive_time;
    bool is_elite = false;

    TArray<float> sensor_outputs;
    float current_speed;
    // -1 Left, 1 Right
    float steering_direction;
    FVector acceleration_vector;
    FVector velocity_vector;
    FVector lateral_velocity;
    FVector lateral_friction;
    FVector backward_friction;

    const float max_speed = 600.0f;
    float max_sensor_input;
    const int ML_input_count = 9;
    const int ML_output_count = 2;
    const int velocity_input_index = 8;
    const float check_point_score_mult = 100;

    float brake_mult = 250;
    float thrust = 500;
    float rotation_speed = 30;
    float lateral_friction_const = 15;
    float backward_friction_const = 0.1;
    float stale_timer = 0;
    const float destruction_distance = 50.0f;
    const float stale_limit = 3.5f;
    bool has_crashed = false;

    // FOR DEBUG
    int pushed_frame_count = 0;
    int read_frame_count = 0;
    TArray<float> inputs;
    TArray<float> outputs;
    TArray<float> prev_inputs;
    TArray<float> prev_outputs;
    int index;

  private:
};
