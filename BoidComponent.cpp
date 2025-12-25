#include "BoidComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h" // Needed for ConstructorHelpers

ABoidComponent::ABoidComponent()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    // Set default cone shape
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
    if (ConeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(ConeMesh.Object);
    }

    Mesh->SetRelativeScale3D(FVector(0.5f));

    Velocity = FVector(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1), FMath::RandRange(-1, 1)).GetSafeNormal() * MaxSpeed;
}

void ABoidComponent::BeginPlay()
{
    Super::BeginPlay();
}

void ABoidComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector newPos = GetActorLocation() + Velocity * DeltaTime;
    SetActorLocation(newPos);

    if (!Velocity.IsNearlyZero()) 
    {
        FRotator newRotation = Velocity.Rotation();
        SetActorRotation(newRotation);
    }
}

void ABoidComponent::UpdateBoid(const TArray<class ABoidComponent*>& AllBoids, float DeltaTime)
{
    TArray<ABoidComponent*> nearBoids;

    for (ABoidComponent* aBoid : AllBoids) 
    {
        if (aBoid == this) 
            continue;

        float distance = FVector::Dist(GetActorLocation(), aBoid->GetActorLocation());
        if (distance < PerceptionRadius)
            nearBoids.Add(aBoid);
    }

    if (nearBoids.Num() == 0)
        return;

    FVector separationForce = Separation(nearBoids) * SeparationWeight;
    FVector alignmentForce = Alignment(nearBoids) * AlignmentWeight;
    FVector cohesionForce = Cohesion(nearBoids) * CohesionWeight;

    Velocity += separationForce + alignmentForce + cohesionForce;
    Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
}

FVector ABoidComponent::Separation(const TArray<ABoidComponent*>& Boids)
{
    FVector steering = FVector::ZeroVector;

    for (ABoidComponent* boid : Boids) 
    {
        FVector diff = GetActorLocation() - boid->GetActorLocation();
        float distance = diff.Size();

        if (distance > 0)
            steering += diff.GetSafeNormal() / distance;
    }

    return steering.GetSafeNormal();
}

FVector ABoidComponent::Alignment(const TArray<ABoidComponent*>& Boids)
{
    if (Boids.Num() == 0)
        return FVector::ZeroVector;

    FVector averageVelocity = FVector::ZeroVector;

    for (ABoidComponent* boid : Boids) 
    {
        averageVelocity += boid->Velocity;
    }

    averageVelocity /= Boids.Num();

    return (averageVelocity - Velocity).GetSafeNormal();
}

FVector ABoidComponent::Cohesion(const TArray<ABoidComponent*>& Boids)
{
    if (Boids.Num() == 0)
        return FVector::ZeroVector;

    FVector centerOfMass = FVector::ZeroVector;

    for (ABoidComponent* boid : Boids)
    {
        centerOfMass += boid->GetActorLocation();
    }

    centerOfMass /= Boids.Num();

    return (centerOfMass - GetActorLocation()).GetSafeNormal();
}
