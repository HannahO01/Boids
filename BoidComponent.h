#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidComponent.generated.h"

UCLASS()
class BOIDS_API ABoidComponent : public AActor
{
    GENERATED_BODY()

public:
    ABoidComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void UpdateBoid(const TArray<class ABoidComponent*>& AllBoids, float DeltaTime, FVector Center);

    void SetBoundaryBox(FVector SomeBoundaryBox) { BoundaryBox = SomeBoundaryBox; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerceptionRadius = 1000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Velocity;
private:
    UPROPERTY(EditAnywhere)
    class UStaticMeshComponent* Mesh;

    FVector Separation(const TArray<ABoidComponent*>& Boids);
    FVector Alignment(const TArray<ABoidComponent*>& Boids);
    FVector Cohesion(const TArray<ABoidComponent*>& Boids, FVector Center);

    FVector ApplyBoundaryConstraint(const FVector& CurrentLocation, FVector Center);

    UPROPERTY()
    FVector BoundaryBox = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Boids|Boundary")
    float BoundaryForce = 600.0f;

    UPROPERTY(EditAnywhere)
    float SeparationWeight = 1.2f;

    UPROPERTY(EditAnywhere)
    float AlignmentWeight = 1.5f;

    UPROPERTY(EditAnywhere)
    float CohesionWeight = 1.3f;

    UPROPERTY(EditAnywhere)
    float AvoidanceWeight = 2.5f;
};