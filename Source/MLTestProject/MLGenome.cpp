// Fill out your copyright notice in the Description page of Project Settings.

#include "MLGenome.h"
#include <cassert>
MLGenome::MLGenome()
  : input_count(0)
  , output_count(0)
  , node_count(0)
  , layer_count(0)
{
}

MLGenome::MLGenome(int icount, int ocount, bool cross_over)
  : input_count(icount)
  , output_count(ocount)
  , layer_count(2)
  , node_count(0)
{
    if (cross_over)
        return;
    nodes.Reserve(icount + ocount + 1);
    for (int i = 0; i < icount; i++)
    {
        nodes.Add(MLNode(node_count++, 0, NULL));
    }
    for (int i = 0; i < ocount; i++)
    {
        nodes.Add(MLNode(node_count++, 1, NULL));
    }
    bias_node_index = node_count++;
    nodes.Add(MLNode(bias_node_index, 0, NULL));
}

void
MLGenome::create_empty_genome(int icount, int ocount, bool cross_over)
{
    input_count = icount;
    output_count = ocount;
    layer_count = 2;
    node_count = 0;
    if (cross_over)
        return;
    nodes.Reserve(icount + ocount + 1);
    for (int i = 0; i < icount; i++)
    {
        nodes.Add(MLNode(node_count++, 0, NULL));
    }
    for (int i = 0; i < ocount; i++)
    {
        nodes.Add(MLNode(node_count++, 1, NULL));
    }
    bias_node_index = node_count++;
    nodes.Add(MLNode(bias_node_index, 0, NULL));
}

MLGenome::MLGenome(const MLGenome& src)
{
    input_count = src.input_count;
    output_count = src.output_count;
    layer_count = src.layer_count;
    node_count = src.node_count;
	
    nodes.Reserve(src.nodes.Num());
    for (auto& it : src.nodes)
    {
        nodes.Add(it);
    }

    connections.Reserve(src.connections.Num());
    for (auto& it : src.connections)
    {
        connections.Add(it);
    }
}

MLGenome::~MLGenome() {}

void
MLGenome::new_connection(int from_node,
                         int to_node,
                         TArray<MLInnovation>& innovation_history,
                         float weight)
{
    if (weight == -2)
    {
        StaticRandomNumberGenerator.seed();
        weight = StaticRandomNumberGenerator.GetUniform(-1, 1);
    }

    int connection_number = add_innovation_to_history(innovation_history, from_node, to_node);
    MLConnection connection = MLConnection(from_node, to_node, weight, connection_number);
    connections.Add(connection);
    nodes[from_node].output_connections.Add(connection);
}

void
MLGenome::init_connect(TArray<MLInnovation>& innovation_history)
{
    for (int i = 0; i < input_count; i++)
    {
        for (int j = input_count; j < input_count + output_count; j++)
        {
            new_connection(i, j, innovation_history);
        }
    }

    for (int i = input_count; i < input_count + output_count; i++)
    {
        new_connection(bias_node_index, i, innovation_history);
    }
}

int
MLGenome::add_innovation_to_history(TArray<MLInnovation>& innovation_history,
                                    int from_node,
                                    int to_node)
{
    bool is_new = true;
    int connection_number = innovation_history.Num();
    for (auto& innovation : innovation_history)
    {
        if (innovation.is_same_innovation(connections, from_node, to_node))
        {
            is_new = false;
            connection_number = innovation.innovation_number;
            break;
        }
    }
    TArray<int> inno_numbers;
    inno_numbers.Reserve(connections.Num());
    if (is_new)
    {
        for (auto& connection : connections)
        {
            inno_numbers.Add(connection.innovation_number);
        }
    }
    innovation_history.Add(MLInnovation(from_node, to_node, connection_number, inno_numbers));
    return connection_number;
}

void
MLGenome::create_nodes_output_connection()
{
    for (auto& connection : connections)
    {
        nodes[connection.from_node].add_connection(connection);
    }
}

void
MLGenome::add_random_connection(TArray<MLInnovation>& innovation_history)
{
    if (is_fully_connected())
    {
        return;
    }
    StaticRandomNumberGenerator.seed();
    int index_1 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
    int index_2 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
    while (can_make_connection(nodes[index_1], nodes[index_2]))
    {
        StaticRandomNumberGenerator.seed();
        index_2 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
    }

    if (nodes[index_1].layer > nodes[index_2].layer)
        new_connection(index_1, index_2, innovation_history);
    else
        new_connection(index_2, index_1, innovation_history);
}

