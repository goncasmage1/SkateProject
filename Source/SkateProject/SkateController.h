// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SkateController.generated.h"

/**
 * 
 */
UCLASS()
class SKATEPROJECT_API ASkateController : public APlayerController
{
	GENERATED_BODY()

private:

	class UAnalogViewWidget* AnalogViewWidget;
	
protected:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UAnalogViewWidget> AnalogViewBP;

public:

	virtual void Possess(APawn* NewPawn) override;

	void UpdateAnalogLocation(FVector2D NewLocation);
	void UpdateLastLocation(FVector2D NewLocation);
	void UpdateTrickLocation(FVector2D NewLocation);
	
};
