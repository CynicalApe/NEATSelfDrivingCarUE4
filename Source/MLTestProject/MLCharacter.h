// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLGenome.h"
#include "GameFramework/Character.h"
#include <Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h>
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
    void update(float DeltaTime, bool draw_sensor_rays);
    void reset_player(const FVector& start_point_location, const FRotator& start_point_rotation);

    // Input functions
    void pitch_camera(float AxisValue);
    void yaw_camera(float AxisValue);
    void camera_zoom(float AxisValue);
    void move_forward(float Value);
    void move_right(float Value);
    void check_point_update(void* ptr);

    // ML Functions
    void tick_sensors(bool draw_debug_lines = false);
    void calculate_score();
    void update_pos(float dt, float steering_input);
    void update_rotation(float dt, float acceleration_input);
    void update_actor_vectors();
    void update_network_inputs();
    void update_char_w_network_output(float DeltaTime);
    void handle_char_camera();
    void kill_if_stale(float DeltaTime);
    void kill_if_checkpoint();
    void set_car_color(const FVector& color);

    float normalize(float val, float min, float max);
    inline float fvector_lenght(const FVector& vec)
    {
        return FMath::Sqrt(vec.X * vec.X + vec.Y * vec.Y + vec.Z * vec.Z);
    }

    void* first_check_point = NULL;
    void* prev_check_point = NULL;
    void* prev_prev_check_point = NULL;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* StaticMesh = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USensorComponent* FrontLeftSensor = nullptr;
    class USensorComponent* FrontRightSensor = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USensorComponent* BackRightSensor = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
    int lap_count = 0;
    const int max_lap_limit = 3;
    float fitness;
    float score;
    float check_point_score;
    float sensor_penalty_to_add = 0;
    int pushed_frame_count = 0;
    const float sensor_penalty_mult = .07f;
    float sensor_penalty = 0;
    int checkpoint_count;
    float last_check_point_time;
    float alive_time;

    const float checkpoint_deadline = 4.0f;

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
    const int ML_ray_count = 4;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool has_crashed = false;

    // FOR DEBUG

    int read_frame_count = 0;
    TArray<float> inputs;
    TArray<float> outputs;
    TArray<float> prev_inputs;
    TArray<float> prev_outputs;
    int index;
    float distance_traveled;

    // Actor vectors
    FVector actor_position;
    FVector actor_forward;
    FVector actor_up;
    FVector actor_right;
    TArray<float> network_output;

    UMaterialInstanceDynamic* material1;
    UMaterialInstanceDynamic* material2;
    FVector car_color_simple_red;
    FVector car_color_orange;
    FVector car_color_elite;
    bool dead_set = false;
    FVector actor_new_position;
    FRotator actor_new_rotation;

  private:
};
