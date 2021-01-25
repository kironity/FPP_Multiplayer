// Fill out your copyright notice in the Description page of Project Settings.


#include "CSBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ACSBaseCharacter::ACSBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->bOwnerNoSee = true;

	Mesh1p = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1p"));
	Mesh1p->SetupAttachment(GetRootComponent());
	Mesh1p->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1p->bOnlyOwnerSee = true;

	CMLocation = CreateDefaultSubobject<USceneComponent>(TEXT("CMLocation"));
	CMLocation->SetupAttachment(GetRootComponent());
	CMLocation->SetRelativeLocation(FVector(10.f, 0.f, 85.f));

}

// Called when the game starts or when spawned
void ACSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Axis Move
	PlayerInputComponent->BindAxis("MoveForward", this, &ACSBaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSBaseCharacter::MoveRight);

	// Controller Axis
	PlayerInputComponent->BindAxis("LookUp", this, &ACSBaseCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ACSBaseCharacter::Turn);

}

void ACSBaseCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), FMath::Clamp(Value, -1.f, 1.f));
}

void ACSBaseCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), FMath::Clamp(Value, -1.f, 1.f));
	GetCameraLocation();
}

void ACSBaseCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * -1.f);
}

void ACSBaseCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ACSBaseCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("Mesh1p")));
	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->GetRelativeRotation(), DefMesh1P->GetRelativeLocation());
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	Mesh1p->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}