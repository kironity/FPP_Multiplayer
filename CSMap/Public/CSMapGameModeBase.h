// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CSMapGameModeBase.generated.h"

/**
 * 
 */

UCLASS()
class CSMAP_API ACSMapGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/** Public Functions */

	virtual void Tick(float DeltaTime) override;

	ACSMapGameModeBase();

	virtual void StartPlay() override;
	
	void PlayerDeath(class ACSCharacter* DeathPlayer);

	UFUNCTION(BlueprintCallable)
	void AddToTeamBlue(class ACSCharacter* NewCharacter);

	UFUNCTION(BlueprintCallable)
	void AddToTeamRed(class ACSCharacter* NewCharacter);

	/** Getter and Setter to RoundTime */
	FORCEINLINE int32 GetRoundTime() { return RoundTime; }
	FORCEINLINE void SetRoundTime(int32 NewTime) { RoundTime = NewTime; }

	/** Getter and Setter to DeltaTime */
	FORCEINLINE int32 GetDeltaTime() { return RoundDelta; }
	FORCEINLINE void SetDeltaTime(int32 NewDeltaTime) { RoundDelta = NewDeltaTime; }

	/** Getter and Setter to WarmTime */
	FORCEINLINE int32 GetWarmTime() { return WarmTime; }
	FORCEINLINE void SetWarmTime(int32 NewWarmTime) { WarmTime = NewWarmTime; }

public:
	/** Public variables */

	UPROPERTY(BlueprintReadWrite)
	TArray<class ACSCharacter*> TeamBlue;

	UPROPERTY(BlueprintReadWrite)
	TArray<class ACSCharacter*> TeamRed;

protected:
	/** Protected Functions*/

	/** Realize Warm Up */
	UFUNCTION()
	void StartWarm();

	UFUNCTION()
	void StopWarm();

	UFUNCTION()
	void WarmTimer();

	
	/** Main Round */
	UFUNCTION()
	void StartRound();

	UFUNCTION()
	void BeginRound();

	UFUNCTION()
	void StopRound();

	UFUNCTION()
	void EndRound();

	UFUNCTION()
	void RoundTimer();

	UFUNCTION()
	void StartDelta();

	UFUNCTION()
	void DeltaTimer();

	UFUNCTION()
	void StopDelta();

protected:
	/** Protected Variables */

	class ACSGameState* MyGS;

private: 

	/** Private Variables */

	int32 WarmTime;

	int32 RoundTime;

	int32 RoundDelta;

	int32 TeamRedPoints;

	int32 TeamReadDeaths;

	int32 TeamBlueDeaths;

	int32 TeamBluePoints;

	UPROPERTY(EditDefaultsOnly, Category = "RoundProperty")
	int32 DefaultWarmTime;

	UPROPERTY(EditDefaultsOnly, Category = "RoundProperty")
	int32 DefaultRoundTime;

	UPROPERTY(EditDefaultsOnly, Category = "RoundProperty")
	int32 DefaultDeltaTime;

	FTimerHandle WarmTimerHandle;

	FTimerHandle RoundTimerHandle;

	FTimerHandle DeltaTimerHandle;

};
