// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
//Must be last include
#include "SimpleVoxel.generated.h"

UCLASS()
class VOXELACTORS_API ASimpleVoxel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASimpleVoxel();

	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent *mesh;
	UPROPERTY(EditAnywhere)
		UMaterial *MyMaterial;

	void SetVerts(TArray<FVector> verts, float scale, bool grow);

	FVector bounds;

	TArray<TArray<FVector2D>> GetUVs();
	TArray<TArray<FVector>> GetVerts();
	TArray<TArray<FVector>> GetNormals();
	TArray<TArray<FProcMeshTangent>> GetTangents();
	TArray<TArray<FLinearColor>> GetColors();
	TArray<TArray<int32>> GetFaces();

private:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int TriFromI(int a, int b, int c);
	int TriFromI(int a, int b, int c, int n);

	TArray<int32> SimpleTris(TArray<int32> idxs);

	TArray<TArray<int32>> GenerateFaces();
	TArray<FVector2D> GenerateUVs(TArray<FVector> pos, FVector2D uv_range, FVector2D point_range, int row, int col);
	TArray<FVector> GenerateVerts(TArray<int32> idx);
	TArray<FVector> GenerateNormals(TArray<FVector> points);
	TArray<FProcMeshTangent> GenerateTangents(TArray<FVector> points);
	TArray<FLinearColor> GenerateColors(TArray<FVector> points);
	
	void CreateVoxel(FVector2D uv_center);
	
	FVector trans;
	TArray<FVector> verts;
	TArray<FVector> sort_verts;
	FVector avg_sort_vert;
	int num_v;
	int cnt;
	bool grow;

	TArray<TArray<FVector>> verts_arr;
	TArray<TArray<FVector>> normals;
	TArray<TArray<FVector2D>> uvs;
	TArray<TArray<FProcMeshTangent>> tans;
	TArray<TArray<FLinearColor>> colors;
	TArray<TArray<int32>> face_t;
	TArray<TArray<int32>> face_i;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
