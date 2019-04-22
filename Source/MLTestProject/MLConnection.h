// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MLRandomNumberGenerator.h"
#include "CoreMinimal.h"
#define MUTATION_SCALE 0.02f // 1 / 50
class MLNode;
/**
 *
 */
class MLTESTPROJECT_API MLConnection
{
  public:
    MLConnection();
    MLConnection(int f_node, int t_node, double weight, int in_number);
    ~MLConnection();
    void mutate();
    int from_node;
    int to_node;
    double weight;
    int innovation_number;
    bool enabled;
};
