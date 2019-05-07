// Fill out your copyright notice in the Description page of Project Settings.

#include "MLNode.h"
#include <Runtime/Core/Public/Math/Vector.h>

MLNode::MLNode()
  : number(0)
  , input(0)
  , output(0)
  , layer(0)
{
}

MLNode::MLNode(const MLNode& src)
{
    output_connections.Empty();
    output_connections.Reserve(src.output_connections.Num());
    number = src.number;
    input = src.input;
    output = src.output;
    layer = src.layer;
    for (auto& it : src.output_connections)
    {
        output_connections.Add(it);
    }
}

MLNode::MLNode(int node_number, int layer /*= 0*/, TArray<int>* ptr)
  : number(node_number)
  , input(0)
  , output(0)
  , layer(layer)
{
    // TODO_OGUZ: Maybe preallocate connections??
    if (ptr)
    {
        output_connections = *ptr;
    }
}

MLNode::~MLNode() {}

void
MLNode::feed_forward(TArray<MLNode>& genome_nodes, const TArray<MLConnection>& connections)
{
    if (layer == 0)
        output = input;
    else
        output = tanh(input);

    for (auto it : output_connections)
    {
        if (connections[it].enabled)
            genome_nodes[connections[it].to_node].input += output * connections[it].weight;
    }
}

void
MLNode::add_connection(int connection_index)
{
    output_connections.Add(connection_index);
}

bool
MLNode::is_connected(const MLNode& node, const TArray<MLConnection>& connections)
{
    if (node.layer == layer)
        return false;
    if (layer > node.layer)
    {
        for (auto& connection_index : node.output_connections)
        {

            if (number == connections[connection_index].to_node)
                return true;
        }
    }
    else
    {
        for (auto& connection_index : output_connections)
        {
            if (node.number == connections[connection_index].to_node)
                return true;
        }
    }

    return false;
}

void
MLNode::operator=(const MLNode& src)
{
    output_connections.Empty();
    output_connections.Reserve(src.output_connections.Num());
    number = src.number;
    input = src.input;
    output = src.output;
    layer = src.layer;
    for (auto& it : src.output_connections)
    {
        output_connections.Add(it);
    }
}
