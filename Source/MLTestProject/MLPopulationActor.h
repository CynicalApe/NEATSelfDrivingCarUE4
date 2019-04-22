// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLCharacter.h"
#include "MLGenome.h"
#include "MLNode.h"
#include "MLConnection.h"
#include "MLInnovation.h"
#include "MLSpecie.h"
#include "GameFramework/Actor.h"
#include <Runtime/Engine/Classes/Engine/World.h>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
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

    void spawn_new_batch();
    void crossover_and_add(AMLCharacter* parent1, AMLCharacter* parent2);
    void sort(); // sort by avg or best??
    void remove_stale_species();
    void set_best();
    void create_child(int specie_index);
    void remove_half();
    void remove_useless_species();
    void calculate_fitness();
    void classify_species();
    void new_generation();
    int select_specie();

    TArray<AMLCharacter*> players;
    TArray<MLInnovation> innovation_history;
    TArray<MLSpecie> species;
    TArray<MLGenome> generation_best_brains;
    MLGenome best_brain;
    int current_geneneration;
    int cur_gen_best_score;
    int overall_best_score;
    int simulation_batch;
    int render_batch;
    TArray<AActor*> start_point_arr;
    TArray<MLGenome> children_genomes;
    AActor* default_start_point;
    FName start_point_tag;

    int curr_max_check_point;

    UPROPERTY(EditAnywhere)
    int population_size;
};
