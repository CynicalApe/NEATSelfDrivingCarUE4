// Fill out your copyright notice in the Description page of Project Settings.
#include "MLPopulationActor.h"
#include <cassert>

// Sets default values
AMLPopulationActor::AMLPopulationActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if
    // you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    start_point_tag = TEXT("PlayerStartTag");
    children_genomes.Reserve(population_size);
}

// Called when the game starts or when spawned
void
AMLPopulationActor::BeginPlay()
{
    Super::BeginPlay();

    UGameplayStatics::GetAllActorsWithTag(GetWorld(), start_point_tag, start_point_arr);
    if (start_point_arr.Num() >= 1)
    {
        default_start_point = start_point_arr[0];
    }
    spawn_new_batch();
    for (auto& it : players)
    {
        it->network.init_connect(innovation_history);
        it->network.mutate(innovation_history);
    }
}

// Called every frame
void
AMLPopulationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    /*for (auto& player : players)
    {
        player->TickSensors();
    }*/
}

void
AMLPopulationActor::spawn_new_batch()
{
    FActorSpawnParameters* param = new FActorSpawnParameters();

    for (int i = 0; i < population_size; i++)
    {
        param->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AMLCharacter* spawned_actor =
          GetWorld()->SpawnActor<AMLCharacter>(AMLCharacter::StaticClass(),
                                               default_start_point->GetActorLocation(),
                                               default_start_point->GetActorRotation(),
                                               *param);
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
        if (genome1.has_matching_innovation(genome1.connections[i].innovation_number,
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
        offspring.create_nodes_output_connection();
        offspring.mutate(innovation_history);
        children_genomes.Add(offspring);
    }
}

inline static bool
specie_predicate(const MLSpecie& sp1, const MLSpecie& sp2)
{
    return sp1.avg_fitness > sp2.avg_fitness;
}

void
AMLPopulationActor::sort()
{
    for (auto& specie : species)
    {
        specie.sort();
    }
    species.Sort(specie_predicate);
}

struct is_stale
{
    bool operator()(const MLSpecie& sp) { return sp.staleness > 15; }
};

void
AMLPopulationActor::remove_stale_species()
{
    species.RemoveAll([](const MLSpecie& sp) { return sp.staleness > 15; });
}

void
AMLPopulationActor::set_best()
{
    for (auto& it : players)
    {
        if (it->fitness > overall_best_score)
        {
            generation_best_brains.Add(it->network);
            overall_best_score = it->fitness;
            best_brain = it->network;
        }
    }
}

void
AMLPopulationActor::create_child(int specie_index)
{
    StaticRandomNumberGenerator.seed();
    auto rand_chance = StaticRandomNumberGenerator.GetUniform(0, 1);
    if (rand_chance <= 0.001 && species.Num() > 1)
    {
        int other_specie_index = select_specie();
        while (specie_index == other_specie_index)
        {
            other_specie_index = select_specie();
        }
        crossover_and_add(species[specie_index].select_player(),
                          species[other_specie_index].select_player());
    }
    else if (rand_chance < 0.25 || species[specie_index].players.Num() < 2)
    {
        MLGenome child_genome = species[specie_index].select_player()->network;
        child_genome.mutate(innovation_history);
        children_genomes.Add(child_genome);
    }
    else
    {
        AMLCharacter* player1 = species[specie_index].select_player();
        AMLCharacter* player2 = species[specie_index].select_player();
        while (player1 == player2)
        {
            player2 = species[specie_index].select_player();
        }
        crossover_and_add(player1, player2);
    }
}

void
AMLPopulationActor::remove_half()
{
    for (auto& specie : species)
    {
        specie.remove_bottom_half();
        specie.explicit_fitness_sharing();
        specie.update_avg_fitness();
    }
}

void
AMLPopulationActor::remove_useless_species()
{
    int avg_fitness_sum = 0;
    int player_count = 0;
    for (auto& specie : species)
    {
        avg_fitness_sum += specie.avg_fitness;
    }
    player_count = players.Num();
    species.RemoveAll([avg_fitness_sum, player_count](MLSpecie& sp) {
        return (sp.avg_fitness / avg_fitness_sum * player_count) < 1;
    });
}

void
AMLPopulationActor::calculate_fitness()
{
    for (auto& player : players)
    {
        player->calculate_fitness();
    }
}

void
AMLPopulationActor::classify_species()
{
    species.Empty();
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
            species.Add(MLSpecie(player));
        }
    }
}

void
AMLPopulationActor::new_generation()
{
    children_genomes.Empty();
    children_genomes.Reserve(players.Num());
    AMLCharacter* previous_gen_elite = players[0];
    classify_species();
    calculate_fitness();
    sort();
    remove_half();
    set_best();
    remove_stale_species();
    remove_useless_species();
    curr_max_check_point = 0;
    for (auto& it : players)
    {
        if (curr_max_check_point < it->checkpoint_count)
            curr_max_check_point = it->checkpoint_count;
    }

    int avg_fitness_sum = 0;
    for (auto& specie : species)
    {
        avg_fitness_sum += specie.avg_fitness;
    }

    for (int i = 0; i < species.Num(); i++)
    {
        children_genomes.Add(species[i].players[0]->network);
        int allowed_children_count = (species[i].avg_fitness * players.Num() / avg_fitness_sum) - 1;

        for (int j = 0; j < allowed_children_count; j++)
        {
            create_child(i);
        }
    }

    if (children_genomes.Num() < players.Num())
    {
        children_genomes.Add(previous_gen_elite->network);
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
        it->reset_player();
    }
    cur_gen_best_score = 0;
    current_geneneration++;
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
