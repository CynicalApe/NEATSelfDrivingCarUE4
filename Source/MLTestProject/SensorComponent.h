// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
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
    inline void RayCast(const FVector& SocketForward,
                        const FVector& SocketRight,
                        const FVector& SocketUp,
                        TArray<float>& sensor_outputs,
                        int starting_index)
    {
        FVector Start = GetComponentLocation();
        RayDirections[0] = SocketForward;
        RayDirections[1] = -1 * RayDirections[0];
        RayDirections[2] = SocketRight;
        RayDirections[3] = -1 * RayDirections[2];
        RayDirections[4] = SocketUp;
        RayDirections[5] = -1 * RayDirections[4];

        for (int i = 0; i < DirectionCount; i++)
        {
            if (CastDirections[i])
            {
                RealRayStart = RayDirections[i] * RayDistanceFromCenter + Start;
                RayEnd = (RayDirections[i] * RayTravelDistance) + RealRayStart;
                if (!world->LineTraceSingleByChannel(
                      RayHitResults[i], RealRayStart, RayEnd, ECC_WorldDynamic))
                {
                    sensor_outputs[starting_index++] = RayTravelDistance;
                }
                else
                {
                    sensor_outputs[starting_index++] = RayHitResults[i].Distance;
                }
            }
        }
    }

    void SetRayCastDirections(bool CastForward = false,
                              bool CastBehind = false,
                              bool CastRight = false,
                              bool CastLeft = false,
                              bool CastUp = false,
                              bool CastDown = false);

    float RayDistanceFromCenter = 20.0f;
    float RayTravelDistance = 1000.0f;
    float RayWidth = 15.0f;

    int DirectionCount = 6;
    FVector* RayDirections;
    FHitResult* RayHitResults;
    bool* HitRays;
    bool* CastDirections;

    bool CastForward = false;
    bool CastBehind = false;
    bool CastRight = false;
    bool CastLeft = false;
    bool CastUp = false;
    bool CastDown = false;
    FVector RealRayStart;
    FVector RayEnd;
    class UWorld* world;
    class USceneComponent* RootComponent;
    class UStaticMeshComponent* StaticMesh;

  private:
};
