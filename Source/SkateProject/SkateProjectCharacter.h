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
	UFUNCTION(BlueprintCallable)
		void CustomBeginPlay();
	virtual void Tick(float DeltaTime) override;

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
	//Fetches the raw analog values
	FVector2D AnalogRaw;
	//Saves the position of the analog in the last tick event
	FVector2D PreviousLocation;
	//Tracks the position of the last reached trick point
	FVector2D LastTrickLocation;
	int TrickPointIndex;

	//Is the player dragging the analog around the edge?
	bool bIsDragging;
	//Determines whether the analog is still returning to origin after doing a trick
	bool bReturningFromTrick;

	void AttemptExecuteTrick();
	bool RemoveTrick(int index);

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

