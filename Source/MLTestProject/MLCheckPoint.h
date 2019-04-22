// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Engine/Classes/Engine/TriggerBox.h>
#include "MLCharacter.h"
#include "MLCheckPoint.generated.h"

UCLASS()
class MLTESTPROJECT_API AMLCheckPoint : public ATriggerBox
{
    GENERATED_BODY()

  public:
    // Sets default values for this actor's properties
    AMLCheckPoint();

  protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

  public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    class UStaticMeshComponent* StaticMesh;

    UPROPERTY(VisibleAnywhere)
    int CheckpointNumber;

    // declare overlap begin function
    UFUNCTION()
    void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

    // declare overlap end function
    UFUNCTION()
    void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

    FName TriggerTag;
};
