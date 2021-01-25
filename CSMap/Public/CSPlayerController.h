// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CSPlayerController.generated.h"

UENUM(BlueprintType)
enum class EGameTeam : uint8
{
	Blue,
	Red,
	Spectator
};

/**
 * 
 */
UCLASS()
class CSMAP_API ACSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/** Public Functions */

	ACSPlayerController(const FObjectInitializer& ObjectInitializer);
	//Update Widget Healths called only client
	UFUNCTION(Client, Reliable)
	void UpdateHealth(float Health);

	UFUNCTION(exec)
	void SetGMRoundTimer(int32 NewTime);

	UFUNCTION(Server, Reliable)
	void ServerConsole(int32 NewTime);

	/** Set New Camera location manager based on ACSCharacter CMLocation */
	

public: 
	/** Public Variables */

	UPROPERTY(BlueprintReadWrite)
	EGameTeam PlayerTeam;



protected:
	/** Protected Functions*/

	//Update Widget Healths called only client
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealth_OnClient(float Health);

	virtual void OnPossess(APawn* aPawn) override;
	
};
