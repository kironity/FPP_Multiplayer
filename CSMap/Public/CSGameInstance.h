// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CSMAP_API UCSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public: 

	/*UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void LaunchLobby(int32 NumOfPlayers, bool bLAN, FName ServerName);*/

	/*UFUNCTION(BlueprintImplementableEvent)
	void JoinLobby(const FOnlineSessionSearchResult& FindResult);*/

public:
	/** Public Variables*/

protected:
	/** Protected Variables */

	//const FOnlineSessionSearchResult& SearchResult;
};
