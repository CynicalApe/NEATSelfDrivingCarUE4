// Fill out your copyright notice in the Description page of Project Settings.

#include "SensorComponent.h"
#include <Engine/Public/DrawDebugHelpers.h>
#include <Components/StaticMeshComponent.h>
#include <Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h>
#include <Engine/World.h>

// Sets default values for this component's properties
USensorComponent::USensorComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You
    // can turn these features off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
      TEXT("/Game/sensor2/sensor_sphere"));
    if (MeshAsset.Succeeded())
    {
        UStaticMesh* Asset = MeshAsset.Object;
        StaticMesh->SetStaticMesh(Asset);
    }
    StaticMesh->SetMobility(EComponentMobility::Movable);
    StaticMesh->SetVisibility(true);
    StaticMesh->SetGenerateOverlapEvents(false);
    StaticMesh->SetCollisionObjectType(ECC_WorldDynamic);
    StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    StaticMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

    RootComponent->SetupAttachment(this);
    StaticMesh->SetupAttachment(RootComponent);
    world = GetWorld();
}

// Called when the game starts
void
USensorComponent::BeginPlay()
{
    Super::BeginPlay();
    RayDirections = (FVector*)malloc(sizeof(FVector) * DirectionCount);
    RayHitResults = (FHitResult*)malloc(sizeof(FHitResult) * DirectionCount);
    HitRays = (bool*)malloc(sizeof(bool) * DirectionCount);
    CastDirections = (bool*)malloc(sizeof(bool) * DirectionCount);
    CastDirections[0] = CastForward;
    CastDirections[1] = CastBehind;
    CastDirections[2] = CastRight;
    CastDirections[3] = CastLeft;
    CastDirections[4] = CastUp;
    CastDirections[5] = CastDown;
}

void
USensorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    FreeAllocations();
}

void
USensorComponent::UninitializeComponent()
{
    FreeAllocations();
}

void
USensorComponent::FreeAllocations()
{
    free(RayDirections);
    free(RayHitResults);
    free(HitRays);
    free(CastDirections);
    RayDirections = NULL;
    RayHitResults = NULL;
    HitRays = NULL;
    CastDirections = NULL;
}

// Called every frame
void
USensorComponent::TickComponent(float DeltaTime,
                                ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void
USensorComponent::SetRayCastDirections(bool CastForward,
                                       bool CastBehind,
                                       bool CastRight,
                                       bool CastLeft,
                                       bool CastUp,
                                       bool CastDown)
{
    this->CastForward = CastForward;
    this->CastBehind = CastBehind;
    this->CastRight = CastRight;
    this->CastLeft = CastLeft;
    this->CastUp = CastUp;
    this->CastDown = CastDown;
}
