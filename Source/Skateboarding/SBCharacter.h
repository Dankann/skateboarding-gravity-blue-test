// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SBCharacter.generated.h"

class USBCharacterMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ASBCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASBCharacter();
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Accelerate Input Action */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AccelerateAction;
	
	/** Break Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BreakAction;

	/** Lean Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LeanAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	/** Toggle between Skate/Wal movement modes Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleSkate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skate")
	UStaticMeshComponent* SkateboardStaticMesh;
	
protected:
	/** Force to add to the skateboard forward direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skate")
	float ImpulseForce = 1000.f;
	/** Multiply friction to this value when breaking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skate")
	float BreakFrictionScalar = 7.f;
	/** Lean rate for rotating the skateboard */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skate")
	double LeanRate = 100.f;

	FVector LastPosition;

	USBCharacterMovementComponent* SkateMovementComponent;
	

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	virtual void Jump() override;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;
	
	/** Called for movement input */
	void Lean(const FInputActionValue& Value);
	/** Called for looking input */
	void Look(const FInputActionValue& Value);	
	void ToggleMovementMode();
	void Accelerate();
	void BreakStarted();
	void BreakCompleted();	
};