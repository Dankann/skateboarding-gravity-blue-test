// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateboardingGameMode.h"
#include "SkateboardingCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkateboardingGameMode::ASkateboardingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
