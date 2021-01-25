// Fill out your copyright notice in the Description page of Project Settings.


#include "CSCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CSWeapon.h"
#include "HealthStats.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CSMap.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "HealthStats.h"
#include "Net/UnrealNetwork.h"
#include "CSPlayerController.h"
#include "CSMapGameModeBase.h"
#include "MyCharacterMovementComponent.h"

// Sets default values
ACSCharacter::ACSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>
		(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*TPP_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPPCameraComponent"));
	TPP_Camera->SetupAttachment(GetMesh());*/

	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSKHands"));
	FP_Mesh->SetupAttachment(GetRootComponent());
	FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FP_Mesh->bOnlyOwnerSee = true; 

	TPP_Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKTPPWeapon"));
	TPP_Weapon->SetupAttachment(GetMesh(), FName("GripPoint"));
	TPP_Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TPP_Weapon->bOwnerNoSee = true;

	WeaponCameraLoc = CreateDefaultSubobject<USceneComponent>(TEXT("FPSCWeaponCameraLocation"));
	WeaponCameraLoc->SetupAttachment(GetRootComponent());
	WeaponCameraLoc->SetRelativeLocation(FVector(13.571661f, 0.000137f, -0.000008f));

	CMLocation = CreateDefaultSubobject<USceneComponent>(TEXT("CMLocation"));
	CMLocation->SetupAttachment(GetRootComponent());

	WeaponStartLoc = CreateDefaultSubobject<USceneComponent>(TEXT("FPSCWeaponStartLocation"));
	WeaponStartLoc->SetupAttachment(GetRootComponent());
	WeaponStartLoc->SetRelativeLocation(FVector(-5.029604f, 14.372208f, -152.863449f));
	WeaponStartLoc->SetRelativeRotation(FRotator(0.f, 0.f, -20.f));
	HealthStat = CreateDefaultSubobject<UHealthStats>(TEXT("HealthStat"));

	WeaponSocket = "GripPoint";

	HeadSocketName = "FPViewPoint";

	CurrentMovementState = EGroundState::Transient;

	ADSTime = 0.2f;
	OverlaySpeedUp = 250.f;
	OverlaySpeedDown = 250.f;
	CrouchSpeed = 300.f;
	RunSpeed = 800.f;
	AimSpeed = 250.f;
	DefaultMovementSpeed = 600.f; 

	bDeath = false;
	

	CurrentMovementState = EGroundState::Transient;
	CurrentOverlayState = EOverlayState::Transient;

	SetReplicates(true);
	SetReplicateMovement(true); 
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthStat->OnHealthChanged.AddDynamic(this, &ACSCharacter::OnHealthChanged);
	if (GetLocalRole() == ROLE_Authority)
	{
		if (WeaponClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			CurrentWeapon = GetWorld()->SpawnActor<ACSWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (CurrentWeapon)
			{
				CurrentWeapon->AttachToComponent(FP_Mesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), WeaponSocket);
				TPP_Weapon = CurrentWeapon->WeaponSKMesh; 
			}
		}
	}
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsLocallyControlled())
	{
		FRotator Rot = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraRotation();
		Rot.Pitch = RemoteViewPitch * 360.f / 255.f;
		RemotePitch = Rot.Pitch;
	}
}

// Called to bind functionality to input
void ACSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Move
	PlayerInputComponent->BindAxis("MoveForward", this, &ACSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSCharacter::MoveRight);
	// Controller Axis
	PlayerInputComponent->BindAxis("Turn", this, &ACSCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACSCharacter::LookUp);
	// Actions 
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ACSCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ACSCharacter::StopFire);

	PlayerInputComponent->BindAction("Run", EInputEvent::IE_Pressed, this, &ACSCharacter::StartRun);
	PlayerInputComponent->BindAction("Run", EInputEvent::IE_Released, this, &ACSCharacter::StopRun);

	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ACSCharacter::StartAim);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ACSCharacter::StopAim);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACSCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACSCharacter::StopJump);

	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ACSCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &ACSCharacter::StopCrouch);

	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ACSCharacter::StartReload);
}

void ACSCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);
	if (isAlive())
	{
		OutResult.Location = GetCameraLocation();
		OutResult.Rotation = GetControlRotation();
	}
	else
	{
		Super::CalcCamera(DeltaTime, OutResult);
	}
}

void ACSCharacter::Server_PlayShootMontages_Implementation()
{
	Multicast_PlayShootMontages();
}

void ACSCharacter::Multicast_PlayShootMontages_Implementation()
{
	if (CurrentWeapon && isAlive())
	{
		//GetFP_Mesh()->GetAnimInstance()->Montage_Play(Recoil);
		GetTPP_Mesh()->GetAnimInstance()->Montage_Play(TPP_FireWeapon);
		GetTPP_Weapon()->GetAnimInstance()->Montage_Play(TPP_FireWeapon);
	}
}

