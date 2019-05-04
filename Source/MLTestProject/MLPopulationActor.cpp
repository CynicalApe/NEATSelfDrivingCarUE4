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
        it->network.mutate(innovation_history, mutation_constant);
    }
}

// Called every frame
void
AMLPopulationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
	int dead_count = 0;
    for (auto& player : players)
    {
        if (player->has_crashed)
        {
            dead_count++;
			continue;
		}
        player->update(DeltaTime);
    }
    if (dead_count == players.Num())
    {
		new_generation();
	}
}

void
AMLPopulationActor::update(float DeltaTime)
{
}

void
AMLPopulationActor::spawn_new_batch()
{
    FActorSpawnParameters* param = new FActorSpawnParameters();
    param->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int i = 0; i < population_size; i++)
    {
        AMLCharacter* spawned_actor =
          GetWorld()->SpawnActor<AMLCharacter>(AMLCharacter::StaticClass(),
                                               default_start_point->GetActorLocation(),
                                               default_start_point->GetActorRotation(),
                                               *param);
        assert(!spawned_actor->network.nodes.Empty());
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
        offspring.mutate(innovation_history, mutation_constant);
        children_genomes.Add(offspring);
    }
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
    float score = best_player->score;

    if (score > overall_best_score)
    {
        overall_best_score = score;
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
            species.Add(MLSpecie(player));
        }
    }
}

void
AMLPopulationActor::new_generation()
{
    AMLCharacter* previous_gen_elite = players[0];
    children_genomes.Empty();
    children_genomes.Reserve(players.Num());

    classify_species();
    calculate_all_player_fitness();
    sort();
    AMLCharacter* current_best = players[0];
    remove_stale_species();
    remove_half();
	update_fitness_per_specie();
    remove_useless_species();
    update_mutation_constant();
    set_best();
    curr_max_check_point = 0;


    for (auto& player : players)
    {
        if (curr_max_check_point < player->checkpoint_count)
            curr_max_check_point = player->checkpoint_count;
		//TODO_OGUZ: DEBUG ONLY
        if (player->is_elite) 
        {
            UE_LOG(LogTemp, Warning, TEXT("Elite score: %f"), player->score);
		}
    }

	// TODO_OGUZ: DEBUG BLOCK
    {
        UE_LOG(LogTemp,
               Warning,
               TEXT("Gen: %d, total mutations: %d, specie count: %d"),
               current_geneneration,
               innovation_history.Num(),
               species.Num());

        UE_LOG(LogTemp, Warning, TEXT("Max checkpoint: %d"), curr_max_check_point);
        UE_LOG(LogTemp, Warning, TEXT("Mutation Constant: %f Staleness: %f"), mutation_constant, staleness);
		float avg_fitness = 0;
		float avg_score = 0;
		for (auto& specie : species)
        {
			avg_fitness += specie.avg_fitness;
			avg_score += specie.avg_score;
		}
        avg_fitness /= species.Num();
        avg_score /= species.Num();
		UE_LOG(LogTemp, Warning, TEXT("Species Avg Fitness: %f  Avg Score: %f"), avg_fitness, avg_score);

    }
	// TODO_OGUZ: DEBUG BLOCK

    int avg_fitness_sum = 0;
    for (auto& specie : species)
    {
        avg_fitness_sum += specie.avg_fitness;
    }

	children_genomes.Add(current_best->network);
	
    for (int i = 0; i < species.Num(); i++)
    {
        if (species[i].players[0] != current_best)
        {
            generation_info.copy_count++;
            children_genomes.Add(species[i].players[0]->network);
		}

		int allowed_children_count = (species[i].avg_fitness * players.Num() / avg_fitness_sum) - 1;

        for (int j = 0; j < allowed_children_count; j++)
        {
            create_child(i);
        }
    }

    if (children_genomes.Num() < players.Num())
    {
        children_genomes.Add(previous_gen_elite->network);
		generation_info.copy_count++;
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
    generation_info =  {0};
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

