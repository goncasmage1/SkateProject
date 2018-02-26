// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SkateProjectCharacter.h"
#include "SkateController.h"
#include "Curves/CurveVector.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASkateProjectCharacter

ASkateProjectCharacter::ASkateProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SM_Skateboard = CreateDefaultSubobject<USkeletalMeshComponent>(("Skateboard"));
	SM_Skateboard->SetupAttachment(RootComponent);

	MinimumRequiredDot = 0.6f;
	TrickPointExecDistance = 0.1f;
	Deadzone = 0.05f;
	TimeCounter = 0.f;
	TrickTime = 0.f;
	LastTrickLocation = FVector2D(0.f, 0.f);
	PreviousLocation = FVector2D(0.1f, 0.f);

	bIsDragging = false;
	bInsideSafezone = false;
	bExecutingTrick = false;
}

void ASkateProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	StartPosition = GetMesh()->GetRelativeTransform().GetLocation();
	StartRotation = GetMesh()->GetComponentRotation();
}

void ASkateProjectCharacter::CustomBeginPlay()
{
	SkatePC = Cast<ASkateController>(GetController());

	check(SkatePC != nullptr && "NO PC FOUND")

	//Calculate Joystick Position for each trickpoint
	for (int i = 0; i < Tricks.Num(); i++)
	{
		for (int j = 0; j < Tricks[i].Points.Num(); j++)
		{
			Tricks[i].Points[j].DesiredPosition = FVector2D(FMath::Cos(FMath::DegreesToRadians(Tricks[i].Points[j].Angle)), FMath::Sin(FMath::DegreesToRadians(Tricks[i].Points[j].Angle)));
		}
		Tricks[i].TrickIndex = i;
	}
	TrickQueue.Empty();
	TrickQueue.Append(Tricks);
}

void ASkateProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bExecutingTrick)
	{
		HandleTrickExecution(DeltaTime);
	}
	else
	{
		HandleAnalogInput(DeltaTime);
	}
}

void ASkateProjectCharacter::HandleTrickExecution(float DeltaTime)
{
	GetMesh()->SetRelativeLocation(StartPosition + TricksLocationCurves[EligibleTrick.TrickIndex]->GetVectorValue(TimeCounter));
	FVector NewRotation = TricksRotationCurves[EligibleTrick.TrickIndex]->GetVectorValue(TimeCounter);
	FRotator Rot1, Rot2, Rot3, Result;
	Rot1 = FRotator(NewRotation.Z, 0.f, 0.f);
	Rot2 = FRotator(0.f, NewRotation.Y, 0.f);
	Rot3 = FRotator(0.f, 0.f, NewRotation.X);
	Result = UKismetMathLibrary::ComposeRotators(UKismetMathLibrary::ComposeRotators(Rot1, Rot2), Rot3);

	GetMesh()->SetRelativeRotation(Result);
	//GetMesh()->SetRelativeLocationAndRotation(StartPosition + TricksLocationCurves[EligibleTrick.TrickIndex]->GetVectorValue(TimeCounter), FRotator(NewRotation.Z, NewRotation.Y, NewRotation.X).Quaternion());
	//GetMesh()->AddLocalRotation(FRotator(1.f, 0.f, 0.f).Quaternion());

	TimeCounter += DeltaTime;
	if (TimeCounter >= TrickTime)
	{
		TimeCounter = 0.f;
		bExecutingTrick = false;
	}
}

void ASkateProjectCharacter::HandleAnalogInput(float DeltaTime)
{
	FVector2D AnalogLocation = AnalogRaw;
	AnalogLocation.Normalize();

	//Update analog location in UI
	if (SkatePC) SkatePC->UpdateAnalogLocation(AnalogRaw);

	//After the player executed a trick, the analog must return to the deadzone
	if (bReturningFromTrick)
	{
		if (AnalogRaw.Size() > Deadzone)
			return;
		bReturningFromTrick = false;
	}

	//Check if the player is outside the safezone
	if (bInsideSafezone && (AnalogRaw - LastTrickLocation).Size() >= TrickPointExecDistance*2.f)
	{
		bInsideSafezone = false;
	}

	if (bInsideSafezone) return;

	//Check if the player is still dragging the analog along the edge
	if (bIsDragging && !bInsideSafezone && AnalogRaw.Size() < 0.9f)
		bIsDragging = false;

	AnalyzeTricks();

	FVector2D Dir = AnalogRaw - PreviousLocation;
	PreviousLocation = AnalogRaw;
}

