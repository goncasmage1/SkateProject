// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SkateProjectGameMode.h"
#include "SkateProjectCharacter.h"
#include "SkateController.h"
#include "UObject/ConstructorHelpers.h"

ASkateProjectGameMode::ASkateProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = ASkateController::StaticClass();
}
