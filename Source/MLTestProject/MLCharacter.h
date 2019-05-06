// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLGenome.h"
#include "GameFramework/Character.h"
#include "MLCharacter.generated.h"
#define SIMULATE_ML 1

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
#if !SIMULATE_ML
    virtual void Tick(float DeltaTime) override;
#endif

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

    // Character Functions
    void update(float DeltaTime);
    void reset_player(const FVector& start_point_location, const FRotator& start_point_rotation);

    // Input functions
    void pitch_camera(float AxisValue);
    void yaw_camera(float AxisValue);
    void camera_zoom(float AxisValue);
    void move_forward(float Value);
    void move_right(float Value);
    void check_point_update(void* ptr);

    // ML Functions
    void tick_sensors();
    void calculate_score();
    void update_pos(float dt, float steering_input);
    void update_rotation(float dt, float acceleration_input);
    void update_actor_vectors();
    void update_network_inputs();
    void update_char_w_network_output(float DeltaTime);
    void handle_char_camera();
    void kill_if_stale(float DeltaTime);

    float normalize(float val, float min, float max);
    inline float fvector_lenght(const FVector& vec)
    {
        return FMath::Sqrt(vec.X * vec.X + vec.Y * vec.Y + vec.Z * vec.Z);
    }

    void* first_check_point = NULL;
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
    TArray<float> network_inputs;
    float current_speed;
    // -1 Left, 1 Right
    float steering_direction;
    FVector acceleration_vector;
    FVector velocity_vector;
    FVector lateral_velocity;
    FVector lateral_friction;
    FVector backward_friction;

    float ML_input_count;
    float max_sensor_input;
    const int ML_sensor_count = 8;
    const int ML_output_count = 2;
    const int velocity_input_index = 8;
    const float check_point_score_mult = 100;

    UPROPERTY(EditAnywhere)
    float max_speed = 50.f;
    UPROPERTY(EditAnywhere)
    float brake_mult = 250;
    UPROPERTY(EditAnywhere)
    float thrust = 40.f;
    UPROPERTY(EditAnywhere)
    float rotation_speed = 150;
    UPROPERTY(EditAnywhere)
    float lateral_friction_const = 6;
    UPROPERTY(EditAnywhere)
    float backward_friction_const = 0.6;
    UPROPERTY(EditAnywhere)
    float stale_timer = 0;
    UPROPERTY(EditAnywhere)
    float destruction_distance = 50.0f;
    UPROPERTY(EditAnywhere)
    float stale_limit = 3.5f;
    bool has_crashed = false;

    // FOR DEBUG
    int pushed_frame_count = 0;
    int read_frame_count = 0;
    TArray<float> inputs;
    TArray<float> outputs;
    TArray<float> prev_inputs;
    TArray<float> prev_outputs;
    int index;

    // Actor vectors
    FVector actor_position;
    FVector actor_forward;
    FVector actor_right;
    TArray<float> network_output;

  private:
};
