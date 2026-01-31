// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "RetroTVSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class RETROTV_API URetroTVSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TSharedPtr<class FRetroTVSceneViewExtension, ESPMode::ThreadSafe> CustomSceneViewExtension;

};

