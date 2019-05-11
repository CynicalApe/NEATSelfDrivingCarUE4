// Fill out your copyright notice in the Description page of Project Settings.
#include "MLPopulationActor.h"
#include <Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h>
#include <Runtime/Engine/Classes/Engine/StaticMesh.h>
#include <Runtime/Core/Public/Async/ParallelFor.h>
#include <cassert>

// Sets default values
AMLPopulationActor::AMLPopulationActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if
    // you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    start_point_tag = TEXT("PlayerStartTag");
    children_genomes.Reserve(population_size);
    current_geneneration = 1;
    acculamator = 0.f;
}

// Called when the game starts or when spawned
void
AMLPopulationActor::BeginPlay()
{
    Super::BeginPlay();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), start_point_tag, start_point_arr);
    if (start_point_arr.Num() >= 1)
    {
        default_start_location = start_point_arr[0]->GetActorLocation();
        default_start_rotation = start_point_arr[0]->GetActorRotation();
    }
    spawn_new_batch();
    for (auto& it : players)
    {
        it->network.init_connect(innovation_history);
        it->network.mutate(innovation_history, mutation_constant);
    }
}

// Called every frame
void
AMLPopulationActor::Tick(float DeltaTime)
{
    acculamator += DeltaTime;
    while (acculamator >= dt)
    {
        update(dt);
        acculamator -= dt;
    }
}

void
AMLPopulationActor::update(float DeltaTime)
{
    Super::Tick(DeltaTime);
    int dead_count = 0;
    ParallelFor(population_size, [&](int idx) { players[idx]->update(DeltaTime); });
    for (auto& player : players)
    {
        if (player->has_crashed)
        {
            if (!player->dead_set)
            {
                player->set_car_color(player->car_color_simple_red);
                player->dead_set = true;
            }
            dead_count++;
        }
        else
        {
            player->SetActorLocationAndRotation(player->actor_new_position,
                                                player->actor_new_rotation);
        }
    }
    if (dead_count == players.Num())
    {
        new_generation();
    }
}

void
AMLPopulationActor::spawn_new_batch()
{
    FActorSpawnParameters* param = new FActorSpawnParameters();
    param->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int i = 0; i < population_size; i++)
    {
        AMLCharacter* spawned_actor = GetWorld()->SpawnActor<AMLCharacter>(
          AMLCharacter::StaticClass(), default_start_location, default_start_rotation, *param);
        assert(spawned_actor->network.nodes.Num() != 0);
        spawned_actor->actor_position = default_start_location;
        players.Add(spawned_actor);
    }
    delete param;
}

void
AMLPopulationActor::crossover_and_add(AMLCharacter* parent1, AMLCharacter* parent2)
{
    MLGenome genome1, genome2;
    if (parent2->fitness > parent1->fitness)
    {
        genome1 = parent2->network;
        genome2 = parent1->network;
    }
    else
    {
        genome1 = parent1->network;
        genome2 = parent2->network;
    }
    MLGenome offspring;
    offspring = genome1;
    offspring.remove_all_connections();

    for (int i = 0; i < genome1.connections.Num(); i++)
    {
        MLConnection other_connection;
        if (genome2.has_matching_innovation(genome1.connections[i].innovation_number,
                                            other_connection))
        {
            bool is_enabled = true;
            if (!(genome1.connections[i].enabled && other_connection.enabled))
            {
                StaticRandomNumberGenerator.seed();
                if (StaticRandomNumberGenerator.GetUniform(0, 1) < 0.25f)
                {
                    is_enabled = false;
                }
            }

            StaticRandomNumberGenerator.seed();
            if (StaticRandomNumberGenerator.GetUniform(0, 1) < 0.5f)
            {
                offspring.connections.Add(genome1.connections[i]);
            }
            else
            {
                offspring.connections.Add(other_connection);
            }
            offspring.connections.Last().enabled = is_enabled;
        }
        else
        {
            offspring.connections.Add(genome1.connections[i]);
        }
    }
    offspring.create_nodes_output_connection();
    offspring.mutate(innovation_history, mutation_constant);
    children_genomes.Add(offspring);
}

inline static bool
specie_predicate(const MLSpecie& sp1, const MLSpecie& sp2)
{
    return sp1.avg_fitness > sp2.avg_fitness;
}

