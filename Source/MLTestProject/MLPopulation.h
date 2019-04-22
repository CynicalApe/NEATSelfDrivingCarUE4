// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MLCharacter.h"
#include "MLInnovation.h"
#include "MLSpecie.h"
#include <Runtime/Engine/Classes/GameFramework/Actor.h>
#include <Runtime/Engine/Classes/Engine/World.h>

#include "CoreMinimal.h"

/**
 *
 */
class MLTESTPROJECT_API MLPopulation
{
  public:
    MLPopulation();
    MLPopulation(int size);
    ~MLPopulation();
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
