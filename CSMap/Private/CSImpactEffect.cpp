// Fill out your copyright notice in the Description page of Project Settings.

//
#include "CSImpactEffect.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "CSMap.h"

// Sets default values
ACSImpactEffect::ACSImpactEffect()
{
	SetAutoDestroyWhenFinished(true);
}

void ACSImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UPhysicalMaterial* PhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface CurrentSurface = UPhysicalMaterial::DetermineSurfaceType(PhysMat);

	UParticleSystem* ImpactFX = GetImpactFX(CurrentSurface);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	USoundBase* ImpactSound = GetImpactSound(CurrentSurface);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

UParticleSystem* ACSImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* ImpactFX = nullptr;

	switch (SurfaceType)
	{
	case SURFACE_CONCRETE:  ImpactFX = ConcreteFX; break;
	default: ImpactFX = DefaultFX; break;
	}
	return ImpactFX;
}

USoundBase* ACSImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundBase* ImpactSound = nullptr;
	switch(SurfaceType)
	{
	case SURFACE_CONCRETE: ImpactSound = ConcreteSound; break;
	default: ImpactSound = DefaultSound; break; 
	}
	return ImpactSound;
}

