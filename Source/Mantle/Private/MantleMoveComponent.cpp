// Fill out your copyright notice in the Description page of Project Settings.


#include "MantleMoveComponent.h"

#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Mantle/Public/MovementRestrictionMaterial.h"

// TODO: MANTLE - Need to compensate for in air mantling. Need to tighten the mantle min/max heights to make it easier
// TODO: MANTLE - Need to add look at feature to make mantle less accidental
// TODO: MANTLE - Need to make sure player velocity isn't opposite of look direction
// TODO: MANTLE - Need to do a multi-shape sweep to see if there is an open space on the way down (Compare top hit or top position to first hit)

void UMantleMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bWantsToMantle && MantleData.bValidMantle && MantleCurve)
	{
		UpdateMantle();
	}
	else
	{
		ResetMantle();
	}	
}

void UMantleMoveComponent::BeginPlay()
{
}

bool UMantleMoveComponent::CheckForMantle()
{
	UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
	FQuat Rot = GetOwner()->GetActorQuat();
	
	FCollisionParameters Params;
	Params.CollisionQueryParam.AddIgnoredActor(GetOwner());

	TArray<AActor*> Actors;
	
	FHitResult OutHit_Ledge;

	float PlayerRadius = CapsuleComp->GetScaledCapsuleRadius();
	float PlayerHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	// Find Mantle To Position (from the top downwards)
	{
		// Start position is forward two player widths and then Max mantle height from the floor
		FVector StartPos = GetActorLocation() + (GetOwner()->GetActorForwardVector() * (PlayerRadius * 2));
		StartPos.Z = (StartPos.Z - PlayerHalfHeight) + MaxMantleHeight;
		
		// End position is forward two player widths and then min mantle height from the floor
		FVector EndPos = GetActorLocation() + (GetOwner()->GetActorForwardVector() * (PlayerRadius * 2));
		EndPos.Z = (EndPos.Z - PlayerHalfHeight) + MinMantleHeight;

		// TODO: MANTLE - Set Collision channel to something proper
		Params.CollisionShape = FCollisionShape::MakeSphere(PlayerRadius);
		Params.CollisionQueryParam.bReturnPhysicalMaterial = true;
		//Params.ObjectQueryParam = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic)); // FCollisionObjectQuery has type if you hover over it

		FCollisionObjectQueryParams ObjectQueryParams;
		
		// bool HasHit = GetWorld()->SweepSingleByChannel(OutHit_Ledge, StartPos, EndPos, Rot, ECC_Visibility, Params.CollisionShape, Params.CollisionQueryParam); // ECC_GameTraceChannel1 is custom trace channel in project settings

		TArray<FHitResult> Hits;

		//bool HasHit = GetWorld()->SweepMultiByObjectType(Hits, StartPos, EndPos, FQuat::Identity, ObjectQueryParams, Params.CollisionShape, Params.CollisionQueryParam);
		//bool HasHit = GetWorld()->LineTraceMultiByChannel(Hits, StartPos, EndPos, ECC_WorldStatic);

		bool HasHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartPos, EndPos, PlayerRadius, MantleTraceObjects, false, Actors, EDrawDebugTrace::ForDuration, Hits, true);

		UE_LOG(LogTemp, Warning, TEXT("Overlaps: %d"), Hits.Num());

		if(HasHit)
		{
			OutHit_Ledge = Hits.Last();
			
			// Can we mantle on this material?
			UMovementRestrictionMaterial* PhysMat = (UMovementRestrictionMaterial*) OutHit_Ledge.PhysMaterial.Get();
			if(OutHit_Ledge.PhysMaterial.IsValid() && PhysMat && PhysMat->bCanMantle == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("PHYS MATERIAL DOESNT ALLOW MANTLE"));
				ResetMantle();
				return false;
			}
		}
		else
		{	
			//DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red, false, 1, 2);
			//DrawDebugSphere(GetWorld(), EndPos, PlayerRadius, 16, FColor::Red, false, 5, 2);
			ResetMantle();
			return false;
		}
	}


	// Check Position to Mantle to make sure its open
	{
		// If we hit ground and can walk on it
		if(OutHit_Ledge.bBlockingHit && IsWalkable(OutHit_Ledge))
		{
			// Guarantee that we get off the ground
			const float ZOffset = .1f;
		
			const FVector CapsulePos = OutHit_Ledge.Location + FVector(0, 0, PlayerHalfHeight);
			Params.CollisionShape = FCollisionShape::MakeCapsule(PlayerRadius, PlayerHalfHeight);			

			FHitResult OutHit_Capsule;
			//bool HasHit = GetWorld()->SweepSingleByChannel(OutHit_Capsule, CapsulePos, CapsulePos, Rot, ECC_Visibility, Params.CollisionShape, Params.CollisionQueryParam);

			bool HasHit = UKismetSystemLibrary::CapsuleTraceSingleForObjects(this, CapsulePos, CapsulePos, PlayerRadius, PlayerHalfHeight, MantleTraceObjects, false, Actors, EDrawDebugTrace::ForDuration, OutHit_Capsule, true);


			// Can't Mantle
			if(HasHit)
			{
				// DrawDebugCapsule(GetWorld(), FVector(CapsulePos.X, CapsulePos.Y, CapsulePos.Z + PlayerHalfHeight), PlayerHalfHeight, PlayerRadius, Rot, FColor::Red, false, 10.f, 0, 1);
			}
			else
			{
				// We can mantle
				FVector TMP_TopLinePos = OutHit_Ledge.Location;
				TMP_TopLinePos.Z = (TMP_TopLinePos.Z - PlayerHalfHeight) + MaxMantleHeight;

				FVector TMP_CapPos = OutHit_Ledge.Location;
				TMP_CapPos.Z += (PlayerHalfHeight - PlayerRadius);
			
				// DrawDebugLine(GetWorld(), TMP_CapPos, TMP_TopLinePos, FColor::Green, false, 10, 3, 3);			
				// DrawDebugCapsule(GetWorld(), FVector(TMP_CapPos.X, TMP_CapPos.Y, TMP_CapPos.Z), PlayerHalfHeight, PlayerRadius, Rot, FColor::Green, false, 5.f, 0, 1);
				
				MantleData.bValidMantle = true;
				MantleData.StartLocation = GetOwner()->GetActorLocation();
				MantleData.EndLocation = CapsulePos;
				MantleData.CurrentProgress = 0.f;
				bIsMantling = true;

				GetCharacterOwner()->SetActorEnableCollision(false);
				
				return true;
			}			
		}
	}

	ResetMantle();
	return false;
}

