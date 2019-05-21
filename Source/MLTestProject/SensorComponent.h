// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "SensorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MLTESTPROJECT_API USensorComponent : public USceneComponent
{
    GENERATED_BODY()

  public:
    // Sets default values for this component's properties
    USensorComponent();

  protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
    virtual void UninitializeComponent();
    void FreeAllocations();

  public:
    // Called every frame
    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    inline void ray_cast(const FVector& SocketForward,
                         const FVector& SocketRight,
                         TArray<float>& sensor_outputs,
                         int starting_index,
                         bool draw_debug_lines = false)
    {
        FVector Start = GetComponentLocation();
        ray_directions[0] = SocketForward;
        ray_directions[1] = SocketRight;
        ray_directions[2] = -SocketRight;
        for (int i = 0; i < 3; i++)
        {
            if (!cast_directions[i])
            {
                continue;
            }
            real_ray_start = ray_directions[i] * ray_distance_from_center + Start;
            ray_end = (ray_directions[i] * ray_travel_distance) + real_ray_start;
            if (!world->LineTraceSingleByChannel(
                  ray_hit_results[i], real_ray_start, ray_end, ECC_WorldStatic))
            {
                sensor_outputs[starting_index++] = ray_travel_distance;
                if (draw_debug_lines)
                {
                    DrawDebugLine(world,
                                  real_ray_start,
                                  ray_end,
                                  FColor::Green,
                                  false,
                                  0.0333333333333f,
                                  0,
                                  ray_width);
                }
            }
            else
            {
                sensor_outputs[starting_index++] = ray_hit_results[i].Distance;
                if (draw_debug_lines)
                {
                    DrawDebugLine(world,
                                  real_ray_start,
                                  ray_end,
                                  FColor::Red,
                                  false,
                                  0.0333333333333f,
                                  0,
                                  ray_width);
                }
            }
        }
    }
    void set_ray_cast_directions(bool CastForward = false,
                                 bool CastRight = false,
                                 bool CastLeft = false);

    float ray_distance_from_center = 20.0f;
    float ray_travel_distance = 1000.0f;
    float ray_width = 15.0f;

    int direction_count = 3;
    FVector* ray_directions;
    FHitResult* ray_hit_results;
    bool* hit_rays;
    bool* cast_directions;

    bool cast_forward = false;
    bool cast_right = false;
    bool cast_left = false;
    FVector real_ray_start;
    FVector ray_end;
    class UWorld* world;
    class USceneComponent* root_component;
    class UStaticMeshComponent* static_mesh;

  private:
};
