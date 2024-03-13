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

UENUM(BlueprintType)
enum ECameraMode
{
	CameraMode_SkateFreeLook     UMETA(DisplayName = "SkateFreeLook"),
	CameraMode_SkateFixedForward UMETA(DisplayName = "SkateFixedForward"),
};

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

	/** Toggle between Skate/Wal movement modes Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleSkate;

	/** Toggle between fixed and movable camera mode while skateboarding - Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleSkateCameraMode;
	
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

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

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
	/** Min amount of time in seconds the player has to wait before accelerating again */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skate")
	float AccelerationDelay = 1.f; 
	
	/** Socket to attach the skateboard while walking */
	UPROPERTY(EditAnywhere, Category = "Socket")
	FName WalkingSkateboardSocket = "hand_r";

	UPROPERTY(EditAnywhere, Category = "Socket")
	USceneComponent* SkateboardSocket;
	
	/** Socket to attach the skateboard while skating */
	UPROPERTY(EditAnywhere, Category = "Socket")
	FName SkatingSkateboardSocket = "foot_r";
	
	FVector LastPosition;

	float LeanDirection;
	bool bIsAccelerating;

	int64 LastAccelerationTimeTicks;

	ECameraMode CurrentCameraMode = ECameraMode::CameraMode_SkateFreeLook;

	USBCharacterMovementComponent* SkateMovementComponent;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	virtual void Jump() override;

	USBCharacterMovementComponent* GetSkateMovementComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetLean();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsAccelerating();
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;

	/** Make the camera face forward direction if the camera mode desires it */
	void HandleCameraRotationWhileSkating(float DeltaSeconds);
	/** Toggles between free look and fixed camera mode while skateboarding */
	void ToggleCameraMode();
	/** Called for lean right and left on a skateboard */
	void Lean(const FInputActionValue& Value);
	/** Resets lean value for animations */
	void LeanCompleted();
	/** Toggle between skating and walking*/
	void ToggleMovementMode();
	/** Applies a forward force the the skate */
	void Accelerate();
	/** Resets accelerate flag for animations */
	void AccelerateCompleted();
	/** Increases the friction of the skate physics, causing it to break */
	void BreakStarted();
	/** Resets the friction of the skate physics, stop breaking */
	void BreakCompleted();

	/** Called for movement input while walking */
	void Move(const FInputActionValue& Value);
	/** Called for looking input while walking */
	void Look(const FInputActionValue& Value);
	
	void StartWalking();
	void StartSkating();
	void UpdateSkateCameraControlRotation();
};