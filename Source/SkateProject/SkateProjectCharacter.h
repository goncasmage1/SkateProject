// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkateProjectCharacter.generated.h"

USTRUCT(BlueprintType)
struct FTrickPoint
{
	GENERATED_BODY()

	//Angle in degrees that will execute this point of the trick
	UPROPERTY(BlueprintReadWrite)
		int	Angle;
	//The 2D Position in which the analog must me moved in order to hit this point
	//UPROPERTY(BlueprintReadWrite)
		FVector2D DesiredPosition;

	//Whether the player should move analog directly to this point, or drag along the edge
	UPROPERTY(BlueprintReadWrite)
		bool bDrag;
};

USTRUCT(BlueprintType)
struct FTrick
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		FName TrickName;
	UPROPERTY(BlueprintReadWrite)
		TArray<FTrickPoint> Points;
};

UCLASS(config=Game)
class ASkateProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FTrick> Tricks;

	/*
	The minimum dot product for the trick point to be considered as eligible, 
	i.e. if the dot between the Analog's direction and the trick point's direction
	is less than this value, the trick is no longer eligible for execution
	*/
	UPROPERTY(EditDefaultsOnly)
		float MinimumRequiredDot;

	//The distance at which the analog has to be from a trick point to execute it
	UPROPERTY(EditDefaultsOnly)
		float TrickPointExecDistance;

	//Analog inner deadzone
	UPROPERTY(EditDefaultsOnly)
		float Deadzone;

	ASkateProjectCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	//Called after the trick array is set in the blueprint on BeginPlay
	UFUNCTION(BlueprintCallable)
		void CustomBeginPlay();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	class ASkateController* SkatePC;

	//Keeps all the tricks that can still be executed
	TArray<FTrick> TrickQueue;
	//The trick that is currently able to be executed
	FTrick EligibleTrick;
	//Raw analog value
	FVector2D AnalogRaw;
	//Raw analog value from last frame
	FVector2D PreviousLocation;
	//Raw analog value when the last trick point was executed
	FVector2D LastTrickLocation;

	int TrickPointIndex;

	//Is the player dragging the analog around the edge?
	bool bIsDragging;
	//Determines whether the analog is still returning to origin after doing a trick
	bool bReturningFromTrick;

	/*Attempts to execute an eligible trick and resets the variables accordingly
	to allow the player to execute another trick*/
	void AttemptExecuteTrick();
	
	/*Removes the trick located at the specified index
	and returns whether there are no more tricks left to execute*/
	void RemoveTrick(int index);

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

