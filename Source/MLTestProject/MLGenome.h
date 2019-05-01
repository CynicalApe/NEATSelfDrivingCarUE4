// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MLConnection.h"
#include "MLNode.h"
#include "MLInnovation.h"
#include "CoreMinimal.h"

/**
 *
 */
class MLTESTPROJECT_API MLGenome
{
  public:
    MLGenome();
    MLGenome(int input_count, int output_count, bool cross_over = false);
    MLGenome(const MLGenome& src);
    void create_empty_genome(int icount, int ocount, bool cross_over);
    ~MLGenome();

    void new_connection(int from_node,
                        int to_node,
                        TArray<MLInnovation>& innovation_history,
                        float weight = -2);
    void init_connect(TArray<MLInnovation>& innovation_history);
    int add_innovation_to_history(TArray<MLInnovation>& innovation_history,
                                  int from_node,
                                  int to_node);

    void create_nodes_output_connection();
    void add_random_connection(TArray<MLInnovation>& innovation_history);
    bool is_fully_connected();
    bool can_make_connection(MLNode& node1, MLNode& node2);
    bool has_matching_innovation(int innovation_number, MLConnection& out_connection);
    TArray<float> feed_forward(TArray<float>& sensor_inputs);
    void add_node_between(int f_node, int t_node, TArray<MLInnovation>& innovation_history);
    void add_random_node(TArray<MLInnovation>& innovation_history);
    void mutate(TArray<MLInnovation>& innovation_history);
    void remove_all_connections();
    void reset_genome();
    MLGenome operator=(const MLGenome& src);

    int input_count;
    int output_count;
    int node_count;
    int layer_count;
    int bias_node_index;
    TArray<MLNode> nodes;
    TArray<MLConnection> connections;
};
