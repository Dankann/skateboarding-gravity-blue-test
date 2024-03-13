// Copyright Epic Games, Inc. All Rights Reserved.

#include "SBCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SBCharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASkateboardingCharacter

ASBCharacter::ASBCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SkateboardStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateboardMesh"));
	SkateboardStaticMesh->SetupAttachment(RootComponent);
	SkateboardStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkateboardSocket = CreateDefaultSubobject<USceneComponent>(TEXT("SkateboardSocket"));
	SkateboardSocket->SetupAttachment(RootComponent);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASBCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	StartSkating();
}

void ASBCharacter::HandleCameraRotationWhileSkating(float DeltaSeconds)
{
	if (GetSkateMovementComponent()->MovementMode == MOVE_Walking)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("MOVE_Walking - dont move camera"));
		return;
	}

	if(CurrentCameraMode == ECameraMode::CameraMode_SkateFreeLook)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("CameraMode_SkateFreeLook - dont move camera"));
		return;
	}

	FVector Vector = (GetActorLocation() - LastPosition) / DeltaSeconds;
	FVector NormalizedVector = Vector.GetSafeNormal();
	float Yaw = FMath::RadiansToDegrees(FMath::Atan2(NormalizedVector.Y, NormalizedVector.X));

	CameraBoom->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
	LastPosition = GetActorLocation();
}


void ASBCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleCameraRotationWhileSkating(DeltaSeconds);
}

USBCharacterMovementComponent* ASBCharacter::GetSkateMovementComponent()
{
	if (SkateMovementComponent == nullptr)
	{
		SkateMovementComponent = Cast<USBCharacterMovementComponent>(GetMovementComponent());
	}

	return SkateMovementComponent;
}

float ASBCharacter::GetLean()
{
	return LeanDirection;
}

bool ASBCharacter::GetIsAccelerating()
{
	return bIsAccelerating;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		///Skating and Walking///
		EnhancedInputComponent->BindAction(ToggleSkateCameraMode, ETriggerEvent::Completed, this, &ASBCharacter::ToggleCameraMode);
		EnhancedInputComponent->BindAction(ToggleSkate, ETriggerEvent::Completed, this, &ASBCharacter::ToggleMovementMode);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASBCharacter::Jump);


		///Skating inputs///
		EnhancedInputComponent->BindAction(LeanAction, ETriggerEvent::Triggered, this, &ASBCharacter::Lean);
		EnhancedInputComponent->BindAction(LeanAction, ETriggerEvent::Completed, this, &ASBCharacter::LeanCompleted);
		EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &ASBCharacter::Accelerate);
		EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &ASBCharacter::AccelerateCompleted);
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Started, this, &ASBCharacter::BreakStarted);
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Completed, this, &ASBCharacter::BreakCompleted);

		///Walking inputs///
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASBCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASBCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void ASBCharacter::ToggleCameraMode()
{
	if(CurrentCameraMode == ECameraMode::CameraMode_SkateFreeLook)
	{
		CurrentCameraMode = ECameraMode::CameraMode_SkateFixedForward;
	}
	else
	{
		CurrentCameraMode = ECameraMode::CameraMode_SkateFreeLook;
	}
	UpdateSkateCameraControlRotation();
}

void ASBCharacter::ToggleMovementMode()
{
	if (GetCharacterMovement()->MovementMode == MOVE_Custom)
	{
		StartWalking();
	}
	else
	{
		StartSkating();
	}
}

void ASBCharacter::Jump()
{
	if (Controller != nullptr)
	{
		if (GetSkateMovementComponent()->MovementMode == MOVE_Custom
			&& GetSkateMovementComponent()->CustomMovementMode == CMOVE_Skate
			&& GetSkateMovementComponent()->GetIsGrounded() == true)
		{
			GetSkateMovementComponent()->AddImpulse(GetCapsuleComponent()->GetUpVector() * ImpulseForce, true);
		}
		else
		{
			Super::Jump();
		}
	}
}

