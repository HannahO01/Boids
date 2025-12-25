#include "BoidsManager.h"
#include "BoidComponent.h"

ABoidsManager::ABoidsManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoidsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (ABoidComponent* boid : Boids) 
	{
		if (boid)
			boid->UpdateBoid(Boids, DeltaTime);
	}
}

void ABoidsManager::BeginPlay()
{
	Super::BeginPlay();

	if (BoidClass)
		SpawnBoids();
}

void ABoidsManager::SpawnBoids()
{
	Boids.Empty();

	for (int32 i = 0; i < BoidCount; i++)
	{
		FVector spawnLocation = GetActorLocation() +
			FVector(FMath::RandRange(-SpawnBounds.X, SpawnBounds.X),
					FMath::RandRange(-SpawnBounds.Y, SpawnBounds.Y),
					FMath::RandRange(-SpawnBounds.Z, SpawnBounds.Z));

		FRotator spawnRotation = FRotator(0, FMath::RandRange(0, 360), 0);

		ABoidComponent* newBoid = GetWorld()->SpawnActor<ABoidComponent>(BoidClass, spawnLocation, spawnRotation);
		if (newBoid)
			Boids.Add(newBoid);
	}
}
