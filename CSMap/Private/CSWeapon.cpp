// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CSCharacter.h"
#include "CSMap.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DecalComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "CSImpactEffect.h"

//#define DEBUG

// Sets default values
ACSWeapon::ACSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(RootComp);

	WeaponSKMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMeshComponent"));
	WeaponSKMesh->SetupAttachment(GetRootComponent());
	WeaponSKMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponSKMesh->bOnlyOwnerSee = true; 

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollisionComponent"));
	WeaponCollision->SetupAttachment(GetRootComponent());

	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	LastShot = 0.f;

	SetReplicates(true);
	FireRate = 600.f;
	MaxAmmo = 60;
	WeaponName = "R-303";
	ShootCount = 0;

	TargetTracerName = "BeamEnd";

	MuzzleSocketName = "MuzzleFlash";
	AmmoMagazine = 30; 

	Damage = 30.f;
}

// Called when the game starts or when spawned
void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();
	DelayBetweenShots = 60.f / FireRate;
	CurrentAmmo = AmmoMagazine;
}


void ACSWeapon::ReduceAmmo()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentAmmo = FMath::Max(--CurrentAmmo, 0);
	}
}

void ACSWeapon::AddRecoilForce(AController* OwnerController)
{
	ACSCharacter* MyOwner = Cast<ACSCharacter>(GetOwner());
	if (MyOwner)
	{
		bool bRecoilPattern = MyOwner->isAiming();
		if (bRecoilPattern)
		{
			ShootCount++;
			if (RecoilImage.Contains(ShootCount))
			{
				MyOwner->GetController();
				const FVector2D RecoilForce = RecoilImage.FindRef(ShootCount);
				if (OwnerController)
				{
					AddRecoil(RecoilForce, OwnerController);
				}
			}
		}
		else
		{
			if (OwnerController)
			{
				const FVector2D RecoilForce = FVector2D(FMath::RandRange(-0.1f, 0.1f), FMath::RandRange(-0.1f, 0.1f));
				AddRecoil(RecoilForce, OwnerController);
			}
		}
	}
}

void ACSWeapon::ResetRecoilForce()
{
	ShootCount = 0;
}

void ACSWeapon::SpawnImpactDecal(class UMaterialInterface* DecalMat, class USceneComponent* AttachedComp, const EPhysicalSurface PhysSurface,
	const FVector& ImpactNormal, const FVector& ImpactLocation)
{
	FRotator ImpactRot = FRotationMatrix::MakeFromX(ImpactNormal * -1).Rotator();
	FRotator DecalRot = FRotator(ImpactRot.Pitch, ImpactRot.Yaw, FMath::RandRange(0.f, 360.f));

	UMaterialInterface* SelectedDecal = nullptr;
	switch (PhysSurface)
	{
	case SURFACE_DEFAULT:
	case SURFACE_CONCRETE:
		SelectedDecal = ImpactShootDecal;
		break;
	}
	if (SelectedDecal)
	{
		UDecalComponent* SpawnedDecal = UGameplayStatics::SpawnDecalAttached(SelectedDecal, FVector(3.f), AttachedComp,
			FName("None"), ImpactLocation, DecalRot, EAttachLocation::KeepWorldPosition);
		if (SpawnedDecal)
		{
			SpawnedDecal->SetFadeScreenSize(0.f);
		}
	}
}

void ACSWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (TracerEffect)
	{
		FVector MuzzleLocation = WeaponSKMesh->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* SpawnedEffect = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (SpawnedEffect)
		{
			SpawnedEffect->SetVectorParameter(TargetTracerName, TraceEnd);
		}
	}
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* MyPlayerController = Cast<APlayerController>(MyOwner->GetController());
		if (MyPlayerController)
		{
			MyPlayerController->ClientPlayCameraShake(CameraShake);
		}
	}
}

void ACSWeapon::SpawnImpactEffects(EPhysicalSurface SurfaceType, const FVector& ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_DEFAULT:
	case SURFACE_CONCRETE:
		SelectedEffect = FlashImpactEffect;
		break;
	}
	if (SelectedEffect)
	{
		FVector Direction = ImpactPoint - WeaponSKMesh->GetSocketLocation(MuzzleSocketName);
		Direction.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, Direction.Rotation());
	}
}

void ACSWeapon::SpawnImpactEffect(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UsedHit = Impact;
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);
		ACSImpactEffect* SpawnedEffect = GetWorld()->SpawnActorDeferred<ACSImpactEffect>(ImpactTemplate, SpawnTransform, this);
		if (SpawnedEffect)
		{
			SpawnedEffect->SetSurfaceResult(UsedHit);
			UGameplayStatics::FinishSpawningActor(SpawnedEffect, SpawnTransform);
		}
	}
}

