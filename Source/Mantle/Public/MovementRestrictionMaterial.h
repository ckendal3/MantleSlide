// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MovementRestrictionMaterial.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MANTLE_API UMovementRestrictionMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bCanMantle = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bCanJump = true;
};
