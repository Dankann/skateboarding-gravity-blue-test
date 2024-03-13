// Copyright 2024 Dankann Passos Weissmuller


#include "SBScoreObstacle.h"

#include "SBScoreSubsystem.h"
#include "GameFramework/Character.h"

// Sets default values
ASBScoreObstacle::ASBScoreObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASBScoreObstacle::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &ASBScoreObstacle::OnOverlapBegin);
}

void ASBScoreObstacle::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor == nullptr)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character == nullptr)
	{
		return;
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	USBScoreSubsystem* Subsystem = UGameInstance::GetSubsystem<USBScoreSubsystem>(GetWorld()->GetGameInstance());
	if (Subsystem == nullptr)
	{
		return;
	}

	if(Subsystem->RequestAddScore(Character, ScoreAmount))
	{
		OnScoreAdded();
	}

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, *FString::Printf(TEXT("Score: %d"), Subsystem->GetScore()));
}

