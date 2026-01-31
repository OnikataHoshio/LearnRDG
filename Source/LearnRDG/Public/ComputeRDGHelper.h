// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ComputeRDGHelper.generated.h"

class FRHIGPUBufferReadback;

UCLASS()
class LEARNRDG_API AComputeRDGHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AComputeRDGHelper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestShader")
	float ScaleValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestShader")
	float TranslateValue = 2.0f;

	TSharedPtr<FRHIGPUBufferReadback, ESPMode::ThreadSafe> Readback;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddTestComputePass(float Scale, float Translate) const;
};
