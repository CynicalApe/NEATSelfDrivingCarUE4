// Fill out your copyright notice in the Description page of Project Settings.
#include "MLConnection.h"
#include <Runtime/Core/Public/Math/Vector.h>
#include <Runtime/Core/Public/Math/UnrealMathUtility.h>

MLConnection::MLConnection()
  : from_node(0)
  , to_node(0)
  , weight(0)
  , innovation_number(0)
  , enabled(1)
{
}

MLConnection::MLConnection(int f_node, int t_node, double weight, int in_number)
  : from_node(f_node)
  , to_node(t_node)
  , weight(weight)
  , innovation_number(in_number)
  , enabled(1)
{
}

MLConnection::~MLConnection() {}

void
MLConnection::mutate()
{
    StaticRandomNumberGenerator.seed();
    float mutation_randomness = StaticRandomNumberGenerator.GetUniform(0, 1);
    if (mutation_randomness < 0.1f)
    {
        weight = StaticRandomNumberGenerator.GetUniform(-1, 1);
    }
    else
    {
        weight += StaticRandomNumberGenerator.GetNormal(-1, 1) * MUTATION_SCALE;
        weight = FMath::Clamp(weight, -1.0, 1.0);
    }
    check(weight <= 1.f && weight >= -1);
}
