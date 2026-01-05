#include "BoidsManager.h"
#include "BoidComponent.h"

ABoidsManager::ABoidsManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoidsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Center = GetActorLocation();
	DrawDebugBox(GetWorld(), Center, Bounds, FColor::Green, false, -1.0f, 0, 2.0f);

	TArray<ABoidComponent*> validBoids = GetValidBoids();

	for (TWeakObjectPtr<ABoidComponent> BoidPtr : Boids)
	{
		if (ABoidComponent* Boid = BoidPtr.Get())  // Check if still valid
		{
			Boid->UpdateBoid(validBoids, DeltaTime, Center);
		}
	}
}

void ABoidsManager::BeginPlay()
{
	Super::BeginPlay();

	if (BoidClass)
		SpawnBoids();
}

TArray<ABoidComponent*> ABoidsManager::GetValidBoids() const
{
	TArray<ABoidComponent*> ValidBoids;
	ValidBoids.Reserve(Boids.Num());  // Pre-allocate for efficiency

	for (const TWeakObjectPtr<ABoidComponent>& boidPtr : Boids)
	{
		// Convert weak pointer to raw pointer if still valid
		if (ABoidComponent* boid = boidPtr.Get())
		{
			ValidBoids.Add(boid);
		}
	}

	return ValidBoids;
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
		{
			if (i == 0) {
				newBoid->SetBoidMaterial(BoidMaterial);
			}
				
			newBoid->SetBoundaryBox(Bounds);
			Boids.Add(newBoid);
		}
	}
}
