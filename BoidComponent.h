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

    void SetBoidMaterial(UMaterialInterface* NewMaterial);

    void DrawDebugLineDirection(FVector Direction, FColor Colour);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinSpeed = 150.0f;

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
    FVector CalculateSwirlForce(float DeltaTime);

    UPROPERTY(EditAnywhere, Category = "Boids|Swirl")
    float SwirlStrength = 100.0f;
    UPROPERTY(EditAnywhere, Category = "Boids|Swirl")
    float SwirlFrequency = 0.5f;
    UPROPERTY(EditAnywhere, Category = "Boids|Swirl")
    float SwirlWeight = 0.5f;

    float NoiseOffset;

    UPROPERTY()
    FVector BoundaryBox = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Boids|Boundary")
    float BoundaryForce = 600.0f;

    UPROPERTY(EditAnywhere)
    float SeparationWeight = 5.0f;

    UPROPERTY(EditAnywhere)
    float AlignmentWeight = 0.1f;

    UPROPERTY(EditAnywhere)
    float CohesionWeight = 0.05f;

    UPROPERTY(EditAnywhere)
    float AvoidanceWeight = 2.0f;

    FRotator TargetRotation;
    float RotationSpeed = 5.0f;
};