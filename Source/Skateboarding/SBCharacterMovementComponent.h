// Copyright 2024 Dankann Passos Weissmuller

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SBCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_Skate UMETA(DisplayName = "Skate"),
};

/**
 * Custom character movement component for skateboarding
 */
UCLASS()
class SKATEBOARDING_API USBCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void InitializeComponent() override;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void SetFrictionMultiplier(float Value);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsGrounded();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsSkateInAir();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float Friction = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float GroundGravity = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float AirGravity = 4000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SlopeGravityScale = 10.f;

	float FrictionMultiplier = 1.f;

	bool bIsGrounded = true;
	
private:	
	void EnterSkate();
	void ExitSkate();
	void PhysSkate(float DeltaTime, int32 Iterations);
	bool GetSurface(FHitResult& Hit) const;
};
