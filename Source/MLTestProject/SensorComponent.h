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

    inline void RayCastRight(const FVector& SocketForward,
                             const FVector& SocketRight,
                             TArray<float>& sensor_outputs,
                             int starting_index)
    {
        FVector Start = GetComponentLocation();
        RayDirections[0] = SocketForward;
        RayDirections[1] = SocketRight;
        for (int i = 0; i < 2; i++)
        {
            RealRayStart = RayDirections[i] * RayDistanceFromCenter + Start;
            RayEnd = (RayDirections[i] * RayTravelDistance) + RealRayStart;
            if (!world->LineTraceSingleByChannel(
                  RayHitResults[i], RealRayStart, RayEnd, ECC_WorldStatic))
            {
                sensor_outputs[starting_index++] = RayTravelDistance;
                /*DrawDebugLine(
                  world, RealRayStart, RayEnd, FColor::Green, false, -1.0f, 0, RayWidth);*/
            }
            else
            {
                sensor_outputs[starting_index++] = RayHitResults[i].Distance;
                /*     DrawDebugLine(world, RealRayStart, RayEnd, FColor::Red, false, -1.0f, 0,
                 * RayWidth);*/
            }
        }
    }
    inline void RayCastLeft(const FVector& SocketForward,
                            const FVector& SocketRight,
                            TArray<float>& sensor_outputs,
                            int starting_index)
    {
        FVector Start = GetComponentLocation();
        RayDirections[0] = SocketForward;
        RayDirections[1] = -SocketRight;
        for (int i = 0; i < 2; i++)
        {
            RealRayStart = RayDirections[i] * RayDistanceFromCenter + Start;
            RayEnd = (RayDirections[i] * RayTravelDistance) + RealRayStart;
            if (!world->LineTraceSingleByChannel(
                  RayHitResults[i], RealRayStart, RayEnd, ECC_WorldStatic))
            {
                sensor_outputs[starting_index++] = RayTravelDistance;
               /* DrawDebugLine(
                  world, RealRayStart, RayEnd, FColor::Green, false, -1.0f, 0, RayWidth);*/
            }
            else
            {
                sensor_outputs[starting_index++] = RayHitResults[i].Distance;
                /*       DrawDebugLine(world, RealRayStart, RayEnd, FColor::Red, false, -1.0f, 0,
                 * RayWidth);*/
            }
        }
    }

    void SetRayCastDirections(bool CastForward = false,
                              bool CastRight = false,
                              bool CastLeft = false);

    float RayDistanceFromCenter = 20.0f;
    float RayTravelDistance = 1000.0f;
    float RayWidth = 15.0f;

    int DirectionCount = 3;
    FVector* RayDirections;
    FHitResult* RayHitResults;
    bool* HitRays;
    bool* CastDirections;

    bool CastForward = false;
    bool CastRight = false;
    bool CastLeft = false;
    FVector RealRayStart;
    FVector RayEnd;
    class UWorld* world;
    class USceneComponent* RootComponent;
    class UStaticMeshComponent* StaticMesh;

  private:
};
