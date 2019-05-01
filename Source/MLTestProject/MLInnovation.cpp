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
MLInnovation::is_same_innovation(TArray<MLConnection>& genome_connections,
                                 int out_from_node,
                                 int out_to_node)
{
    return (out_from_node == from_node && out_to_node == to_node);
}

// bool
// MLInnovation::is_same_innovation(TArray<MLConnection>& genome_connections,
//                                 int out_from_node,
//                                 int out_to_node)
//{
//    if (genome_connections.Num() == innovation_number)
//    {
//        if (out_from_node == from_node && out_to_node == to_node)
//        {
//            for (auto& connection : genome_connections)
//            {
//                bool found_connection;
//                for (auto& innovation_number : innovation_history)
//                {
//                    if (connection.innovation_number == innovation_number)
//                        found_connection = true;
//                }
//                if (!found_connection)
//                {
//                    return false;
//                }
//            }
//            return true;
//        }
//    }
//}
