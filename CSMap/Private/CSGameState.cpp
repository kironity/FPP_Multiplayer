// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "CSPlayerController.h"
#include "CSCharacter.h"

void ACSGameState::OnRep_RoundTimeUpdate()
{
	RoundUpdated(CurrentRoundTime);
}

void ACSGameState::OnRep_WarmRoundUpdate()
{
	WarmUpdated(CurrentWarmTime);
}

void ACSGameState::OnRep_DeltaTimeUpdate()
{
	DeltaUpdated(CurrentDeltaTime);
}

void ACSGameState::OnRep_OnRoundStateChanged()
{
	RoundStateChanged(CurrentRoundState);
}

void ACSGameState::MulticastOnDeltaRoundStarted_Implementation()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ACSPlayerController* MyPC = Cast<ACSPlayerController>(It->Get());
		if (MyPC && MyPC->IsLocalController())
		{
			ACSCharacter* MyCharacter = Cast<ACSCharacter>(MyPC->GetPawn());
			if (MyCharacter)
			{
				MyCharacter->StopAllActions();
				MyCharacter->DisableInput(MyPC);
			}
		}
	}
}

void ACSGameState::MulticastOnRoundStarted_Implementation()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ACSPlayerController* MyPC = Cast<ACSPlayerController>(It->Get());
		if (MyPC && MyPC->IsLocalController())
		{
			//MyPC->IsLocalController()
			ACSCharacter* MyCharacter = Cast<ACSCharacter>(MyPC->GetPawn());
			if (MyCharacter && MyCharacter->isAlive())
			{
				MyCharacter->EnableInput(MyPC);
			}
		}
	}
}

void ACSGameState::OnRep_OnTeamBlueUpdated()
{
	OnBlueTeamPointsUpdate(TeamBluePoints);
}

void ACSGameState::OnRep_OnRedTeamUpdated()
{
	OnRedTeamPointsUpdate(TeamRedPoints);
}
void ACSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSGameState, CurrentRoundTime);
	DOREPLIFETIME(ACSGameState, CurrentWarmTime);
	DOREPLIFETIME(ACSGameState, CurrentDeltaTime);
	DOREPLIFETIME(ACSGameState, CurrentRoundState);
	DOREPLIFETIME(ACSGameState, TeamBluePoints);
	DOREPLIFETIME(ACSGameState, TeamRedPoints);
}