inline static bool
players_predicate(const AMLCharacter* player1, const AMLCharacter* player2)
{
    return player1->score > player2->score;
}

void
AMLPopulationActor::sort()
{
    for (auto& specie : species)
    {
        specie.sort();
    }
    species.Sort(specie_predicate);
    Algo::Sort(players, players_predicate);
}

void
AMLPopulationActor::set_best()
{
    AMLCharacter* best_player = players[0];
    cur_gen_best_score = best_player->score;

    if (cur_gen_best_score > overall_best_score)
    {
        overall_best_score = cur_gen_best_score;
        best_brain = best_player->network;
        generation_best_brains.Add(best_brain);
        staleness = 0;
        return;
    }

    staleness++;
}

void
AMLPopulationActor::create_child(int specie_index)
{
    StaticRandomNumberGenerator.seed();
    auto rand_chance = StaticRandomNumberGenerator.GetUniform(0, 1);
    // interspecies crossover
    if (rand_chance <= interspecies_crossover_chance && species.Num() > 1)
    {
        int other_specie_index = select_specie();
        while (specie_index == other_specie_index)
        {
            other_specie_index = select_specie();
        }
        crossover_and_add(species[specie_index].select_player(),
                          species[other_specie_index].select_player());
        generation_info.interspecies_crossover++;
    }
    // Just mutation
    else if (rand_chance < offspring_mutation_chance || species[specie_index].players.Num() < 2)
    {
        MLGenome child_genome = species[specie_index].select_player()->network;
        child_genome.mutate(innovation_history, mutation_constant);
        children_genomes.Add(child_genome);
        generation_info.mutation_count++;
    }
    // Crossover between the same species
    else
    {
        AMLCharacter* player1 = species[specie_index].select_player();
        AMLCharacter* player2 = species[specie_index].select_player();
        while (player1 == player2)
        {
            player2 = species[specie_index].select_player();
        }
        crossover_and_add(player1, player2);
        generation_info.crossover++;
    }
}

void
AMLPopulationActor::update_fitness_per_specie()
{
    for (auto& specie : species)
    {
        specie.explicit_fitness_sharing();
        specie.update_avg_fitness();
    }
}

void
AMLPopulationActor::remove_half()
{
    for (auto& specie : species)
    {
        specie.remove_bottom_half();
    }
}

struct is_stale
{
    bool operator()(const MLSpecie& sp) { return sp.staleness > 15; }
};

void
AMLPopulationActor::remove_stale_species()
{
    species.RemoveAll([](const MLSpecie& sp) { return sp.staleness > 15; });
    check(species.Num() != 0);
}

void
AMLPopulationActor::remove_useless_species()
{
    if (species.Num() == 1)
    {
        return;
    }
    float avg_fitness_sum = 0;
    int player_count = 0;
    for (auto& specie : species)
    {
        avg_fitness_sum += specie.avg_fitness;
    }
    player_count = players.Num();
    species.RemoveAll([avg_fitness_sum, player_count](MLSpecie& sp) {
        return (sp.avg_fitness / avg_fitness_sum * player_count) < 1;
    });
    check(species.Num() != 0);
}

void
AMLPopulationActor::calculate_all_player_fitness()
{
    for (auto& player : players)
    {
        player->calculate_score();
    }
}

void
AMLPopulationActor::classify_species()
{
    for (auto& specie : species)
    {
        specie.clear_players();
    }

    bool specie_found;
    for (auto& player : players)
    {
        specie_found = false;
        for (auto& specie : species)
        {
            if (specie.is_same_specie(player))
            {
                specie.add_player(player);
                specie_found = true;
                break;
            }
        }
        if (!specie_found)
        {
            MLSpecie new_species = MLSpecie(player);
            species.Add(new_species);
        }
    }
}

