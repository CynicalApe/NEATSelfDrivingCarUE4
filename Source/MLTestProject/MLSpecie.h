// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MLGenome.h"
#include "MLCharacter.h"
#include <Runtime/Core/Public/Containers/Array.h>
#include "CoreMinimal.h"

/**
 *
 */
class MLTESTPROJECT_API MLSpecie
{
  public:
    MLSpecie();
    MLSpecie(AMLCharacter* player);
    ~MLSpecie();

    void add_player(AMLCharacter* player);
    void get_excess_disjoint_and_w_difference(const MLGenome& genome1,
                                              const MLGenome& genome2,
                                              int& excess_count,
                                              float& w_difference);
    bool is_same_specie(AMLCharacter* player);
    void explicit_fitness_sharing();
    void update_avg_fitness();
    void sort();
    AMLCharacter* select_player();
    void remove_bottom_half();
    void clear_players();

    float excess_disjoint_constant;
    float weight_constant;
    float similarity_constant;
    int staleness;
    int gens_without_improvement;

    TArray<AMLCharacter*> players;
    float best_fitness;
    float avg_fitness;
	float best_score;
	float avg_score;
    MLGenome representetive;
};
