// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
//Must be last include
#include "SimpleVoxel.generated.h"

//For cube voxels
struct G_Vox {
	TArray<FVector> verts;
	TArray<FVector> normals;
	TArray<FVector2D> uvs;
	TArray<FProcMeshTangent> tans;
	TArray<FLinearColor> colors;
	TArray<int32> tris;
};

struct Tri_3 {
	FVector vert[3];
	int idx[3];
};

struct Seg_3 {
	FVector vert[2];
	int idx[2];
};

UCLASS()
class VOXELACTORS_API ASimpleVoxel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASimpleVoxel();

	G_Vox voxel;
	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent *mesh;
	UPROPERTY(EditAnywhere)
		UMaterial *MyMaterial;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool Overlap(Seg_3 seg1, Seg_3 seg2);
	void ResolveOverlap(int a, int b);
	bool CheckIgTri(int a, int b);
	int SegFromI(int a, int b);

	TArray<FVector> verts;
	TArray<int> safes, ignores;
	TArray<Seg_3> segs;
	int num_v;
	bool mesh_made;

	TArray<int32> GetTris();
	TArray<FVector2D> GetUV(TArray<FVector> pos, FVector2D center, FVector2D uv_range, FVector2D point_range);
	TArray<FVector> GetNormals();
	TArray<FProcMeshTangent> GetTangents();
	TArray<FLinearColor> GetColors();
	
	void CreateVoxel(FVector2D uv_center);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
