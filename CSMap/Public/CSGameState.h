// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CSGameState.generated.h"

UENUM(Blueprintable)
enum class ERoundState : uint8
{
	WarmRound,
	MainRound,
	DeltaRound,
};
/**
 * 
 */
UCLASS()
class CSMAP_API ACSGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	/** Public Functions */

	UFUNCTION(BlueprintImplementableEvent)
	void RoundUpdated(int32 CurrentTime);

	UFUNCTION(BlueprintImplementableEvent)
	void WarmUpdated(int32 NewWarmTime);

	UFUNCTION(BlueprintImplementableEvent)
	void DeltaUpdated(int32 NewDeltaTime);

	UFUNCTION(BlueprintImplementableEvent)
	void RoundStateChanged(ERoundState NewState);

	UFUNCTION(BlueprintImplementableEvent)
	void OnBlueTeamPointsUpdate(int32 NewPoints);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRedTeamPointsUpdate(int32 NewPoints);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnDeltaRoundStarted();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundStarted();

public:
	/** Public Variables */

	UPROPERTY(ReplicatedUsing = OnRep_WarmRoundUpdate)
	int32 CurrentWarmTime;

	UPROPERTY(ReplicatedUsing = OnRep_RoundTimeUpdate)
	int32 CurrentRoundTime;

	UPROPERTY(ReplicatedUsing = OnRep_DeltaTimeUpdate)
	int32 CurrentDeltaTime;

	UPROPERTY(ReplicatedUsing = OnRep_OnTeamBlueUpdated)
	int32 TeamBluePoints;

	UPROPERTY(ReplicatedUsing = OnRep_OnRedTeamUpdated)
	int32 TeamRedPoints;

	UPROPERTY(ReplicatedUsing = OnRep_OnRoundStateChanged)
	ERoundState CurrentRoundState;

	UPROPERTY(BlueprintReadWrite)
	TArray<class ACSCharacter*> BlueTeam;

	UPROPERTY(BlueprintReadWrite)
	TArray<class ACSCharacter*> RedTeam;

protected:

	/** Protected Functions */
	UFUNCTION()
	void OnRep_WarmRoundUpdate();

	UFUNCTION()
	void OnRep_RoundTimeUpdate();

	UFUNCTION()
	void OnRep_DeltaTimeUpdate();

	UFUNCTION()
	void OnRep_OnRoundStateChanged();

	UFUNCTION()
	void OnRep_OnTeamBlueUpdated();

	UFUNCTION()
	void OnRep_OnRedTeamUpdated();
	
};
