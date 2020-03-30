// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "Private/HelperGeometry.h"
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

    void SetVerts(const TArray<FVector>& verts_, const float& scale, const bool& grow_);
    void SetEdges(const TArray<TTuple<int, int>>& edges);

	FVector GetBounds();
	TArray<TArray<FVector2D>> GetUVs();
	TArray<TArray<FVector>> GetNormals();
	TArray<TArray<FProcMeshTangent>> GetTangents();
    TArray<TArray<FLinearColor>> GetColors();

private:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateFaces();
	void GenerateUVs(FVector2D uv_range, FVector2D point_range, int row, int col);
	void GenerateNormalsAndTans();
	void GenerateColors();
	
	void CreateVoxel();
	
	FVector trans;
    TArray<FVector> verts;
    bool grow;

	FVector bounds;

    TArray<TSharedRef<FPoint>> Points;
    TArray<FVector> PointsPosition;
    TMap<TBitArray<FDefaultBitArrayAllocator>, TSharedRef<FLine>> LineSegments;
    TArray<TSharedRef<FFace>> Faces;

    TBitArray<FDefaultBitArrayAllocator> MakeSegmentKey(TSharedRef<FPoint>& Start,
                                                        TSharedRef<FPoint>& End);
    void Triangulate();

	TArray<TArray<FVector>> normals;
	TArray<TArray<FVector2D>> uvs;
	TArray<TArray<FProcMeshTangent>> tans;
	TArray<TArray<FLinearColor>> colors;
    TArray<TArray<int32>> face_t;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
