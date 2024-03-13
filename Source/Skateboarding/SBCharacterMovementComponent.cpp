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
	case CMOVE_Skate:
		{
			PhysSkate(DeltaTime, Iterations);
			break;
		}
	default: ;
	}
}

void USBCharacterMovementComponent::SetFrictionMultiplier(float Value)
{
	FrictionMultiplier = Value;
}

bool USBCharacterMovementComponent::GetIsGrounded()
{
	return bIsGrounded;
}

void USBCharacterMovementComponent::PhysSkate(float DeltaTime, int32 Iterations)
{
	if(DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	FHitResult Hit;
	//Hit a ground surface
	if (GetSurface(Hit) == true)
	{
		bIsGrounded = true;
		/// On flat surfaces, DotProduct will be 1.f
		///    	               \                    |                  /           
		///   __⬆__  Dot: 1     \ ↗  Dot: 0.7       |➡  Dot: 0       /↘  Dot: -0.7 
		///	                     \                  |                /						
		const float UpDotProduct = Hit.Normal.Dot(FVector::UpVector);
		const float RightDotProduct = Hit.Normal.Dot(FVector::RightVector);
		const float DownDotProduct = Hit.Normal.Dot(FVector::DownVector);

		DrawDebugString(GetWorld(), Hit.Location, FString::Printf(TEXT("Dot(Normal, UpVector): %f"), UpDotProduct), nullptr, FColor::Emerald, DeltaTime, true);
		DrawDebugDirectionalArrow(GetWorld(), Hit.Location, Hit.Location + Hit.Normal * 50.f, 100.f, FColor::Green, false, 2.f);
		
		// Calculate the parallel acceleration using the magnitude of the parallel gravity force
		float ScaledMassRelativeToSlope = UpDotProduct / Mass;
		Velocity += Hit.Normal * GroundGravity * SlopeGravityScale * ScaledMassRelativeToSlope * DeltaTime;
		
		//Apply gravity 
		Velocity += GroundGravity * FVector::DownVector * DeltaTime;
		
		// if(FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector()))> .5f)
		// {
		// 	Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
		// }
		// else
		// {
		// 	Acceleration = FVector::ZeroVector;
		// } 
		
	}// Mid air
	else
	{
		bIsGrounded = false;
		//Turning not possible mid air
		Acceleration = FVector::ZeroVector;
	
		// Apply skate air gravity 
		Velocity += AirGravity * FVector::DownVector * DeltaTime;
	}

	//Calculating Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, Friction * FrictionMultiplier, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	Iterations++;
	bJustTeleported = false;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FVector Adjusted = Velocity * DeltaTime;
	
	//Rotate player to slope if grounded
	// FQuat NewRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	// FVector VelocityPlaneDirection = FVector::VectorPlaneProject(Velocity, Hit.Normal).GetSafeNormal();
	// FQuat NewRotation = FRotationMatrix::MakeFromZX(VelocityPlaneDirection, Hit.Normal).ToQuat();

	FRotator GroundAlignment = FRotationMatrix::MakeFromZX(Hit.Normal, UpdatedComponent->GetForwardVector()).Rotator();
	FRotator DirectionOfMovement = FRotator(GroundAlignment.Euler().X, GroundAlignment.Euler().X, Velocity.ToOrientationRotator().Yaw);
	GroundAlignment.Yaw = Velocity.ToOrientationRotator().Yaw;
	FQuat NewRotation = FMath::RInterpTo(UpdatedComponent->GetComponentRotation(), GroundAlignment, DeltaTime, 15.f).Quaternion();

	FHitResult InTime(1.f);
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, InTime);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - InTime.Time), InTime.Normal, InTime, true);
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
		GetActorUpVector()) * 1.5f;
	
	DrawDebugDirectionalArrow(GetWorld(), Start, End, 100.f, FColor::Red, false, 2.f);
	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility);
}
