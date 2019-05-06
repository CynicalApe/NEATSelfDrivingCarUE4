// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MLConnection.h"
#include <Runtime/Core/Public/Containers/Array.h>
class MLGenome;
/**
 *
 */
class MLTESTPROJECT_API MLInnovation
{
  public:
    MLInnovation();
    MLInnovation(int f_node, int t_node, int i_number, TArray<int>& inno_numbers);
    ~MLInnovation();

    bool is_same_innovation(int from_node, int to_node);
    int from_node;
    int to_node;
    int innovation_number;
    TArray<int> innovation_history;
};
