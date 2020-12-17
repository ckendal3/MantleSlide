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
// TODO: MANTLE - Make sure hit isn't too far over an edge

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

	// Find Position to Mantle (from the top downwards)
	{
		// Start position is forward two player widths and then Max mantle height from the floor
		FVector StartPos = GetActorLocation() + (GetOwner()->GetActorForwardVector() * (PlayerRadius * 2));
		StartPos.Z = (StartPos.Z - PlayerHalfHeight) + MaxMantleHeight;
		
		// End position is forward two player widths and then min mantle height from the floor
		FVector EndPos = GetActorLocation() + (GetOwner()->GetActorForwardVector() * (PlayerRadius * 2));
		EndPos.Z = (EndPos.Z - PlayerHalfHeight) + MinMantleHeight;

		
		Params.CollisionShape = FCollisionShape::MakeSphere(PlayerRadius);
		Params.CollisionQueryParam.bReturnPhysicalMaterial = true;
		//Params.ObjectQueryParam = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic)); // FCollisionObjectQuery has type if you hover over it

		FCollisionObjectQueryParams ObjectQueryParams;
		
		TArray<FHitResult> Hits;		

		bool HasHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartPos, EndPos, PlayerRadius, MantleTraceObjects, false, Actors, bDebugMantle ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, Hits, true, FColor::Cyan, FColor::Magenta);

		// Found floor
		if(HasHit)
		{
			OutHit_Ledge = Hits.Last();
			
			// Can we mantle on this material?
			UMovementRestrictionMaterial* PhysMat = (UMovementRestrictionMaterial*) OutHit_Ledge.PhysMaterial.Get();
			if(OutHit_Ledge.PhysMaterial.IsValid() && PhysMat && PhysMat->bCanMantle == false)
			{
				ResetMantle();
				return false;
			}
		}
		else
		{
			ResetMantle();
			return false;
		}
	}


	// Check Position to Mantle to make sure its open
	{
		// If we hit ground and can walk on it
		if(OutHit_Ledge.bBlockingHit && IsWalkable(OutHit_Ledge))
		{		
			const FVector CapsulePos = OutHit_Ledge.Location + FVector(0, 0, (PlayerHalfHeight - PlayerRadius + .1f));
			Params.CollisionShape = FCollisionShape::MakeCapsule(PlayerRadius, PlayerHalfHeight);			

			FHitResult OutHit_Capsule;

			bool HasHit = UKismetSystemLibrary::CapsuleTraceSingleForObjects(this, CapsulePos, CapsulePos, PlayerRadius, PlayerHalfHeight, MantleTraceObjects, false, Actors,bDebugMantle ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, OutHit_Capsule, true, FColor::Black, FColor::Blue);


			// Didn't hit anything, allow mantle
			if(!HasHit)
			{				
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
	float DotDir = FVector::DotProduct(GetOwner()->GetActorForwardVector() , Velocity.GetSafeNormal2D());

	bool bMovementAllows = false;

	// if not moving backwards or moving very slow
	if(DotDir > -.1f || Velocity.Size2D() < 5)
	{
		bMovementAllows = true;
	}

	return !bIsMantling && bMovementAllows;
}

void UMantleMoveComponent::PerformMantle()
{
	if(MantleData.bValidMantle)
	{
		bWantsToMantle = true;
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
