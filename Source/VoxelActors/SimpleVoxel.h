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

	void SetVerts(TArray<FVector> verts);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	FVector avg_sort_vert;
	int num_v;
	int cnt;
	bool mesh_made;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Points for basic shapes that I use a lot
	static const TArray<FVector> PENTAGON_3D;
	static const TArray<FVector> RTRI_3D;
	static const TArray<FVector> CUBE;
	static const TArray<FVector> DODECAHEDRON;
	static const TArray<FVector> ICOSAHEDRON;

};