// Called every frame
void ACSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACSWeapon::Fire()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		ServerFire();
	}

	ACSCharacter* MyOwner = Cast<ACSCharacter>(GetOwner());

	if (MyOwner && MyOwner->isAlive())
	{
		
		if (CurrentAmmo > 0)
		{
			FHitResult Hit;
			ReduceAmmo();
			MyOwner->Server_PlayShootMontages();
			FVector StartLocation;
			FRotator Direction;

			APlayerCameraManager* MyCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
			StartLocation = MyCameraManager->GetCameraLocation();
			Direction = MyCameraManager->GetCameraRotation();

			FVector EndLocation = StartLocation + (Direction.Vector() * 10000.f);
			float Accuracy = MyOwner->isAiming() ? 0.f : 1.5f;
			Direction += FRotator(FMath::RandRange(-Accuracy, Accuracy), FMath::RandRange(-Accuracy, Accuracy), 0.f);


			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.bReturnPhysicalMaterial = true;
			bool bBlock = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, WEAPON_TRACE, QueryParams);
			if (bBlock)
			{
				EndLocation = Hit.ImpactPoint;

				AActor* HitActor = Hit.GetActor();
				if (HitActor)
				{
					UGameplayStatics::ApplyPointDamage(HitActor, Damage, Hit.ImpactNormal,
						Hit, MyOwner->GetController(), MyOwner, DamageType);
					//UGameplayStatics::ApplyPointDamage(HitActor, Damage, MyOwner->GetController(), MyOwner, DamageType);
				}

				EPhysicalSurface CurrentSurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				SpawnImpactDecal(ImpactShootDecal, Hit.GetComponent(), CurrentSurfaceType, Hit.ImpactNormal, EndLocation);
				//SpawnImpactEffects(CurrentSurfaceType, EndLocation);
				ImpactHit = Hit;
				SpawnImpactEffect(ImpactHit);
				
				if (GetLocalRole() == ROLE_Authority)
				{
					ValueTrace.TraceTo = Hit.Location;
				}
			}
			PlayFireEffects(EndLocation);
			LastShot = GetWorld()->TimeSeconds;
			if (Rifle_Fire)
			{
				WeaponSKMesh->GetAnimInstance()->Montage_Play(Rifle_Fire, 1.1f);
			}
			
			AddRecoilForce(MyOwner->GetController());
#ifdef DEBUG
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.f, 0, 5.f);
#endif // DEBUG
		}
		else
		{
			MyOwner->StartReload();
		}
	}
	else
	{
		StopFire();
	}
}

void ACSWeapon::StartFire_Implementation()
{
	float FirstDelay = FMath::Max(LastShot + DelayBetweenShots - GetWorld()->TimeSeconds, 0.f);
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ACSWeapon::Fire, DelayBetweenShots, true, FirstDelay);
}

void ACSWeapon::StopFire_Implementation()
{
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	ResetRecoilForce();
}

void ACSWeapon::Reload()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GetOwner())
		{
			if (AmmoMagazine == CurrentAmmo)
			{
				return;
			}

			int32 Substract = CurrentAmmo - AmmoMagazine;
			if (MaxAmmo >= FMath::Abs(Substract))
			{
				MaxAmmo = FMath::Max(MaxAmmo + Substract, 0);
				CurrentAmmo = FMath::Max(CurrentAmmo - Substract, 0);
			}
			else
			{
				CurrentAmmo += MaxAmmo;
				MaxAmmo = 0;
			}
		}
	}
}

void ACSWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(ValueTrace.TraceTo);
	SpawnImpactEffect(ImpactHit);
}

void ACSWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ACSWeapon::ServerFire_Validate()
{
	return true;
}

bool ACSWeapon::isWantReload()
{
	if (GetLocalRole() == ROLE_Authority) 
	{ 
		return AmmoMagazine != CurrentAmmo && MaxAmmo != 0;
	}
	return false;
}

void ACSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACSWeapon, ValueTrace, COND_SkipOwner);
	DOREPLIFETIME(ACSWeapon, CurrentAmmo);
	DOREPLIFETIME(ACSWeapon, MaxAmmo);
	DOREPLIFETIME(ACSWeapon, AmmoMagazine);
	DOREPLIFETIME(ACSWeapon, FireSound);
	DOREPLIFETIME(ACSWeapon, ReloadStartSound);
	DOREPLIFETIME(ACSWeapon, ReloadEndSound);
}
