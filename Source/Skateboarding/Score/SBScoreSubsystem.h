// Copyright 2024 Dankann Passos Weissmuller

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBScoreSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SKATEBOARDING_API USBScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void AddToScore(int32 Value);
	void RemoveFromScore(int32 Value);
	int32 GetScore();

	void RequestAddScore(ACharacter* Character, int32 ScoreAmount);
	
protected:
	int32 CurrentScore = 0;
};
