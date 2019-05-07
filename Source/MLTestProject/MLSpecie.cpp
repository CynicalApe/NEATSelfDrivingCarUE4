// Fill out your copyright notice in the Description page of Project Settings.

#include "MLSpecie.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMath.h"

MLSpecie::MLSpecie()
{
    staleness = 0;
    best_fitness = 0;
    avg_fitness = 0;
    best_score = 0;
    avg_score = 0;
    gens_without_improvement = 0;
}

MLSpecie::MLSpecie(AMLCharacter* player)
{
    staleness = 0;
    best_fitness = 0;
    avg_fitness = 0;
    best_score = 0;
    avg_score = 0;
    gens_without_improvement = 0;

    players.Add(player);
    best_fitness = player->fitness;
    avg_fitness = best_fitness;
    best_score = player->score;
    avg_score = best_score;
    representetive = player->network;
}

MLSpecie::~MLSpecie() {}

void
MLSpecie::add_player(AMLCharacter* player)
{
    players.Add(player);
}

void
MLSpecie::get_excess_disjoint_and_w_difference(const MLGenome& genome1,
                                               const MLGenome& genome2,
                                               int& excess_count,
                                               float& w_difference)
{
    excess_count = 0;
    w_difference = 0;

    int mutual_count = 0;
    for (auto& it1 : genome1.connections)
    {
        for (auto& it2 : genome2.connections)
        {
            if (it1.innovation_number == it2.innovation_number)
            {
                mutual_count++;
                w_difference += FMath::Abs(it1.weight - it2.weight);
                break;
            }
        }
    }
    excess_count = genome1.connections.Num() + genome2.connections.Num() - 2 * mutual_count;
    if (mutual_count == 0)
    {
        w_difference = 10;
    }
    else
    {
        w_difference /= mutual_count;
    }
}

bool
MLSpecie::is_same_specie(AMLCharacter* player)
{
    MLGenome genome = player->network;
    // MLGenome::cpy(player->network, genome);

    int N;

    if (genome.nodes.Num() < 20 && representetive.nodes.Num() < 20)
    {
        N = 1;
    }
    else
    {
        N = FMath::Max(genome.nodes.Num(), representetive.nodes.Num());
    }
    float avg_w_difference = 0;
    int excess_disjoint_count = 0;
    get_excess_disjoint_and_w_difference(
      genome, representetive, excess_disjoint_count, avg_w_difference);
    float distance =
      excess_disjoint_constant * excess_disjoint_count / N + weight_constant * avg_w_difference;

    return similarity_constant > distance;
}

void
MLSpecie::explicit_fitness_sharing()
{
    for (auto& it : players)
    {
        it->fitness /= (float)players.Num();
    }
    best_fitness = players[0]->fitness;
}

void
MLSpecie::update_avg_fitness()
{
    float total_fitness = 0;
    float total_score = 0;
    for (auto& it : players)
    {
        total_fitness += it->fitness;
        total_score += it->score;
    }
    avg_fitness = total_fitness / players.Num();
    avg_score = total_score / players.Num();
}

struct player_fitness_predicate
{
    bool operator()(const AMLCharacter* player1, const AMLCharacter* player2) const
    {
        return player1->fitness > player2->fitness;
    }
};

void
MLSpecie::sort()
{
    if (players.Num() == 0)
    {
        staleness = 20;
        return;
    }

    Algo::Sort(players, player_fitness_predicate());

    if (players[0]->score > best_score)
    {
        best_score = players[0]->score;
    }

    if (players[0]->fitness > best_fitness)
    {
        staleness = 0;
        best_fitness = players[0]->fitness;
        representetive = players[0]->network;
    }
    else
    {
        staleness++;
    }
}

AMLCharacter*
MLSpecie::select_player()
{
    float total_fitness = 0;
    for (auto& it : players)
    {
        total_fitness += it->fitness;
    }

    StaticRandomNumberGenerator.seed();
    float random_fitness = StaticRandomNumberGenerator.GetUniform(0, total_fitness);

    float curr_sum = 0;
    for (auto& it : players)
    {
        curr_sum += it->fitness;
        if (curr_sum > random_fitness)
        {
            return it;
        }
    }
    return players[0];
}

void
MLSpecie::remove_bottom_half()
{
    if (players.Num() < 2)
    {
        return;
    }
    int length = players.Num() / 2;
    players.RemoveAt(length, players.Num() - length);
}

void
MLSpecie::clear_players()
{
    players.Empty();
}
