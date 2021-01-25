// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSImpactEffect.generated.h"

UCLASS()
class CSMAP_API ACSImpactEffect : public AActor
{
	GENERATED_BODY()

	ACSImpactEffect();
public: 
    /** spawn effect */
	virtual void PostInitializeComponents() override;

	FORCEINLINE void SetSurfaceResult(const FHitResult& ImpactHit) { SurfaceHit = ImpactHit; }
private:
	/** default impact FX used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults", meta=(AllowPrivateAccess = "true"))
	class UParticleSystem* DefaultFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ParticleEffects", meta=(AllowPrivateAccess = "true"))
	class UParticleSystem* ConcreteFX; 

	/** impact FX on concrete */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults", meta = (AllowPrivateAccess = "true"))
	class USoundBase* DefaultSound;

	/** impact sound on concrete */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SoundEffects", meta = (AllowPrivateAccess = "true"))
	class USoundBase* ConcreteSound;

	/** Surface type data */
	UPROPERTY(BlueprintReadOnly, Category = "Surface", meta = (AllowPrivateAccess = "true"))
	FHitResult SurfaceHit;

protected:

	/** get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** get sound for material type */
	USoundBase* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

};
