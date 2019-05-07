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

typedef struct
{
    int mutation_count;
    int copy_count;
    int crossover;
    int interspecies_crossover;
} GENINFO;

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
    virtual void Tick(float DeltaTime) override;

  public:
    // Called every frame

    void update(float DeltaTime);
    void spawn_new_batch();
    void crossover_and_add(AMLCharacter* parent1, AMLCharacter* parent2);
    void sort(); // sort by avg or best??
    void set_best();
    void create_child(int specie_index);
    void update_fitness_per_specie();
    void remove_half();
    void remove_stale_species();
    void remove_useless_species();
    void calculate_all_player_fitness();
    void classify_species();
    void new_generation();
    void update_mutation_constant();
    void kill_based_on_avg_speed(float total_speed, int dead_count);

    int select_specie();
    // ML Storage
    TArray<MLSpecie> species;
    TArray<AMLCharacter*> players;
    TArray<MLGenome> children_genomes;
    TArray<MLInnovation> innovation_history;

    // ML info
    MLGenome best_brain;
    int population_size = 170;
    int staleness = 0;
    int staleness_constant = 3;
    int max_mutation_constant = 100;
    float mutation_constant = 1;
    float mutation_increase_rate = 0.8;
    float acculamator;
    const float interspecies_crossover_chance = 0.003;
    const float crossover_chance = 0.75;
    const float offspring_mutation_chance = 0.25;
    const float avg_speed_min_threshold = 3;
    const float dt = 1 / 30.0;
    TArray<MLConnection> prev_connections;

    // Runtime info
    TArray<MLGenome> generation_best_brains;
    int current_geneneration;
    float cur_gen_best_score;
    float overall_best_score;
    int car_no_to_simulate;
    int curr_max_check_point;
    float current_gen_time;

    // Debug info
    GENINFO generation_info{ 0 };
    float time_interval = 0;
    float prev_best_score = 0;

    // Editor
    TArray<AActor*> start_point_arr;
    FVector default_start_location;
    FRotator default_start_rotation;
    FName start_point_tag;
};
