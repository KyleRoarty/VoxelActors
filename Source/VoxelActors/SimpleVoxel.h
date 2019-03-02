// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
//Must be last include
#include "SimpleVoxel.generated.h"

//For cube voxels
struct G_Vox {
	TArray<TArray<FVector>> verts;
	TArray<TArray<FVector>> normals;
	TArray<TArray<FVector2D>> uvs;
	TArray<TArray<FProcMeshTangent>> tans;
	TArray<TArray<FLinearColor>> colors;
	TArray<TArray<int32>> face_t;
	TArray<TArray<int32>> face_i;
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

	int SegFromI(int a, int b);
	int SegFromI(int a, int b, int n);
	int TriFromI(int a, int b, int c);
	int TriFromI(int a, int b, int c, int n);

	TArray<TArray<int32>> GetFaces();
	TArray<int32> SimpleTris(TArray<int32> idxs);

	TArray<FVector2D> GetUV(TArray<FVector> pos, FVector2D uv_range, FVector2D point_range, int row, int col);
	TArray<FVector> GetVerts(TArray<int32> idx);
	TArray<FVector> GetNormals(TArray<FVector> points);
	TArray<FProcMeshTangent> GetTangents(TArray<FVector> points);
	TArray<FLinearColor> GetColors(TArray<FVector> points);
	
	void CreateVoxel(FVector2D uv_center);
	
	FVector trans;
	TArray<FVector> verts;
	TArray<FVector> sort_verts;
	int num_v;
	int cnt;
	bool mesh_made;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
