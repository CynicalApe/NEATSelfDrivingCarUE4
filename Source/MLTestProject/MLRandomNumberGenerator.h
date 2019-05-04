// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <random>
#include <ctime>
#include "CoreMinimal.h"

/**
 *
 */

class MLTESTPROJECT_API MLRandomNumberGenerator
{ 
	std::random_device rd;
    std::mt19937 MLRandomEngine;
    std::uniform_real_distribution<float> MLUniformDistribution;
    std::normal_distribution<float> MLNormalDistribution;

  public:
    MLRandomNumberGenerator();
    ~MLRandomNumberGenerator();
    void seed() { MLRandomEngine.seed(rd()); }
    float GetUniform() { return MLUniformDistribution(MLRandomEngine); }
    float GetNormal() { return MLNormalDistribution(MLRandomEngine); }
    float GetUniform(float min, float max)
    {
        MLUniformDistribution = std::uniform_real_distribution<float>(min, max);
        return MLUniformDistribution(MLRandomEngine);
    }

    float GetNormal(float min, float max)
    {
        MLUniformDistribution = std::uniform_real_distribution<float>(min, max);
        return MLUniformDistribution(MLRandomEngine);
    }
};

static MLRandomNumberGenerator StaticRandomNumberGenerator;
