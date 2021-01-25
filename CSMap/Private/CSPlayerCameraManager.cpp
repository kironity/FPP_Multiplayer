// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPlayerCameraManager.h"
#include "CSCharacter.h"
ACSPlayerCameraManager::ACSPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void ACSPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);
	ACSCharacter* MyPawn = PCOwner ? PCOwner->GetPawn<ACSCharacter>() : nullptr;
	if (MyPawn)
	{
		MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}
