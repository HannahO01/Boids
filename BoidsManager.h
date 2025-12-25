#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidsManager.generated.h"

UCLASS()
class BOIDS_API ABoidsManager : public AActor 
{
	GENERATED_BODY()
public:
	ABoidsManager();

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BoidCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ABoidComponent> BoidClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnBounds = FVector(1000, 1000, 1000);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<class ABoidComponent*> Boids;

    void SpawnBoids();
};