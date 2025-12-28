#include "BoidComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h" // Needed for ConstructorHelpers

ABoidComponent::ABoidComponent()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Mesh->SetSimulatePhysics(false);
    Mesh->SetEnableGravity(false);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Set default cone shape
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
    if (ConeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(ConeMesh.Object);
    }

    Mesh->SetRelativeScale3D(FVector(0.2f));
    Mesh->SetRelativeRotation(FRotator(0, 0, -90));

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

void ABoidComponent::UpdateBoid(const TArray<class ABoidComponent*>& AllBoids, float DeltaTime, FVector Center)
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
    FVector cohesionForce = Cohesion(nearBoids, Center) * CohesionWeight;
    FVector boundaryForce = ApplyBoundaryConstraint(GetActorLocation(), Center) * AvoidanceWeight;

    Velocity += separationForce + alignmentForce + cohesionForce + boundaryForce;
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

FVector ABoidComponent::Cohesion(const TArray<ABoidComponent*>& Boids, FVector Center)
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

FVector ABoidComponent::ApplyBoundaryConstraint(const FVector& CurrentLocation, FVector Center)
{
    FVector force = FVector::ZeroVector;
    FVector relativeLocation = CurrentLocation - Center;

    //// DEBUG: Print everything
    //UE_LOG(LogTemp, Warning, TEXT("Current: (%.0f,%.0f,%.0f), Center: (%.0f,%.0f,%.0f), Relative: (%.0f,%.0f,%.0f), Boundary: (%.0f,%.0f,%.0f)"),
    //    CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z,
    //    Center.X, Center.Y, Center.Z,
    //    relativeLocation.X, relativeLocation.Y, relativeLocation.Z,
    //    BoundaryBox.X, BoundaryBox.Y, BoundaryBox.Z);

    if (FMath::Abs(relativeLocation.X) > BoundaryBox.X)
    {
        // Reverse X velocity with damping
        Velocity.X *= -0.8f;  // Reverse and reduce by 20%
        force.X = -FMath::Sign(relativeLocation.X) * BoundaryForce * 5.0f; // Emergency boost
    }

    if (FMath::Abs(relativeLocation.Y) > BoundaryBox.Y)
    {
        Velocity.Y *= -0.8f;
        force.Y = -FMath::Sign(relativeLocation.Y) * BoundaryForce * 5.0f;
    }

    if (FMath::Abs(relativeLocation.Z) > BoundaryBox.Z)
    {
        Velocity.Z *= -0.8f;
        force.Z = -FMath::Sign(relativeLocation.Z) * BoundaryForce * 5.0f;
    }

    if (!force.IsNearlyZero())
    {
        DrawDebugLine(GetWorld(), GetActorLocation(),
            GetActorLocation() + force * 0.1f,
            FColor::Red, false, -1.0f, 0, 3.0f);
    }

    return force.GetSafeNormal() * BoundaryForce;
}