bool
MLGenome::is_fully_connected()
{
    TArray<int> node_count_in_layer;
    node_count_in_layer.SetNumZeroed(layer_count, true);

    for (auto& node : nodes)
    {
        node_count_in_layer[node.layer] += 1;
    }
    int max_connections = 0;
    for (int i = 0; i < layer_count; i++)
    {
        for (int j = i + 1; j < layer_count; j++)
        {
            max_connections += node_count_in_layer[i] * node_count_in_layer[j];
        }
    }
    assert(max_connections >= connections.Num());
    return max_connections == connections.Num();
}

bool
MLGenome::can_make_connection(MLNode& node1, MLNode& node2)
{
    return (node1.layer != node2.layer) && !node1.is_connected(node2, nodes);
}

bool
MLGenome::has_matching_innovation(int innovation_number, MLConnection& out_connection)
{
    for (auto& connection : connections)
    {
        if (connection.innovation_number == innovation_number)
        {
            out_connection = connection;
            return true;
        }
    }
    return false;
}

TArray<float>
MLGenome::feed_forward(TArray<float>& sensor_inputs)
{
    TArray<float> result;
    result.Reserve(output_count);
    for (int i = 0; i < sensor_inputs.Num(); i++)
    {
        nodes[i].input = sensor_inputs[i];
    }
    for (auto& node : nodes)
    {
		node.output=0;
	}
    nodes[bias_node_index].output = 1.0f;

    for (int i = 0; i < layer_count; i++)
    {
        for (auto& node : nodes)
        {
            if (node.layer == i)
            {
                node.feed_forward(nodes);
            }
        }
    }

    for (int i = input_count; i < input_count + output_count; i++)
    {
        result.Add(nodes[i].output);
    }
    return result;
}

void
MLGenome::add_node_between(int f_node, int t_node, TArray<MLInnovation>& innovation_history)
{
    int new_node_layer = nodes[f_node].layer + 1;
    if (nodes[t_node].layer - nodes[f_node].layer)
    {
        for (auto& node : nodes)
        {
            if (node.layer >= new_node_layer)
            {
                node.layer += 1;
            }
        }
        layer_count += 1;
    }
    float weight = 0;
    for (auto& connection : nodes[f_node].output_connections)
    {
        if (connection.to_node == t_node)
        {
            connection.enabled = false;
            weight = connection.weight;
            break;
        }
    }
    nodes.Add(MLNode(node_count++, new_node_layer, NULL));
    new_connection(f_node, node_count - 1, innovation_history, 1);
    new_connection(node_count - 1, t_node, innovation_history, weight);
    new_connection(bias_node_index, node_count - 1, innovation_history, 1);
}

void
MLGenome::add_random_node(TArray<MLInnovation>& innovation_history)
{
    StaticRandomNumberGenerator.seed();
    int index_1 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
    int index_2 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
    while (index_1 == index_2 || index_1 == bias_node_index || index_2 == bias_node_index)
    {
        index_2 = (int)StaticRandomNumberGenerator.GetUniform(0, nodes.Num());
        StaticRandomNumberGenerator.seed();
    }

    if (index_1 > index_2)
        add_node_between(index_1, index_2, innovation_history);
    else
        add_node_between(index_2, index_1, innovation_history);
}

void
MLGenome::mutate(TArray<MLInnovation>& innovation_history,float mutation_constant)
{
    StaticRandomNumberGenerator.seed();
    float prop = StaticRandomNumberGenerator.GetUniform(0, 1);

    if (prop < connection_mutation_prob)
    {
        for (auto& connection : connections)
        {
            connection.mutate();
        }
    }

    StaticRandomNumberGenerator.seed();
    prop = StaticRandomNumberGenerator.GetUniform(0, 1);

    if (add_new_connection_prob * mutation_constant)
    {
        add_random_connection(innovation_history);
    }
    StaticRandomNumberGenerator.seed();
    prop = StaticRandomNumberGenerator.GetUniform(0, 1);

    if (prop < add_new_node_porb * mutation_constant)
    {
        add_random_node(innovation_history);
    }
}

void
MLGenome::remove_all_connections()
{
    connections.Empty();
    for (auto& it : nodes)
    {
        it.output_connections.Empty();
    }
}

void
MLGenome::reset_genome()
{
    for (auto& node : nodes)
    {
        node.input = 0.0f;
        node.output = 0.0f;
    }
}



void
MLGenome::operator=(const MLGenome& src)
{
    input_count = src.input_count;
    output_count = src.output_count;
    node_count = src.node_count;
    layer_count = src.layer_count;
    bias_node_index = src.bias_node_index;

    nodes.Reserve(src.nodes.Num());
    for (auto& it : src.nodes)
    {
        nodes.Add(it);
    }

    connections.Reserve(src.connections.Num());
    for (auto& it : src.connections)
    {
        connections.Add(it);
    }
}
