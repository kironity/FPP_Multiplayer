// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthStats.h"
#include "CSCharacter.h"

// Sets default values for this component's properties
UHealthStats::UHealthStats()
{
	MaxHealth = 100;
	Health = 100;
}


// Called when the game starts
void UHealthStats::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	if (GetOwner())
	{
		AActor* MyOwner = GetOwner();
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthStats::HandleTakeAnyDamage);
	}
}

void UHealthStats::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f)
	{
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}


// Called every frame
void UHealthStats::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthStats::ResetHealth()
{
	Health = MaxHealth;
}

