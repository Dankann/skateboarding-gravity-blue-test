// Copyright 2024 Dankann Passos Weissmuller

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBScoreSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreAdded, int32, ScoreAdded, int32, TotalScore);

/**
 * 
 */
UCLASS()
class SKATEBOARDING_API USBScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AddToScore(int32 Value);
	
	UFUNCTION(BlueprintCallable)
	void RemoveFromScore(int32 Value);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetScore();

	bool RequestAddScore(ACharacter* Character, int32 ScoreAmount);

	// UFUNCTION(BlueprintImplementableEvent)
	// void OnScoreAdded;
	UPROPERTY(BlueprintAssignable)
	FOnScoreAdded OnScoreAdded;
	
protected:
	int32 CurrentScore = 0;
};
