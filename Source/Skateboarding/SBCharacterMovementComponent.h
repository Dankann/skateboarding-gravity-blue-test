// Copyright 2024 Dankann Passos Weissmuller

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SBCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	MOVE_Skate UMETA(DisplayName = "Skate"),
};

/**
 * 
 */
UCLASS()
class SKATEBOARDING_API USBCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void InitializeComponent() override;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SkateFriction = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SkateGroundGravity = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SkateAirGravity = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SkateSlopeGravityScale = 10.f;
	
	void EnterSkate();
	void ExitSkate();
	bool IsGrounded();
	void PhysSkate(float DeltaTime, int32 Iterations);
	bool GetSurface(FHitResult& Hit) const;
};
