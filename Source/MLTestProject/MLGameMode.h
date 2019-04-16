// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine.h"
#include "MLGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MLTESTPROJECT_API AMLGameMode : public AGameModeBase
{
	GENERATED_BODY()
	virtual void StartPlay() override;
};
