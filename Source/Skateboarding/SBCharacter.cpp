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
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SkateboardStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkateboardMesh"));
	SkateboardStaticMesh->SetupAttachment(RootComponent);
	SkateboardStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASBCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	SkateMovementComponent = Cast<USBCharacterMovementComponent>(GetMovementComponent());	

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASBCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FVector Vector = (GetActorLocation() - LastPosition) / DeltaSeconds;

	FVector NormalizedVector = Vector.GetSafeNormal();
	// Find yaw.
	float Yaw = FMath::RadiansToDegrees(FMath::Atan2(NormalizedVector.Y, NormalizedVector.X));
	
	CameraBoom->SetRelativeRotation(FRotator(0.f,Yaw, 0.f));
	LastPosition = GetActorLocation();
	
	// FRotator NewRotation = FRotator(SkateMovementComponent->Velocity.X, 0.f, SkateMovementComponent->Velocity.Z);
	// GetCapsuleComponent()->SetRelativeRotation(NewRotation);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASBCharacter::Jump);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		
		EnhancedInputComponent->BindAction(ToggleSkate, ETriggerEvent::Completed, this, &ASBCharacter::ToggleMovementMode);

		// Lean
		EnhancedInputComponent->BindAction(LeanAction, ETriggerEvent::Triggered, this, &ASBCharacter::Lean);
		
		EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &ASBCharacter::Accelerate);
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Started, this, &ASBCharacter::BreakStarted);
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Completed, this, &ASBCharacter::BreakCompleted);

		// Looking
		// EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASBCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASBCharacter::ToggleMovementMode()
{
	if(GetCharacterMovement()->MovementMode == MOVE_Custom)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("Set walking"));
		GetCharacterMovement()->SetMovementMode(MOVE_Walking, MOVE_None);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("Set skating"));
		GetCharacterMovement()->SetMovementMode(MOVE_Custom, CMOVE_Skate);
	}
}

void ASBCharacter::Accelerate()
{
	if (Controller != nullptr)
	{
		//Add Skate Impulse
		if(SkateMovementComponent->MovementMode == MOVE_Custom
			&& SkateMovementComponent->CustomMovementMode == CMOVE_Skate
			&& SkateMovementComponent->GetIsGrounded() == true)
		{
			SkateMovementComponent->AddImpulse(FollowCamera->GetForwardVector() * ImpulseForce, true);
		}
	}
}

void ASBCharacter::BreakStarted()
{
	BreakFrictionScalar = 7.f;
	SkateMovementComponent->SetFrictionMultiplier(BreakFrictionScalar);
}

void ASBCharacter::BreakCompleted()
{
	SkateMovementComponent->SetFrictionMultiplier(1.f);
}

void ASBCharacter::Jump()
{
	//Super::Jump();
	if (Controller != nullptr)
	{
		if(SkateMovementComponent->MovementMode == MOVE_Custom
			&& SkateMovementComponent->CustomMovementMode == CMOVE_Skate
			&& SkateMovementComponent->GetIsGrounded() == true)
		{
			SkateMovementComponent->AddImpulse(GetCapsuleComponent()->GetUpVector() * ImpulseForce, true);
		}
	}
}

void ASBCharacter::Lean(const FInputActionValue& Value)
{

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// FRotator Rotator = FRotator(0.f, MovementVector.X * LeanRate, 0.f);			
		//GetCapsuleComponent()->AddRelativeRotation(Rotator);
	}
	SkateMovementComponent->AddForce(FollowCamera->GetRightVector() * (MovementVector.X * LeanRate ));
	// AddMovementInput(FollowCamera->GetRightVector(), MovementVector.X * LeanRate);	
	
	
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		//AddControllerYawInput(MovementVector.X * LeanRate);
		//AddControllerPitchInput(LookAxisVector.Y);
	}
	
	
	// input is a Vector2D
	// FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		FVector ForwardDirection;
		FVector RightDirection;
		if (GetCharacterMovement()->MovementMode == MOVE_Custom)
		{
			//Skate Controlling based on camera position
			ForwardDirection = FollowCamera->GetForwardVector();
			RightDirection = FollowCamera->GetRightVector();

		}
		else
		{
			//Biped Controlling
			// find out which way is forward based on controller rotation
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}

		// add movement 
		// AddMovementInput(ForwardDirection, MovementVector.Y);
		// AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASBCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if(SkateMovementComponent->MovementMode == MOVE_Walking)
		{
			// add yaw and pitch input to controller
			AddControllerYawInput(LookAxisVector.X);
			AddControllerPitchInput(LookAxisVector.Y);
		}
	}
}