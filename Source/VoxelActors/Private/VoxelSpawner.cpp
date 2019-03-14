// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelSpawner.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
AVoxelSpawner::AVoxelSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	mesh = CreateDefaultSubobject<UMeshComponent>(TEXT("SpawnerMesh"));
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void AVoxelSpawner::BeginPlay()
{
	Super::BeginPlay();
	vox1 = GetWorld()->SpawnActorDeferred<ASimpleVoxel>(ASimpleVoxel::StaticClass(), FTransform(FVector(10, 100, 20)));
	if (vox1 != nullptr) {
		vox1->SetVerts(ASimpleVoxel::PENTAGON_3D);
		UGameplayStatics::FinishSpawningActor(vox1, FTransform(FVector(10, 100, 20)));
	}
	vox2 = GetWorld()->SpawnActorDeferred<ASimpleVoxel>(ASimpleVoxel::StaticClass(), FTransform(vox1->GetActorLocation()+FVector(0, 0, 20)));
	if (vox2 != nullptr) {
		vox2->SetVerts(ASimpleVoxel::PENTAGON_3D);
		UGameplayStatics::FinishSpawningActor(vox2, FTransform(vox1->GetActorLocation()+FVector(0, 0, 20)));
	}

	//vox1->FinishSpawning(FTransform(this->GetActorLocation()+FVector(10,20,0)));
	//vox2->FinishSpawning(FTransform(this->GetActorLocation()-FVector(-10,20,0)));
	//vox1->AttachToActor(vox2, FAttachmentTransformRules::KeepWorldTransform);
	//vox2->mesh->SetSimulatePhysics(false);
	//vox2->mesh->AttachTo(vox1->mesh, NAME_None, EAttachLocation::Type::KeepWorldPosition, true);

}

// Called every frame
void AVoxelSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

