// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputeRDGHelper.h"
#include "RenderGraphBuilder.h"
#include "LearnRDGShader.h"
#include "RHIGPUReadback.h"

// Sets default values
AComputeRDGHelper::AComputeRDGHelper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AComputeRDGHelper::BeginPlay()
{
	Super::BeginPlay();

	check(GEngine != nullptr);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("BeginPlay()!"));

	Readback = MakeShared<FRHIGPUBufferReadback>(TEXT("MyAddReadback"));

	AddTestComputePass(ScaleValue, TranslateValue);
}

// Called every frame
void AComputeRDGHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!Readback.IsValid())
        return;

    if (!Readback->IsReady())
        return;

    TSharedPtr<FRHIGPUBufferReadback, ESPMode::ThreadSafe> Local = Readback;
    Readback.Reset();

    ENQUEUE_RENDER_COMMAND(PollReadback)(
        [Local](FRHICommandListImmediate& RHICmdList)
        {
            const uint32 NumBytes = sizeof(float) * 2;
            const void* DataPtr = Local->Lock(NumBytes);
            const float* Values = static_cast<const float*>(DataPtr);

            const float A = Values[0];
            const float B = Values[1];

            Local->Unlock();

            AsyncTask(ENamedThreads::GameThread, [A, B]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Readback: %f %f"), A, B);
                });
        }
        );
}

void AComputeRDGHelper::AddTestComputePass(float Scale, float Translate) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestComputePass"));

	ENQUEUE_RENDER_COMMAND(AddComputePass)(
		[Scale, Translate, ReadbackPtr = Readback.Get()](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			LearnRDGShader::AddTestComputePass(GraphBuilder, Scale, Translate, ReadbackPtr);

			GraphBuilder.Execute();
		}
		);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestComputePassFinish"));
}

