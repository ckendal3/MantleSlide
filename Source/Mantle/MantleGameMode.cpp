// Copyright Epic Games, Inc. All Rights Reserved.

#include "MantleGameMode.h"
#include "MantleHUD.h"
#include "MantleCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMantleGameMode::AMantleGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMantleHUD::StaticClass();
}