bool UMantleMoveComponent::CanMantle()
{
	return !bIsMantling;
}

void UMantleMoveComponent::PerformMantle()
{
	if(MantleData.bValidMantle)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Is valid Mantle"));
		
		bWantsToMantle = true;
		//GetCharacterOwner()->TeleportTo(MantleData.EndLocation, GetCharacterOwner()->GetActorRotation());		
	}
	else
	{
		ResetMantle();
	}
}

void UMantleMoveComponent::UpdateMantle()
{
	MantleData.CurrentProgress += GetWorld()->GetDeltaSeconds();
	
	float NewProgress = FMath::Clamp(MantleData.CurrentProgress / TimeToMantle, 0.f, 1.f);
	
	FVector NewPos = FMath::Lerp(MantleData.StartLocation, MantleData.EndLocation, NewProgress);

	// TODO: See if move is valid
	GetCharacterOwner()->TeleportTo(NewPos, GetCharacterOwner()->GetActorRotation(), false, false);

	//GEngine->AddOnScreenDebugMessage(9999, 1.f, FColor::Blue, FString::Printf(TEXT("Progress: %f"), NewProgress));

	if(FMath::IsNearlyEqual(NewProgress, 1.f))
	{		
		ResetMantle();
	}
}

void UMantleMoveComponent::ResetMantle()
{
	MantleData.bValidMantle = false;
	MantleData.CurrentProgress = 0.f;
	MantleData.StartLocation = FVector::ZeroVector;
	MantleData.EndLocation = FVector::ZeroVector;

	bWantsToMantle = false;
	bIsMantling = false;
	
	GetCharacterOwner()->SetActorEnableCollision(true);
}
