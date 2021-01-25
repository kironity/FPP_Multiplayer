// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPlayerController.h"
#include "CSMapGameModeBase.h"
#include "CSMapGameModeBase.h"
#include "CSCharacter.h"
#include "HealthStats.h"
#include "CSPlayerCameraManager.h"

ACSPlayerController::ACSPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ACSPlayerCameraManager::StaticClass();
}
void ACSPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	if (aPawn)
	{
		ACSCharacter* MyCharacter = Cast<ACSCharacter>(GetPawn());
		MyCharacter->HealthStat->ResetHealth();
		UpdateHealth(MyCharacter->HealthStat->GetCurrentHealth());
	}
}

void ACSPlayerController::UpdateHealth_Implementation(float Health)
{
	UpdateHealth_OnClient(Health);
}

void ACSPlayerController::SetGMRoundTimer(int32 NewTime)
{
	ServerConsole(NewTime);
}

void ACSPlayerController::ServerConsole_Implementation(int32 NewTime)
{
	ACSMapGameModeBase* GM = GetWorld()->GetAuthGameMode<ACSMapGameModeBase>();
	if (GM)
	{
		GM->SetRoundTime(NewTime);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Red, FString("GameMode is NULL"));
		UE_LOG(LogTemp, Warning, TEXT("GameMode is NULL"));
	}
}