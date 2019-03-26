// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleVoxel.h"
#include "Runtime/Engine/Classes/PhysicsEngine/PhysicsConstraintComponent.h"
#include "VoxelSpawner.generated.h"

UCLASS()
class VOXELACTORS_API AVoxelSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelSpawner();

	UPROPERTY(VisibleAnywhere)
		UMeshComponent *mesh;
	UPROPERTY(EditAnywhere)
		UPhysicsConstraintComponent *constraintComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	ASimpleVoxel *vox1, *vox2;
	
};