void ASBCharacter::Accelerate()
{
	if (Controller == nullptr)
	{
		return;
	}

	if (GetSkateMovementComponent()->MovementMode != MOVE_Custom || GetSkateMovementComponent()->CustomMovementMode !=
		CMOVE_Skate || GetSkateMovementComponent()->GetIsGrounded() == false)
	{
		return;
	}

	const int64 CurrentTicks = FTimespan::FromSeconds(GetWorld()->GetTimeSeconds()).GetTicks();
	if (FDateTime(CurrentTicks - LastAccelerationTimeTicks).GetSecond() < AccelerationDelay)
	{
		return;
	}
	//@todo Test with add movement input later
	GetSkateMovementComponent()->AddImpulse(GetCapsuleComponent()->GetForwardVector() * ImpulseForce, true);
	LastAccelerationTimeTicks = FTimespan::FromSeconds(GetWorld()->GetTimeSeconds()).GetTicks();
	bIsAccelerating = true;
}

void ASBCharacter::AccelerateCompleted()
{
	bIsAccelerating = false;
}

void ASBCharacter::BreakStarted()
{
	BreakFrictionScalar = 7.f;
	GetSkateMovementComponent()->SetFrictionMultiplier(BreakFrictionScalar);
}

void ASBCharacter::BreakCompleted()
{
	GetSkateMovementComponent()->SetFrictionMultiplier(1.f);
}

void ASBCharacter::Lean(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		//Add Skate Force
		if (GetSkateMovementComponent()->MovementMode == MOVE_Custom
			&& GetSkateMovementComponent()->CustomMovementMode == CMOVE_Skate)
		{
			// input is a float
			LeanDirection = Value.Get<float>();
			GetSkateMovementComponent()->AddForce(GetCapsuleComponent()->GetRightVector() * (LeanDirection * LeanRate));
		}
	}
}

void ASBCharacter::LeanCompleted()
{
	LeanDirection = 0.f;
}

void ASBCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		FVector ForwardDirection;
		FVector RightDirection;
		if (GetCharacterMovement()->MovementMode == MOVE_Custom)
		{
			return;
		}

		// Only move while walking
		// find out which way is forward based on controller rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASBCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (GetSkateMovementComponent()->MovementMode == MOVE_Custom && CurrentCameraMode == ECameraMode::CameraMode_SkateFixedForward)
		{
			return;
		}
		
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASBCharacter::StartWalking()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("Set walking"));
	GetCharacterMovement()->SetMovementMode(MOVE_Walking, MOVE_None);
	SkateboardStaticMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WalkingSkateboardSocket);
	
	UpdateSkateCameraControlRotation();
	SkateboardStaticMesh->SetRelativeRotation(FRotator::ZeroRotator);
}

void ASBCharacter::StartSkating()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("Set skating"));
	GetCharacterMovement()->SetMovementMode(MOVE_Custom, CMOVE_Skate);
	SkateboardStaticMesh->AttachToComponent(SkateboardSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	UpdateSkateCameraControlRotation();
	// SkateboardStaticMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
}

void ASBCharacter::UpdateSkateCameraControlRotation()
{
	if (GetSkateMovementComponent()->MovementMode == MOVE_Custom)
	{
		bool bShouldUsePawnControlRotation = CurrentCameraMode == ECameraMode::CameraMode_SkateFreeLook;
		CameraBoom->bInheritPitch = bShouldUsePawnControlRotation;
		CameraBoom->bInheritRoll = bShouldUsePawnControlRotation;
		CameraBoom->bInheritYaw = bShouldUsePawnControlRotation;
		CameraBoom->bUsePawnControlRotation = bShouldUsePawnControlRotation;
	}
	else
	{
		CameraBoom->bInheritPitch = true;
		CameraBoom->bInheritRoll = true;
		CameraBoom->bInheritYaw = true;
		CameraBoom->bUsePawnControlRotation = true;
	}	
}