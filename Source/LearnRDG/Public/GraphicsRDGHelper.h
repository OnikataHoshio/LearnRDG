// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GraphicsRDGHelper.generated.h"

UCLASS()
class LEARNRDG_API AGraphicsRDGHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGraphicsRDGHelper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestShader")
	TObjectPtr<UTextureRenderTarget2D> GraphicsRDGRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestShader")
	TObjectPtr<UTextureRenderTarget2D> FullScreenRDGRenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TestShader")
	float BValue = 1.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddTestGraphicsPass(UTextureRenderTarget2D* RenderTarget2D, float BChannelValue) const;

	void AddTestFullscreenPass(UTextureRenderTarget2D* RenderTarget2D, float BChannelValue) const;

};
