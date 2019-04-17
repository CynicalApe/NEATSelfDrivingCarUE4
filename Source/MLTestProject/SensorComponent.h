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
    void RayCast(const FVector& SocketLocation,
                 const FVector& SocketForward,
                 const FVector& SocketRight);

    void SetRayCastDirections(bool CastForward = true,
                              bool CastBehind = true,
                              bool CastRight = true,
                              bool CastLeft = true,
                              bool CastUp = true,
                              bool CastDown = true);

    // UPROPERTY(EditAnywhere, Category = "Ray Casting")
    float RayDistanceFromCenter = 20.0f;

    // UPROPERTY(EditAnywhere, Category = "Ray Casting")
    float RayTravelDistance = 100.0f;

    // UPROPERTY(EditAnywhere, Category = "Ray Casting")
    float RayWidth = 2.0f;

    int DirectionCount = 6;
    FVector* RayDirections;
    FHitResult* RayHitResults;
    bool* HitRays;
    bool* CastDirections;

    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastForward;
    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastBehind;
    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastRight;
    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastLeft;

    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastUp;
    UPROPERTY(EditAnywhere, Category = "Ray Casting")
    bool CastDown;

    class USceneComponent* RootComponent;
    class UStaticMeshComponent* StaticMesh;

  private:
    bool CastRayInDirection(const FVector& RayDirection,
                            const FVector& RayStart,
                            FHitResult& HitResult);
};
