// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

/**
 * 
 */
class VOXELACTORS_API Face
{
public:
	Face();
	Face(TArray<FVector> points, FVector offset);
	~Face();

	TArray<FVector> GetPoints();
	int32 NumPoints();

	TArray<int32> GetTris();

	TArray<FVector> GetNormals();
	void SetNormals(TArray<FVector> normals);

	TArray<FProcMeshTangent> GetTangents();
	void SetTangents(TArray<FProcMeshTangent> tangents);

	TArray<FVector2D> GetUVs();
	void SetUVs(TArray<FVector2D> uvs);

	TArray<FLinearColor> GetColors();
	void SetColors(TArray<FLinearColor> colors);

private:
	TArray<FVector> points;
	TArray<FVector> ret_points;
	FVector offset;
	FVector furthest;

	// Stuff relating to creating the face as a procedural generated plane
	TArray<int32> tris; // Triplet of points that make a triangle
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FVector2D> uvs;
	TArray<FLinearColor> colors;

	void GenerateTris();
};
