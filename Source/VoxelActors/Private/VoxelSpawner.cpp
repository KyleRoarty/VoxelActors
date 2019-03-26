// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelSpawner.h"
#include "Runtime/Engine/Classes/Engine/World.h"

// Sets default values
AVoxelSpawner::AVoxelSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	mesh = CreateDefaultSubobject<UMeshComponent>(TEXT("SpawnerMesh"));
	RootComponent = mesh;

	constraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics constraint"));
}

// Called when the game starts or when spawned
void AVoxelSpawner::BeginPlay()
{
	Super::BeginPlay();
	vox1 = GetWorld()->SpawnActorDeferred<ASimpleVoxel>(ASimpleVoxel::StaticClass(), FTransform(FVector(10, 100, 40)));
	if (vox1 != nullptr) {
		vox1->SetVerts(ASimpleVoxel::CUBE, false);
		vox1->FinishSpawning(FTransform(FVector(10, 100, 40)));
	}
	vox2 = GetWorld()->SpawnActorDeferred<ASimpleVoxel>(ASimpleVoxel::StaticClass(), FTransform(vox1->GetActorLocation()+FVector(0, vox1->bounds.Y, 0)));
	if (vox2 != nullptr) {
		vox2->SetVerts(ASimpleVoxel::CUBE, false);
		vox2->FinishSpawning(FTransform(vox1->GetActorLocation() + FVector(0, vox1->bounds.Y, 0)));
	}

	//vox1->SetConstrain(vox2);

	FConstraintInstance constraints;

	constraints.SetDisableCollision(true);

	constraints.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
	constraints.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
	constraints.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);

	constraints.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
	constraints.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
	constraints.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);


	constraintComp->ConstraintInstance = constraints;
	
	constraintComp->AttachToComponent(vox1->mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	constraintComp->ConstraintActor1 = vox1;
	constraintComp->ConstraintActor2 = vox2;


	constraintComp->SetConstrainedComponents(vox1->mesh, NAME_None, vox2->mesh, NAME_None);
}

// Called every frame
void AVoxelSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

