// Copyright 2024 Dankann Passos Weissmuller

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "SBScoreObstacle.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SKATEBOARDING_API ASBScoreObstacle : public ATriggerVolume
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASBScoreObstacle();

	UPROPERTY(EditAnywhere, Category = "Score")
	int32 ScoreAmount = 10;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);
};
