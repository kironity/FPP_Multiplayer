// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CSBaseCharacter.generated.h"

UCLASS()
class CSMAP_API ACSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/** Constructor Defaults */
	// Sets default values for this character's properties
	ACSBaseCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FirstPerson")
	class USkeletalMeshComponent* Mesh1p;

	/** Define Camera manager location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|CameraManager")
	class USceneComponent* CMLocation;

public:
	/** Public Functions */
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	// Location of Defined Camera location 
	UFUNCTION(BlueprintPure, Category = "Camera|CameraManager")
	FORCEINLINE FVector GetCameraLocation() { return CMLocation->GetComponentLocation(); }

protected:
	/** Protected Functions */

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Axis Locomotion
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);
#pragma endregion 


};