void ACSCharacter::PlayFireEffects()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TPP_FireEffect,
		GetTPP_Weapon()->GetSocketLocation(WeaponSocket), FRotator::ZeroRotator);
}

void ACSCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), FMath::Clamp(Value, -1.f, 1.f));
}

void ACSCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), FMath::Clamp(Value, -1.f, 1.f));
}

void ACSCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * -1.f);
}

void ACSCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ACSCharacter::StartFire()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		Server_StartFire();
	}
	if (CurrentOverlayState == EOverlayState::Reload)
	{
		return;
	}
	
	if (CurrentWeapon)
	{
		CurrentOverlayState = EOverlayState::Fire;
		ChangeMovement();
		CurrentWeapon->StartFire();
	}
}

void ACSCharacter::Server_StartFire_Implementation()
{
	StartFire();
}

void ACSCharacter::StopFire()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		Server_StopFire();
	}
	if (CurrentWeapon)
	{
		if (CurrentOverlayState != EOverlayState::Reload)
		{
			CurrentOverlayState = EOverlayState::Transient;
			ChangeMovement();
			CurrentWeapon->StopFire();
		}
	}
}

void ACSCharacter::Server_StopFire_Implementation()
{
	StopFire();
}

//FTransform ACSCharacter::NewHandPosition(FTransform DesiredPosition, USceneComponent* WeaponOwner, FVector WeaponSocketLocation)
//{
//	FVector DesiredLocation = DesiredPosition.GetLocation();
//	
//
//	FRotator DesiredRotation = DesiredPosition.Rotator();
//	FQuat DesireQuat = DesiredPosition.GetRotation();
//	FVector OwnerLocation  = WeaponOwner->GetComponentLocation();
//
//	FVector Distance = FVector(DesiredLocation - WeaponSocketLocation);
//	FVector DesiredOffset = Distance + OwnerLocation;
//
//	FRotator test = FRotator(DesiredRotation);
//	FTransform Result = FTransform(FQuat(DesireQuat), FVector(DesiredOffset), FVector::OneVector);
//
//	return Result;
//}

void ACSCharacter::StartRun()
{
	GetMyMovement()->SetSprint(true);
}

void ACSCharacter::StopRun()
{
	GetMyMovement()->SetSprint(false);
}

void ACSCharacter::StartCrouch()
{
	Crouch();
}

void ACSCharacter::StopCrouch()
{
	UnCrouch();
}

void ACSCharacter::OnRep_ChangeCapsule()
{
	//UpdateHalfHeight(bNetCrouch);
}

void ACSCharacter::StartAim()
{
	if (CurrentWeapon && isAlive())
	{
		/*if (GetLocalRole() != ROLE_Authority)
		{
			Server_StartAim();
		}*/
		if (CurrentOverlayState == EOverlayState::Reload)
		{
			return;
		}
		UGameplayStatics::SpawnSound2D(this, AimingSound, 0.4f);
		bAiming = true;
		/*ChangeMovement();*/
		ADS(bAiming);
	}
}

void ACSCharacter::StopAim()
{
	if (CurrentWeapon && isAlive())
	{
		/*if (GetLocalRole() != ROLE_Authority)
		{
			Server_StopAim();
		}*/
		UGameplayStatics::SpawnSound2D(this, AimingSound, 0.4f);
		bAiming = false;
		ChangeMovement();
		if (CurrentOverlayState == EOverlayState::Reload)
		{
			return;
		}
		ADS(bAiming);
	}
}

void ACSCharacter::StartJump()
{
	Jump();
}

void ACSCharacter::StopJump()
{
	StopJumping();
}

void ACSCharacter::OnHealthChanged(UHealthStats* HealthComponent, float Health, float Damage, 
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	ACSPlayerController* MyPC = Cast<ACSPlayerController>(GetController());
	if (MyPC)
	{
		MyPC->UpdateHealth(Health);
	}
	if (Health <= 0.f && !bDeath)
	{
		bDeath = true;
		Server_Death();
	}
}

void ACSCharacter::Multicast_Death_Implementation()
{
	ShowFP_Mesh(false);
	ShowTPP_Mesh(true);

	if(GetController())
	{
		ACSPlayerController* MyPC = Cast<ACSPlayerController>(GetController());
		if (MyPC->IsLocalController())
		{
			DisableInput(MyPC);
		}
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		ACSMapGameModeBase* MyGM = GetWorld()->GetAuthGameMode<ACSMapGameModeBase>();
		if (MyGM)
		{
			MyGM->PlayerDeath(this);
		}
	}

	GetCharacterMovement()->GravityScale = 0.f;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//EnableDeathCamera(true);
	SetLifeSpan(2.f);
	
}

