// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSWeapon.generated.h"


USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class CSMAP_API ACSWeapon : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties

	ACSWeapon();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USceneComponent* RootComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCapsuleComponent* WeaponCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USkeletalMeshComponent* WeaponSKMesh;

public:
	/** Public functions */

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	/** Shoot implementation */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Shoot")
	void Fire(); 

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Client, Reliable)
	void StartFire();

	UFUNCTION(Client, Reliable)
	void StopFire();

	// Reload Weapon 
	void Reload();

	/** Getter and Setter for ammo */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int GetCurrentAmmo() { return CurrentAmmo; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int GetMaxAmmo() { return MaxAmmo; }

	FORCEINLINE UAnimMontage* GetWeaponShootMontage() { return Rifle_Fire; }

	UFUNCTION(BlueprintPure)
	bool isWantReload();

protected:
	/** Protected functions */

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Degrees Current Ammo 
	void ReduceAmmo();

	/** Recoil implementation */
	// Recoil Pattern. Get Recoil image in Data table.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon|Properties")
	void AddRecoil(FVector2D const RecoilForce, AController* OwnerController);

	/** Get owner controller and add offset to input */
	void AddRecoilForce(AController* OwnerController);

	void ResetRecoilForce();

	/** Spawn Decal on shoot impact. Set in blueprints. */
	UFUNCTION(Category = "FX|Decals")
	void SpawnImpactDecal(class UMaterialInterface* DecalMat, class USceneComponent* AttachedComp, const EPhysicalSurface PhysSurface, 
		const FVector& ImpactNormal, const FVector& ImpactLocation);

	/** Spawn FX when we shoot. */
	void PlayFireEffects(FVector TraceEnd);

	/** Spawn FX on shoot impact. */
	void SpawnImpactEffects(EPhysicalSurface SurfaceType, const FVector& ImpactPoint);

	void SpawnImpactEffect(const FHitResult& Impact);

	UFUNCTION()
	void OnRep_HitScanTrace();

protected:
	/** Protected variables */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties") 
	class UMaterialInterface* ImpactShootDecal;

	FTimerHandle FireTimerHandle;

	/** Shoot Delay Properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	float FireRate;
	float DelayBetweenShots;
	float LastShot;

	/** RecoilTime calc based on FireRate and LastShot */
	FTimerHandle RecoilTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Properties")
	TMap<int, FVector2D> RecoilImage;

	FHitResult ImpactHit;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 AmmoMagazine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	FName WeaponName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	FName TargetTracerName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	class UParticleSystem* FlashImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	class UParticleSystem* TracerEffect;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animations")
	class UAnimMontage* Rifle_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Weapon|Effects")
	TSubclassOf<UCameraShake> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Properties")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace ValueTrace;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SoundEffects")
	class USoundBase* FireSound;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SoundEffects")
	class USoundBase* ReloadStartSound;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SoundEffects")
	class USoundBase* ReloadEndSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|FX")
	TSubclassOf<class ACSImpactEffect> ImpactTemplate;

private:
	float ShootCount;
};
