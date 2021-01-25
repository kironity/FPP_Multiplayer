// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "CSCharacter.generated.h"

class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EGroundState : uint8
{
	Run,
	Crouch,
	Transient
};

UENUM()
enum class EOverlayState : uint8
{	
	Aim,
	Fire,
	Reload,
	Transient
};


UCLASS()
class CSMAP_API ACSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties

	ACSCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USkeletalMeshComponent* FP_Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USkeletalMeshComponent* TPP_Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USceneComponent* WeaponCameraLoc;

	// Define New Camera Location for our CSCameraManager
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent* CMLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USceneComponent* WeaponStartLoc;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	//class UCameraComponent* FP_Camera;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UCameraComponent* TPP_Camera;*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UHealthStats* HealthStat;

public:
	/** Public functions */

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable)
	void Server_PlayShootMontages();

	// Reload system 
	void StartReload();
	void ReloadWeapon();
	void StopReload();

	/** Set New Camera location manager based on ACSCharacter CMLocation */
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	
	void PlayFireEffects();

	// Stop All Action like Reload, Shoot, Aim
	void StopAllActions();

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float RemotePitch;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MyMovement")
	class UMyCharacterMovementComponent* GetMyMovement();

	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	FORCEINLINE void SetCharacterMovement(float NewSpeed) {
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool isAiming() { return bAiming; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool isAlive() { return !bDeath; }

	//Getter for mesh
	FORCEINLINE class USkeletalMeshComponent* GetTPP_Mesh() const { return GetMesh(); }
	FORCEINLINE class USkeletalMeshComponent* GetFP_Mesh() const { return FP_Mesh; }

	//Gettter for weapon 
	FORCEINLINE class USkeletalMeshComponent* GetTPP_Weapon() { return TPP_Weapon; }
	FORCEINLINE class USkeletalMeshComponent* GetFP_Weapon() { return TPP_Weapon; }

	UFUNCTION(BlueprintPure, Category = "Camera|CameraManager")
	FORCEINLINE FVector GetCameraLocation() { return CMLocation->GetComponentLocation(); }

public:
	/** Public variables */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSocket");
	FName HeadSocketName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Locomotion|States")
	bool bDeath;

protected:
	/** Protected functions */

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);

	// StartFire on current weapon 
	void StartFire();

	// StartFire on server 
	UFUNCTION(Server, Reliable)
	void Server_StartFire();

	// StopFire on current weapon
	void StopFire();

	// StopFire on server 
	UFUNCTION(Server, Reliable)
	void Server_StopFire();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayShootMontages();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ADSSystem")
	void ADS(bool bWantAim);

	/** Calc new location for FP_Mesh */
	/*UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ADSSystem")
	FTransform NewHandPosition(FTransform DesiredPosition, USceneComponent* WeaponOwner, FVector WeaponSocketLocation);*/

	/** Run implementation */
	void StartRun();

	void StopRun();

	/** Crouch implementation */
	void StartCrouch();

	void StopCrouch();

	/** Set new size to our capsule component */
	UFUNCTION()
	void OnRep_ChangeCapsule();

	/** Aim implementation */
	void StartAim();

	void StopAim();

	/** Jump implementation */
	void StartJump();
	void StopJump();

	/** Apply damage */
	UFUNCTION()
	void OnHealthChanged(UHealthStats* HealthComponent, float Health, float Damage,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/** Death implementation */
	//void EnableDeathCamera(bool isDeath);

	/** Server Death implementation */
	UFUNCTION(Server, Reliable)
	void Server_Death();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Death();

	/** When we start action like run, crouch, aim. We change movement speed on server and client */
	UFUNCTION(Server, Reliable)
	void ChangeMovement();

	/** Change Movement on the client */
	UFUNCTION()
	void OnRep_ChangeMovement();

	/** Reload on server imlementation */
	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReloadMontage();

	/** Toggle visibity on Mesh */
	void ShowTPP_Mesh(bool bNewOwnerVisible);

	/** Toggle visibility on FP_Mesh */
	void ShowFP_Mesh(bool bNewOwnerVisible);

protected:
	/** Protected variables */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class ACSWeapon> WeaponClass;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	class ACSWeapon* CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocket;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
	class USoundBase* AimingSound;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float AimSpeed;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|States")
	EGroundState CurrentMovementState;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion|States")
	EOverlayState CurrentOverlayState;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|PlayerAction")
	float DefaultMovementSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float RunSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bCrouch;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeCapsule)
	bool bNetCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float CrouchSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bOverlayStates;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|PlayerAction")
	float OverlaySpeedUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float OverlaySpeedDown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float CrouchedHalfHeightUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float CrouchedHalfHeightDown;

	UPROPERTY(BlueprintReadWrite, Category = "ADSSystem")
	float ADSAlpha;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADSSystem")
	float ADSTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Montages")
	class UAnimMontage* Recoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Montages")
	class UAnimMontage* Reload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Montages")
	class UAnimMontage* TPPReload;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Montages")
	class UAnimMontage* TPP_FireWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Montages")
	class UAnimMontage* TPP_Fire;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* TPP_FireSound;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* TPP_FireEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	float PreviosSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bShoot;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|PlayerAction")
	bool bReload;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|PlayerAction")
	float ReloadTime;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeMovement)
	float NewMovementSpeed;
	

private:
	
};
