// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MantleMoveComponent.generated.h"

USTRUCT(BlueprintType)
struct FMantleData
{
	GENERATED_BODY()

public:
	bool bValidMantle;
	float CurrentProgress;
	FVector StartLocation;
	FVector EndLocation;
};


/**
 * 
 */
UCLASS()
class MANTLE_API UMantleMoveComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMantleData MantleData;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;
	
	// The max height a player can mantle (from the top of the player) 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pheonix | Mantle")
	float MaxMantleHeight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pheonix | Mantle")
	float MinMantleHeight = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pheonix | Mantle")
	bool bWantsToMantle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pheonix | Mantle")
	float TimeToMantle = 2.f;

	// TODO: REMOVE WHEN MANTLING IS LOCKED DOWN
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pheonix | Mantle")
	bool bDebugMantle = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Pheonix | Mantle")
	bool bIsMantling = false;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> MantleTraceObjects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pheonix | Mantle")
	UCurveFloat* MantleCurve;
	
	UFUNCTION(Category = "Pheonix | Mantle")
	bool CheckForMantle();

	UFUNCTION(Category = "Pheonix | Mantle")
	bool CanMantle();

	UFUNCTION(Category = "Pheonix | Mantle")
	void PerformMantle();

	UFUNCTION(Category = "Pheonix | Mantle")
	void UpdateMantle();

	UFUNCTION(Category = "Pheonix | Mantle")
	void ResetMantle();
};
