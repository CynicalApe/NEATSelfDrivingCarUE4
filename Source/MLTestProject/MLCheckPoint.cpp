// Fill out your copyright notice in the Description page of Project Settings.

#include "MLCheckPoint.h"
#include <Components/StaticMeshComponent.h>
#include <Engine/Classes/Engine/TriggerBox.h>
#include <Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h>

#include <Engine/Public/DrawDebugHelpers.h>

#define print(text)                                                                                \
    if (GEngine)                                                                                   \
    GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green, text)
#define printFString(text, fstring)                                                                \
    if (GEngine)                                                                                   \
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(text), fstring))

// Sets default values
AMLCheckPoint::AMLCheckPoint()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if
    // you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
      TEXT("/Game/Meshes/CheckPointMesh"));
    if (MeshAsset.Succeeded())
    {
        UStaticMesh* Asset = MeshAsset.Object;
        StaticMesh->SetStaticMesh(Asset);
    }
    StaticMesh->SetVisibility(true);
    // Register Events

    OnActorBeginOverlap.AddDynamic(this, &AMLCheckPoint::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AMLCheckPoint::OnOverlapEnd);
    TriggerTag = FName("MLTrigger");
    // TriggerBox->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void
AMLCheckPoint::BeginPlay()
{
    Super::BeginPlay();

    DrawDebugBox(GetWorld(),
                 GetActorLocation(),
                 GetComponentsBoundingBox().GetExtent(),
                 FColor::Purple,
                 true,
                 -1,
                 0,
                 5);
}

// Called every frame
void
AMLCheckPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void
AMLCheckPoint::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor->ActorHasTag(TriggerTag))
    {
        ((AMLCharacter*)OtherActor)->check_point_update(this);
    }
}

void
AMLCheckPoint::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
}
