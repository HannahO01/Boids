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

    //Mesh->SetRelativeScale3D(FVector(0.5f));
    Mesh->SetRelativeRotation(FRotator(0, 0, -90));

    Velocity = FVector(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1), FMath::RandRange(-1, 1)).GetSafeNormal() * MaxSpeed;
    NoiseOffset = FMath::RandRange(0.0f, 1000.f);
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

        newRotation.Pitch -= 90.0f;

        SetActorRotation(newRotation);
    }

    // Draw velocity direction (RED)
    DrawDebugLine(GetWorld(), GetActorLocation(),
        GetActorLocation() + Velocity.GetSafeNormal() * 100.0f,
        FColor::Red, false, -1.0f, 0, 2.0f);
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

    if (nearBoids.Num() == 0) {
        FVector boundaryForce = ApplyBoundaryConstraint(GetActorLocation(), Center) * AvoidanceWeight;
        Velocity += boundaryForce * DeltaTime;
        Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
        return;
    }

    FVector separationForce = Separation(nearBoids) * SeparationWeight;
    FVector alignmentForce = Alignment(nearBoids) * AlignmentWeight;
    FVector cohesionForce = Cohesion(nearBoids, Center) * CohesionWeight;
    FVector boundaryForce = ApplyBoundaryConstraint(GetActorLocation(), Center) * AvoidanceWeight;
    FVector swirlForce = CalculateSwirlForce(DeltaTime);

    DrawDebugLineDirection(separationForce, FColor::Blue);
    DrawDebugLineDirection(alignmentForce, FColor::Green);
    DrawDebugLineDirection(cohesionForce, FColor::Orange);
    DrawDebugLineDirection(boundaryForce, FColor::Red);
    DrawDebugLineDirection(swirlForce, FColor::Purple);
    swirlForce = FVector::ZeroVector;

    Velocity += (separationForce + alignmentForce + cohesionForce + boundaryForce + swirlForce) * DeltaTime;
    Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);

    float CurrentSpeed = Velocity.Size();

    if (CurrentSpeed < MinSpeed)
    {
        // Boost in current direction
        Velocity += Velocity.GetSafeNormal() * (MinSpeed - CurrentSpeed);
    }
}

void ABoidComponent::SetBoidMaterial(UMaterialInterface* NewMaterial)
{
    if (Mesh && NewMaterial)
    {
        Mesh->SetMaterial(0, NewMaterial); // Index 0 = first material slot
    }
}

void ABoidComponent::DrawDebugLineDirection(FVector Direction, FColor Colour)
{
    DrawDebugLine(GetWorld(), GetActorLocation(),
        GetActorLocation() + Direction.GetSafeNormal() * 100.0f,
        Colour, false, -1.0f, 0, 2.0f);
}

FVector ABoidComponent::Separation(const TArray<ABoidComponent*>& Boids)
{
    FVector steering = FVector::ZeroVector;

    for (ABoidComponent* boid : Boids) 
    {
        FVector diff = GetActorLocation() - boid->GetActorLocation();
        float distance = diff.Size();

        /*if (distance > 0)
            steering += diff.GetSafeNormal() / distance;*/
        if (distance > 0 && distance < 500.0f) // 3D: Only close neighbors
        {
            steering += diff / distance; // Inverse square law for 3D
        }
    }

    return steering/*.GetSafeNormal()*/;
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

    return (averageVelocity - Velocity)/*.GetSafeNormal()*/;
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

    return (centerOfMass - GetActorLocation())/*.GetSafeNormal()*/;
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
        //Velocity.X *= -0.8f;  // Reverse and reduce by 20%
        force.X = -FMath::Sign(relativeLocation.X) * BoundaryForce * 5.0f; // Emergency boost
    }

    if (FMath::Abs(relativeLocation.Y) > BoundaryBox.Y)
    {
        //Velocity.Y *= -0.8f;
        force.Y = -FMath::Sign(relativeLocation.Y) * BoundaryForce * 5.0f;
    }

    if (FMath::Abs(relativeLocation.Z) > BoundaryBox.Z)
    {
        //Velocity.Z *= -0.8f;
        force.Z = -FMath::Sign(relativeLocation.Z) * BoundaryForce * 5.0f;
    }

    if (!force.IsNearlyZero())
    {
        DrawDebugLine(GetWorld(), GetActorLocation(),
            GetActorLocation() + force * 0.1f,
            FColor::Red, false, -1.0f, 0, 3.0f);
    }

    return force/*.GetSafeNormal()*//* * BoundaryForce*/;
}

FVector ABoidComponent::CalculateSwirlForce(float DeltaTime)
{
    static float accumulatedTime = 0.0f;
    accumulatedTime += DeltaTime * SwirlFrequency;

    FVector noise = FVector::ZeroVector;

    noise.X = FMath::PerlinNoise3D(FVector(
        accumulatedTime + NoiseOffset,
        GetActorLocation().Y * 0.001f,
        GetActorLocation().Z * 0.001f
    ));

    noise.Y = FMath::PerlinNoise3D(FVector(
        GetActorLocation().X * 0.001f,
        accumulatedTime + NoiseOffset + 100.0f,
        GetActorLocation().Z * 0.001f
    ));

    noise.Z = FMath::PerlinNoise3D(FVector(
        GetActorLocation().X * 0.001f,
        GetActorLocation().Y * 0.001f,
        accumulatedTime + NoiseOffset + 200.0f
    ));

    DrawDebugLine(GetWorld(), GetActorLocation(),
        GetActorLocation() + noise * 10.1f,
        FColor::Red, false, -1.0f, 0, 3.0f);

    return noise * SwirlStrength;
}
