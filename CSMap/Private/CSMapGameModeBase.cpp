// Copyright Epic Games, Inc. All Rights Reserved.


#include "CSMapGameModeBase.h"
#include "CSGameState.h"
#include "CSPlayerController.h"
#include "CSCharacter.h"
#include "Kismet/KismetStringLibrary.h"

ACSMapGameModeBase::ACSMapGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true; 
	PrimaryActorTick.TickInterval = 1.f;
	DefaultRoundTime = 180;
	DefaultWarmTime = 180;
	DefaultDeltaTime = 8;
	GameStateClass = ACSGameState::StaticClass();

	TeamRedPoints = 0;

	TeamBluePoints = 0;
	
	TeamBlueDeaths = 0;
	TeamReadDeaths = 0;
}


void ACSMapGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ACSPlayerController* MyPC = Cast<ACSPlayerController>(It->Get());
		if (MyPC && MyPC->GetPawn() == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("PlayerController is valid "));
			RestartPlayer(MyPC);
		}
	}
}
void ACSMapGameModeBase::StartPlay()
{
	Super::StartPlay();
	MyGS = GetGameState<ACSGameState>();
	RoundTime = DefaultRoundTime;
	WarmTime = DefaultWarmTime;
	RoundDelta = DefaultDeltaTime;
	
//	StartWarm();
}

void ACSMapGameModeBase::StartWarm()
{
	if (GetWarmTime() <= 0)
	{
		StartRound();
	}
	MyGS->CurrentRoundState = ERoundState::WarmRound;
	GetWorldTimerManager().SetTimer(WarmTimerHandle, this, &ACSMapGameModeBase::WarmTimer, 1.f, true, 0.f);
}

void ACSMapGameModeBase::WarmTimer()
{
	if (WarmTime < 0)
	{
		StopWarm();
		return;
	}
	if (ensure(MyGS))
	{
		MyGS->CurrentWarmTime = WarmTime;
		WarmTime--;
	}
}

void ACSMapGameModeBase::StopWarm()
{
	StartRound();
	GetWorldTimerManager().ClearTimer(WarmTimerHandle);
}

void ACSMapGameModeBase::StartRound()
{
	if (GetDeltaTime() <= 0)
	{
		BeginRound();
	}
	else
	{
		StartDelta();
	}
}

void ACSMapGameModeBase::BeginRound()
{
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ACSMapGameModeBase::RoundTimer, 1.f, true, 0.f);
	MyGS->CurrentRoundState = ERoundState::MainRound;
	MyGS->MulticastOnRoundStarted();
	
}

void ACSMapGameModeBase::StopRound()
{
	RoundTime = DefaultRoundTime;
	GetWorldTimerManager().ClearTimer(RoundTimerHandle); 
}

void ACSMapGameModeBase::EndRound()
{

}

void ACSMapGameModeBase::RoundTimer()
{
	if (ensure(MyGS))
	{
		MyGS->CurrentRoundTime = RoundTime;
		RoundTime--;
	}
}

void ACSMapGameModeBase::StartDelta()
{
	MyGS->CurrentRoundState = ERoundState::DeltaRound;
	GetWorldTimerManager().SetTimer(DeltaTimerHandle, this, &ACSMapGameModeBase::DeltaTimer, 1.f, true, 0.f);
	MyGS->MulticastOnDeltaRoundStarted();
}

void ACSMapGameModeBase::DeltaTimer()
{
	if (RoundDelta < 0)
	{
		StopDelta();
		return;
	}
	if (ensure(MyGS))
	{
		MyGS->CurrentDeltaTime = RoundDelta;
		RoundDelta--;
	}
}

void ACSMapGameModeBase::StopDelta()
{
	GetWorldTimerManager().ClearTimer(DeltaTimerHandle);
	BeginRound();
}

void ACSMapGameModeBase::PlayerDeath(class ACSCharacter* DeathPlayer)
{
	if (DeathPlayer->GetController())
	{
		ACSPlayerController* MyPC = Cast<ACSPlayerController>(DeathPlayer->GetController());
		
		TEnumAsByte<EGameTeam> ByteTeam = MyPC->PlayerTeam;
		FString EnumString = UEnum::GetValueAsString(ByteTeam);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString(EnumString));
	}
	ACSPlayerController* MyPC = Cast<ACSPlayerController>(DeathPlayer->GetController());
	switch (MyPC->PlayerTeam)
	{
	case EGameTeam::Blue:
		TeamBlueDeaths++;
		if (TeamBlueDeaths >= TeamBlue.Num())
		{
			TeamRedPoints++;
			MyGS->TeamRedPoints = TeamRedPoints;
			TeamBlueDeaths = 0;
		}
		break;
	case EGameTeam::Red:
		TeamReadDeaths++;
		if (TeamReadDeaths == TeamRed.Num())
		{
			TeamBluePoints++;
			MyGS->TeamBluePoints = TeamBluePoints;
			TeamReadDeaths = 0;
		}
		break;
	default:
		break;
	}
}

void ACSMapGameModeBase::AddToTeamBlue(class ACSCharacter* NewCharacter)
{
	ACSPlayerController* MyPC = Cast<ACSPlayerController>(NewCharacter->GetController());
	MyPC->PlayerTeam = EGameTeam::Blue;
	TeamBlue.AddUnique(NewCharacter);
}

void ACSMapGameModeBase::AddToTeamRed(class ACSCharacter* NewCharacter)
{
	ACSPlayerController* MyPC = Cast<ACSPlayerController>(NewCharacter->GetController());
	MyPC->PlayerTeam = EGameTeam::Red;
	TeamRed.AddUnique(NewCharacter);
}
