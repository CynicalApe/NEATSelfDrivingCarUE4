// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MLConnection.h"
#include <Runtime/Core/Public/Containers/Array.h>
#include "MLRandomNumberGenerator.h"
#include "CoreMinimal.h"

class MLConnection;
/**
 *
 */
class MLTESTPROJECT_API MLNode
{
  public:
    MLNode();
    MLNode(int node_number, int layer, TArray<MLConnection>* ptr);
    ~MLNode();

    void feed_forward(TArray<MLNode>& genome_nodes);
    void add_connection(const MLConnection& connection);
    bool is_connected(const MLNode& node, TArray<MLNode>& genome_nodes);
    MLNode operator=(const MLNode& src);
    int number;
    float input;
    float output;
    int layer;

    TArray<MLConnection> output_connections;
};
