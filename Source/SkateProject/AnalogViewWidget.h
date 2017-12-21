// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnalogViewWidget.generated.h"

/**
 * 
 */
UCLASS()
class SKATEPROJECT_API UAnalogViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
		void Show();

	UFUNCTION(BlueprintImplementableEvent)
		void Hide();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateAnalogLocation(FVector2D NewLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateLastLocation(FVector2D NewLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateTrickLocation(FVector2D NewLocation);

};
