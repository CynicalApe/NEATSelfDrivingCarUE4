// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLCharacter.h"
#include "MLInnovation.h"
#include "MLSpecie.h"
#include "GameFramework/Actor.h"
#include "MLPopulationActor.generated.h"

UCLASS()
class MLTESTPROJECT_API AMLPopulationActor : public AActor
{
    GENERATED_BODY()

  public:
    // Sets default values for this actor's properties
    AMLPopulationActor();

  protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

  public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    TArray<AMLCharacter*> players;
    TArray<MLInnovation> innovation_history;
    TArray<MLSpecie> species;
    MLGenome best_brain;
    int current_geneneration;
    int cur_gen_best_score;
    int overall_best_score;
    TArray<MLGenome> generation_best_brains;
    int simulation_batch;
    int render_batch;
};