void ACSCharacter::StartReload()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		Server_Reload();
	}
	if (CurrentOverlayState == EOverlayState::Reload)
	{
		return;
	}
	if (CurrentWeapon && isAlive())
	{
		if (CurrentWeapon->isWantReload())
		{
			CurrentWeapon->StopFire();
			if (bAiming)
			{
				ADS(!bAiming);
			}
			CurrentOverlayState = EOverlayState::Reload;

			ChangeMovement();

			Multicast_ReloadMontage();
		}
		/*if (GetLocalRole() != ROLE_Authority)
		{
			Server_Reload();
		}
		else
		{
			Multicast_ReloadMontage();
		}*/
	}
}

void ACSCharacter::Server_Reload_Implementation()
{
	StartReload();
	/*if (CurrentWeapon->isWantReload())
	{
		Multicast_ReloadMontage();
	}*/
}

void ACSCharacter::ReloadWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
		StopReload();
	}
}

void ACSCharacter::StopReload()
{
	CurrentOverlayState = EOverlayState::Transient;
	if (bAiming)
	{
		ADS(bAiming);
	}
	ChangeMovement(); 
	FP_Mesh->GetAnimInstance()->Montage_Stop(0.1f, Reload);
}

void ACSCharacter::StopAllActions()
{
	StopFire();
	StopReload();
	StopAim();
}

UMyCharacterMovementComponent* ACSCharacter::GetMyMovement()
{
	UMyCharacterMovementComponent* MyMovement = Cast<UMyCharacterMovementComponent>(GetCharacterMovement());
	return MyMovement;
}

void ACSCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("FPSKHands")));
	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->GetRelativeRotation(), DefMesh1P->GetRelativeLocation());
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	FP_Mesh->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}

void ACSCharacter::ChangeMovement_Implementation()
{
	float NewMovement = 0.f;
	if (CurrentOverlayState == EOverlayState::Transient && !bAiming)
	{
		switch (CurrentMovementState)
		{
		case EGroundState::Crouch:
			NewMovement = CrouchSpeed;
			break;
		case EGroundState::Run:
			NewMovement = RunSpeed;
			break;
		case EGroundState::Transient:
			NewMovement = DefaultMovementSpeed;
			break;
		}
	}
	else
	{
		NewMovement = OverlaySpeedUp;
	}
	SetCharacterMovement(NewMovement);
	NewMovementSpeed = NewMovement;
}

void ACSCharacter::OnRep_ChangeMovement()
{
	SetCharacterMovement(NewMovementSpeed);
}

void ACSCharacter::Multicast_ReloadMontage_Implementation()
{
	if (CurrentWeapon && isAlive())
	{
		FP_Mesh->GetAnimInstance()->StopAllMontages(0.1f);

		GetMesh()->GetAnimInstance()->Montage_Play(TPPReload);

		ReloadTime = FP_Mesh->GetAnimInstance()->Montage_Play(Reload);
		FTimerHandle ReloadHandle;

		GetWorldTimerManager().SetTimer(ReloadHandle, this, &ACSCharacter::ReloadWeapon, 1.f, false, ReloadTime);
	}
}

void ACSCharacter::Server_Death_Implementation()
{
	Multicast_Death();
}

//void ACSCharacter::EnableDeathCamera(bool isDeath)
//{
//	FP_Camera->SetActive(!isDeath, false);
//	TPP_Camera->SetActive(isDeath, false);
//}

void ACSCharacter::ShowTPP_Mesh(bool bNewOwnerVisible)
{
	GetMesh()->SetOwnerNoSee(!bNewOwnerVisible);
	GetTPP_Weapon()->SetOwnerNoSee(!bNewOwnerVisible);
}

void ACSCharacter::ShowFP_Mesh(bool bNewOwnerVisible)
{
	FP_Mesh->SetOwnerNoSee(!bNewOwnerVisible);
	FP_Mesh->SetVisibility(bNewOwnerVisible, true);
	CurrentWeapon->WeaponSKMesh->SetOwnerNoSee(!bNewOwnerVisible);
	CurrentWeapon->WeaponSKMesh->SetVisibility(bNewOwnerVisible, true);
}


void ACSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSCharacter, CurrentWeapon);
	DOREPLIFETIME(ACSCharacter, bDeath);
	DOREPLIFETIME(ACSCharacter, bNetCrouch);
	DOREPLIFETIME(ACSCharacter, CurrentMovementState);
	DOREPLIFETIME(ACSCharacter, RemotePitch);
	DOREPLIFETIME(ACSCharacter, CurrentOverlayState);
	DOREPLIFETIME(ACSCharacter, NewMovementSpeed);
}