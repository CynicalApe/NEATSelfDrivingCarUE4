// Fill out your copyright notice in the Description page of Project Settings.
#include "MLInnovation.h"
#include "MLNode.h"

MLInnovation::MLInnovation()
  : from_node(0)
  , to_node(0)
  , innovation_number(0)
{
}

MLInnovation::MLInnovation(int f_node, int t_node, int i_number, TArray<int>& inno_numbers)
{
    from_node = f_node;
    to_node = t_node;
    innovation_number = i_number;
    innovation_history.Empty();
    innovation_history.Reserve(inno_numbers.Num());
    for (auto& inno_no : inno_numbers)
    {
        innovation_history.Add(inno_no);
    }
}

MLInnovation::~MLInnovation() {}

bool
MLInnovation::is_same_innovation(int out_from_node, int out_to_node)
{
    return (out_from_node == from_node && out_to_node == to_node);
}
