// Copyright 2024 Dankann Passos Weissmuller


#include "SBCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void USBCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void USBCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case MOVE_Skate:
		{
			PhysSkate(DeltaTime, Iterations);
			break;
		}
	default: ;
	}
}

bool USBCharacterMovementComponent::IsGrounded()
{
	return true;
}

void USBCharacterMovementComponent::PhysSkate(float DeltaTime, int32 Iterations)
{
	FHitResult Hit;
	//Hit a ground surface
	if (GetSurface(Hit) == true)
	{
		/// On flat surfaces, DotProduct will be 1.f
		///    	               \                    |                  /           
		///   __⬆__  Dot: 1     \ ↗  Dot: 0.7       |➡  Dot: 0       /↘  Dot: -0.7 
		///	                     \                  |                /						
		const float UpDotProduct = Hit.Normal.Dot(FVector::UpVector);
		const float RightDotProduct = Hit.Normal.Dot(FVector::RightVector);
		const float DownDotProduct = Hit.Normal.Dot(FVector::DownVector);
		DrawDebugString(GetWorld(), Hit.Location, FString::Printf(TEXT("Dot(Normal, Right): %FMath::Abs(RightDotProduct)"), UpDotProduct), nullptr,
		                FColor::Emerald, DeltaTime, true);

		// Calculate the parallel acceleration using the magnitude of the parallel gravity force
		float ScaledMassRelativeToSlope = UpDotProduct / Mass;
		Velocity += Hit.Normal * SkateGroundGravity * SkateSlopeGravityScale * ScaledMassRelativeToSlope * DeltaTime;
		//Apply gravity 
		Velocity += SkateGroundGravity * FVector::DownVector * DeltaTime;
		
	}// Mid air
	else
	{
		//Turning not possible mid air
		Acceleration = FVector::ZeroVector;
	}
	
	//Either way apply gravity 
	Velocity += SkateAirGravity * FVector::DownVector * DeltaTime;

	//Calculating Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, SkateFriction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	//Perform Actual Movement!!
	Iterations++;
	bJustTeleported = false;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FVector Adjusted = Velocity * DeltaTime;
	
	//Rotate player to slope if grounded
	FQuat NewRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	if (GetSurface(Hit))
	{
		FRotator GroundAlignment = FRotationMatrix::MakeFromZX(Hit.Normal, UpdatedComponent->GetForwardVector()).Rotator();
		NewRotation = FMath::RInterpTo(UpdatedComponent->GetComponentRotation(), GroundAlignment, DeltaTime, .05f).Quaternion();
	}

	//THIS ACTUALLY DOES THE MOVE
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}
}

bool USBCharacterMovementComponent::GetSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	// FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.5f * (-1 * CharacterOwner->GetActorUpVector());//FVector::DownVector;
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * (-1 * CharacterOwner->
		GetActorUpVector()) * 2.f;

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility);
}
