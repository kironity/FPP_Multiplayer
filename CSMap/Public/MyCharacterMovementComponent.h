// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CSMAP_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	friend class FSavedMove_Character_My;
public:
	/** Public Functions */
	UFUNCTION(BlueprintCallable, Category = "CustomMovement")
	void SetSprint(bool bSprint);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool isSprinting() { return bSprintKeyDown; }

public:
	/** Public Variables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Sprinting", meta = (ClampMin = "0"))
	float SprintAcceleration = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Sprinting", meta = (ClampMin = "0"))
	float MaxSprintSpeed = 800.f;
	
protected:
	/** Protected Functions */
	virtual void UpdateFromCompressedFlags(uint8 Flags);
private:
	/** Private variables */
	bool bSprintKeyDown = false;
	uint8 WantsToSprint : 1; 

	
};

class FSavedMove_Character_My : public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;

	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear() override;

	/** Called to set up this saved move (when initially created) to make a predictive correction. */
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;

	/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	/** Called before ClientUpdatePosition uses this SavedMove to make a predictive correction	*/
	virtual void PrepMoveFor(ACharacter* Character) override;

	/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)*/
	virtual uint8 GetCompressedFlags() const override;

private: 
	uint8 SavedWantsToSprint : 1;

};

class FNetworkPredictionData_Client_Character_My : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;
public:
	FNetworkPredictionData_Client_Character_My(const UCharacterMovementComponent& ClientMovement);

	/** Allocate a new saved move. Subclasses should override this if they want to use a custom move class. */
	virtual FSavedMovePtr AllocateNewMove();
};
