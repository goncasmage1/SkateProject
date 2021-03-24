// Fill out your copyright notice in the Description page of Project Settings.

#include "SkateController.h"
#include "AnalogViewWidget.h"
#include "Blueprint/UserWidget.h"

void ASkateController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);
	if (AnalogViewBP)
	{
		//Creating our widget and adding it to our viewport
		AnalogViewWidget = CreateWidget<UAnalogViewWidget>(this, AnalogViewBP);

		AnalogViewWidget->Show();
	}
}

void ASkateController::UpdateAnalogLocation(FVector2D NewLocation)
{
	if (AnalogViewWidget)
	{
		AnalogViewWidget->UpdateAnalogLocation(NewLocation);
	}
}

void ASkateController::UpdateLastLocation(FVector2D NewLocation)
{
	if (AnalogViewWidget)
	{
		AnalogViewWidget->UpdateLastLocation(NewLocation);
	}
}

void ASkateController::UpdateTrickLocation(FVector2D NewLocation)
{
	if (AnalogViewWidget)
	{
		AnalogViewWidget->UpdateTrickLocation(NewLocation);
	}
}