void ASkateProjectCharacter::AnalyzeTricks()
{
	for (int i = 0; i < TrickQueue.Num(); i++)
	{
		//Only check for angles between trickpoints if the player already reached a trick point
		if (TrickPointDepth != 0)
		{
			//Check angle between analog direction and trickpoint direction
			FVector2D Vector1 = AnalogRaw - LastTrickLocation;
			FVector2D Vector2 = TrickQueue[i].Points[TrickPointDepth].DesiredPosition - LastTrickLocation;
			Vector1.Normalize();
			Vector2.Normalize();
			float Dot = FVector2D::DotProduct(Vector1, Vector2);

			/*
			If this trick requires drag and player isn't dragging
			or the analog isn't going towards the direction of this trick,
			remove trick from queue
			*/
			if (TrickQueue[i].Points[TrickPointDepth].bDrag && !bIsDragging
				||
				!TrickQueue[i].Points[TrickPointDepth].bDrag && Dot < MinimumRequiredDot)
			{
				PRINTTEXT(FString::Printf(TEXT("Removed %s"), *TrickQueue[i].TrickName.ToString()));
				RemoveTrick(i);
				break;
			}
		}

		//Check if the analog has reached a TrickPoint location
		if ((AnalogRaw - TrickQueue[i].Points[TrickPointDepth].DesiredPosition).Size() <= TrickPointExecDistance)
		{
			//Give the player a safezone after reaching a trickpoint
			bInsideSafezone = true;
			bIsDragging = true;
			TrickPointDepth++;
			LastTrickLocation = TrickQueue[i].Points[TrickPointDepth - 1].DesiredPosition;

			//If this trick hit its last trick point, select as eligible trick
			if (TrickPointDepth == TrickQueue[i].Points.Num())
			{
				EligibleTrick = TrickQueue[i];
				RemoveTrick(i);
				if (TrickPointDepth == 0) break;
			}

			/*Remove all tricks that don't have this trickpoint at this trickpoint depth
			and all the tricks with the same amount of trickpoints as the one at the current index*/
			int TricksRemoved = 0;
			for (int j = 0; j < (TrickQueue.Num() + TricksRemoved); ++j)
			{
				if ((TrickQueue[j - TricksRemoved].Points.Num() == TrickPointDepth) ||
					(TrickQueue[j - TricksRemoved].Points[TrickPointDepth - 1].DesiredPosition != LastTrickLocation))
				{
					//PRINTTEXT(FString::Printf(TEXT("Removed %s"), *TrickQueue[j - TricksRemoved].TrickName.ToString()));
					RemoveTrick(j - TricksRemoved);
					//If tricks were reset, stop searching
					if (TrickPointDepth == 0) break;
					TricksRemoved++;
				}
			}

			break;
		}
	}
}

void ASkateProjectCharacter::AttemptExecuteTrick()
{
	//Check if the eligible trick is valid
	if (EligibleTrick.Points.Num() != 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Executed %s"), *EligibleTrick.TrickName.ToString()));
		if (TricksLocationCurves.Num() > EligibleTrick.TrickIndex)
		{
			bExecutingTrick = true;
			float Min, Max;
			TricksLocationCurves[EligibleTrick.TrickIndex]->GetTimeRange(Min, Max);
			TrickTime = Max;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Trick Curve not found!"), *EligibleTrick.TrickName.ToString()));
		}
	}
	TrickPointDepth = 0;
	LastTrickLocation = FVector2D(0.f, 0.f);
	bIsDragging = false;
	bReturningFromTrick = true;
	bInsideSafezone = false;
	TrickQueue.Empty();
	TrickQueue.Append(Tricks);
	EligibleTrick.Points.Empty();
}

void ASkateProjectCharacter::RemoveTrick(int index)
{
	TrickQueue.RemoveAt(index);
	//If there are no more eligible tricks, attempt to execute the eligible trick
	if (TrickQueue.Num() == 0) AttemptExecuteTrick();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASkateProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASkateProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASkateProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("TurnRate", this, &ASkateProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASkateProjectCharacter::LookUpAtRate);
}

void ASkateProjectCharacter::TurnAtRate(float Rate)
{
	AnalogRaw.X = Rate;
}

void ASkateProjectCharacter::LookUpAtRate(float Rate)
{
	AnalogRaw.Y = -Rate;
}

void ASkateProjectCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASkateProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
