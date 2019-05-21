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
    root_component = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    static_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
      TEXT("/Game/sensor2/sensor_sphere"));
    if (MeshAsset.Succeeded())
    {
        UStaticMesh* Asset = MeshAsset.Object;
        static_mesh->SetStaticMesh(Asset);
    }
    static_mesh->SetMobility(EComponentMobility::Movable);
    static_mesh->SetVisibility(true);
    static_mesh->SetGenerateOverlapEvents(false);
    static_mesh->SetCollisionObjectType(ECC_WorldDynamic);
    static_mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    static_mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
    static_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    root_component->SetupAttachment(this);
    static_mesh->SetupAttachment(root_component);
    world = GetWorld();
}

// Called when the game starts
void
USensorComponent::BeginPlay()
{
    Super::BeginPlay();
    ray_directions = (FVector*)malloc(sizeof(FVector) * direction_count);
    ray_hit_results = (FHitResult*)malloc(sizeof(FHitResult) * direction_count);
    hit_rays = (bool*)malloc(sizeof(bool) * direction_count);
    cast_directions = (bool*)malloc(sizeof(bool) * direction_count);
    cast_directions[0] = cast_forward;
    cast_directions[1] = cast_right;
    cast_directions[2] = cast_left;
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
    free(ray_directions);
    free(ray_hit_results);
    free(hit_rays);
    free(cast_directions);
    ray_directions = NULL;
    ray_hit_results = NULL;
    hit_rays = NULL;
    cast_directions = NULL;
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
USensorComponent::set_ray_cast_directions(bool CastForward, bool CastRight, bool CastLeft)
{
    cast_forward = CastForward;
    cast_right = CastRight;
    cast_left = CastLeft;
}
