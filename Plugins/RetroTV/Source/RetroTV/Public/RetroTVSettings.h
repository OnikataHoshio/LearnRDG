// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RetroTVSettings.generated.h"


class UTexture2D;

/**
 *
 */
UCLASS(Config = RetroTV, DefaultConfig, meta=(DisplayName="RetroTV"))
class RETROTV_API URetroTVSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Textures")
	TSoftObjectPtr<UTexture2D> TVMaskTexture;

	UPROPERTY(EditAnywhere, Config, Category = "Textures")
	TSoftObjectPtr<UTexture2D> ScanLineTexture;

	URetroTVSettings() = default;
};
