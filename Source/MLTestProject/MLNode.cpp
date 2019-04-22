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

MLNode::MLNode(int node_number, int layer /*= 0*/, TArray<MLConnection>* ptr)
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
MLNode::feed_forward(TArray<MLNode>& genome_nodes)
{
    if (layer == 0)
        output = input;
    else
        output = tanh(input);

    for (auto& it : output_connections)
    {
        if (it.enabled)
            genome_nodes[it.to_node].input += output * it.weight;
    }
}

void
MLNode::add_connection(const MLConnection& connection)
{
    output_connections.Add(connection);
}

bool
MLNode::is_connected(const MLNode& node, TArray<MLNode>& genome_nodes)
{
    if (node.layer == layer)
        return false;
    if (layer > node.layer)
    {
        for (auto& connection : node.output_connections)
        {
            if (number == genome_nodes[connection.to_node].number)
                return true;
        }
    }
    else
    {
        for (auto& connection : output_connections)
        {
            if (node.number == genome_nodes[connection.to_node].number)
                return true;
        }
    }

    return false;
}

MLNode
MLNode::operator=(const MLNode& src)
{
    MLNode node;
    node.number = src.number;
    node.input = src.input;
    node.output = src.output;
    node.layer = src.layer;
    node.output_connections.Reserve(src.output_connections.Num());
    for (auto& it : src.output_connections)
    {
        node.output_connections.Add(it);
    }
    return node;
}