void
AMLPopulationActor::new_generation()
{
    if (population_size <= 0)
        return;
    // DEBUG
    int elite_count = 0;
    for (auto& player : players)
    {
        if (player->network.is_elite)
        {
            elite_count++;
        }
    }
    check(elite_count <= 1);

    AMLCharacter* previous_gen_elite = players[0];

    children_genomes.Empty();
    children_genomes.Reserve(players.Num());

    classify_species();
    calculate_all_player_fitness();
    sort();
    TArray<float> scores_sorted;
    scores_sorted.Reserve(players.Num());
    for (auto& player : players)
    {
        scores_sorted.Add(player->score);
    }
    AMLCharacter* current_best = players[0];
    remove_stale_species();
    remove_half();
    update_fitness_per_specie();
    remove_useless_species();
    update_mutation_constant();
    set_best();

    if (current_geneneration > 1)
    {
        check(previous_gen_elite->network.is_elite);
		check(FMath::Abs(previous_gen_elite->score - prev_best_score) <= elite_precision_epsilon);
    }

    curr_max_check_point = 0;
	for (auto& player : players)
    {
        if (curr_max_check_point < player->checkpoint_count)
        {
            curr_max_check_point = player->checkpoint_count;
        }
    }
    TArray<float> checkpoint_scores;
    TArray<float> sensor_penalties;
    checkpoint_scores.Reserve(population_size);
    for (auto& player : players)
    {
        checkpoint_scores.Add(player->check_point_score);
        sensor_penalties.Add(player->sensor_penalty);
	}
    UE_LOG(LogTemp, Warning, TEXT("#############################################"));

    // TODO_OGUZ: DEBUG BLOCK
    {
        int index = 0;
        for (auto& player : players)
        {
            /*UE_LOG(LogTemp, Warning, TEXT("Player: %d"), index++);
            for (auto& connection : player->network.connections)
            {
                UE_LOG(LogTemp,
                       Warning,
                       TEXT("%d %d %f"),
                       connection.from_node,
                       connection.to_node,
                       connection.weight);
            }*/
            if (player->network.is_elite)
            {
                UE_LOG(LogTemp, Warning, TEXT("Elite score: %f"), player->score);
                /* check(player->inputs.Num() == player->prev_inputs.Num());
                 check(player->outputs.Num() == player->prev_outputs.Num());
                 int total_count = player->inputs.Num();
                 if (player->inputs.Num() > player->prev_inputs.Num())
                 {
                     total_count = player->prev_inputs.Num();
                 }

                 for (int i = 0; i < total_count; i++)
                 {
                     check(player->inputs[i] == player->prev_inputs[i]);
                 }
                 total_count = player->outputs.Num();
                 if (player->outputs.Num() > player->prev_outputs.Num())
                 {
                     total_count = player->prev_outputs.Num();
                 }

                 for (int i = 0; i < total_count; i++)
                 {
                     check(player->outputs[i] == player->prev_outputs[i]);
                 }*/
                /*UE_LOG(LogTemp, Warning, TEXT("Elite connections:"));
                for (auto& node : player->network.nodes)
                {
                    for (auto& connection : node.output_connections)
                        UE_LOG(LogTemp,
                               Warning,
                               TEXT("%d %d %f"),
                               connection.from_node,
                               connection.to_node,
                               connection.weight);
                }*/
            }
            /*for (auto& connection : player->network.connections)
            {
                UE_LOG(LogTemp,
                       Warning,
                       TEXT("%d %d %f"),
                       connection.from_node,
                       connection.to_node,
                       connection.weight);
            }*/
        }

        UE_LOG(LogTemp,
               Warning,
               TEXT("Gen: %d, total mutations: %d, specie count: %d"),
               current_geneneration,
               innovation_history.Num(),
               species.Num());
        UE_LOG(LogTemp, Warning, TEXT("Max Score: %f"), overall_best_score);
        UE_LOG(LogTemp, Warning, TEXT("Current Gen Max Score: %f"), cur_gen_best_score);
        UE_LOG(LogTemp, Warning, TEXT("Checkpoint Score: %f, Sensor Penalty: %f, Distance Score: %f"), current_best->check_point_score,current_best->sensor_penalty,current_best->distance_traveled);
        UE_LOG(LogTemp, Warning, TEXT("Max checkpoint: %d"), curr_max_check_point);
        UE_LOG(LogTemp, Warning, TEXT("Lap Count: %d"), current_best->lap_count);
        UE_LOG(LogTemp,
               Warning,
               TEXT("Mutation Constant: %f Staleness: %d"),
               mutation_constant,
               staleness);
        float avg_fitness = 0;
        float avg_score = 0;
        for (auto& specie : species)
        {
            avg_fitness += specie.avg_fitness;
            avg_score += specie.avg_score;
        }
        avg_fitness /= species.Num();
        avg_score /= species.Num();
        UE_LOG(
          LogTemp, Warning, TEXT("Species Avg Fitness: %f  Avg Score: %f"), avg_fitness, avg_score);
    }
    // TODO_OGUZ: DEBUG BLOCK

    float avg_fitness_sum = 0;
    for (auto& specie : species)
    {
        avg_fitness_sum += specie.avg_fitness;
    }

    children_genomes.Add(current_best->network);
    prev_best_score = current_best->score;
    prev_best_distance_score = current_best->distance_traveled;
    prev_best_score = current_best->score;
    prev_best_checkpoint_score = current_best->check_point_score;
    // DEBUG
    // TArray<float> elite_prev_inputs = current_best->inputs;
    // TArray<float> elite_prev_outputs = current_best->outputs;

    /* UE_LOG(LogTemp, Warning, TEXT("Current best connections:"));
     for (auto& node : current_best->network.nodes)
     {
         for (auto& connection : node.output_connections)
             UE_LOG(LogTemp,
                    Warning,
                    TEXT("%d %d %f"),
                    connection.from_node,
                    connection.to_node,
                    connection.weight);
     }*/

    for (int i = 0; i < species.Num(); i++)
    {
        if (species[i].players[0] != current_best)
        {
            generation_info.copy_count++;
            children_genomes.Add(species[i].players[0]->network);
            // check(children_genomes.Last().connections.Num() >= 20)
        }

        int allowed_children_count = (species[i].avg_fitness * players.Num() / avg_fitness_sum) - 1;

        for (int j = 0; j < allowed_children_count; j++)
        {
            create_child(i);
            // check(children_genomes.Last().connections.Num() >= 20)
        }
    }

    if (children_genomes.Num() < players.Num())
    {
        children_genomes.Add(previous_gen_elite->network);
        generation_info.copy_count++;
    }

    for (auto& player : players)
    {
        // check(player->network.connections.Num() >= 20);
    }

    while (children_genomes.Num() < players.Num())
    {
        create_child(0);
    }

    AMLCharacter* it = NULL;
    for (int i = 0; i < players.Num(); i++)
    {
        it = players[i];
        it->network = children_genomes[i];
        it->network.reset_genome();
        it->reset_player(default_start_location, default_start_rotation);
        // check(it->network.connections.Num() >= 20)
    }
    players[0]->network.is_elite = true;
    players[0]->set_car_color(players[0]->car_color_elite);
    cur_gen_best_score = 0;
    current_gen_time = 0;
    current_geneneration++;
    generation_info = { 0 };

    // DEBUG
    // players[0]->prev_inputs = elite_prev_inputs;
    // players[0]->prev_outputs = elite_prev_outputs;
}

