// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"

void UMyCharacterMovementComponent::SetSprint(bool bSprint)
{
	bSprintKeyDown = bSprint;
}

void UMyCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (GetPawnOwner()->IsLocallyControlled())
	{
		if (bSprintKeyDown)
		{
			FVector Speed2D = GetPawnOwner()->GetVelocity();
			FVector ForwardVector2D = GetPawnOwner()->GetActorForwardVector();
			Speed2D.Z = 0.f;
			ForwardVector2D.Z = 0.f;

			WantsToSprint = FVector::DotProduct(Speed2D, ForwardVector2D) > 0.5f;
		}
		else
		{
			WantsToSprint = false; 
		}
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UMyCharacterMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		if (IsCrouching())
		{
			return MaxWalkSpeedCrouched;
		}
		else
		{
			if (WantsToSprint)
			{
				return MaxSprintSpeed;
			}

			return MaxWalkSpeed;
		}
	case MOVE_Falling:
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

float UMyCharacterMovementComponent::GetMaxAcceleration() const
{
	if (WantsToSprint)
	{
		return SprintAcceleration;
	}
	return Super::GetMaxAcceleration();
}

FNetworkPredictionData_Client* UMyCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UMyCharacterMovementComponent* MutableThis = const_cast<UMyCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_My(*this);
	}
	return ClientPredictionData;
}

void UMyCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	WantsToSprint = (Flags & FSavedMove_Character_My::FLAG_Custom_0) != 0;
}

void FSavedMove_Character_My::Clear()
{
	Super::Clear();
	SavedWantsToSprint = 0;
}

void FSavedMove_Character_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UMyCharacterMovementComponent* MyMovement = Cast<UMyCharacterMovementComponent>(Character->GetCharacterMovement());
	if (MyMovement)
	{
		SavedWantsToSprint = MyMovement->WantsToSprint;
	}
}

bool FSavedMove_Character_My::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Character_My* MySave = static_cast<const FSavedMove_Character_My*>(NewMove.Get());
	if (SavedWantsToSprint != MySave->SavedWantsToSprint)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Character_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UMyCharacterMovementComponent* MyMovement = Cast<UMyCharacterMovementComponent>(Character->GetCharacterMovement());
	if (MyMovement)
	{
		MyMovement->WantsToSprint = SavedWantsToSprint;
	}
}

uint8 FSavedMove_Character_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedWantsToSprint)
		Result |= FLAG_Custom_0;

	return Result;
}


FNetworkPredictionData_Client_Character_My::FNetworkPredictionData_Client_Character_My
(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Character_My::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Character_My());
}
