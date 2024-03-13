// Copyright 2024 Dankann Passos Weissmuller


#include "SBScoreSubsystem.h"

#include "Skateboarding/SBCharacter.h"
#include "Skateboarding/SBCharacterMovementComponent.h"

void USBScoreSubsystem::AddToScore(int32 Value)
{
	CurrentScore += Value;
}

void USBScoreSubsystem::RemoveFromScore(int32 Value)
{
	CurrentScore -= Value;
}

int32 USBScoreSubsystem::GetScore()
{
	return CurrentScore;
}

bool USBScoreSubsystem::RequestAddScore(ACharacter* Character, int32 ScoreAmount)
{
	USBCharacterMovementComponent* SkateMovementComponent = Cast<USBCharacterMovementComponent>(Character->GetMovementComponent());
	if(SkateMovementComponent->GetIsSkateInAir() == true)
	{
		AddToScore(ScoreAmount);
		OnScoreAdded.Broadcast(ScoreAmount, GetScore());
		return true;
	}

	return false;
}