void
AMLPopulationActor::update_mutation_constant()
{
    if (staleness > staleness_constant)
    {
        if (mutation_constant < max_mutation_constant)
        {
            mutation_constant = mutation_constant + (staleness_constant * mutation_increase_rate);
        }
        else
        {
            mutation_constant = max_mutation_constant;
        }
    }
    else
    {
        mutation_constant = 1;
    }
}

void
AMLPopulationActor::kill_based_on_avg_speed(float total_speed, int dead_count)
{
    float avg_speed = total_speed / (players.Num() - dead_count);

    if (avg_speed < avg_speed_min_threshold && current_gen_time > 5.0f)
    {
        for (auto& player : players)
        {
            player->has_crashed = true;
        }
        dead_count = players.Num();
    }
}

int
AMLPopulationActor::select_specie()
{
    float avg_sum = 0;
    for (auto& specie : species)
    {
        avg_sum += specie.avg_fitness;
    }
    StaticRandomNumberGenerator.seed();
    float random_sum = StaticRandomNumberGenerator.GetUniform(0, avg_sum);
    float curr_sum = 0;
    for (int i = 0; i < species.Num(); i++)
    {
        curr_sum += species[i].avg_fitness;
        if (curr_sum > random_sum)
        {
            return i;
        }
    }
    return 0;
}
