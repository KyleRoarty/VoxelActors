// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class VOXELACTORS_API Shapes
{
public:
	Shapes();
	~Shapes();
	
	static TArray<FVector> Pentagon_3D();
	static TArray<FVector> RTri_3D();
	static TArray<FVector> Cube();
	static TArray<FVector> Dodecahedron();
	static TArray<FVector> Icosahedron();
};
