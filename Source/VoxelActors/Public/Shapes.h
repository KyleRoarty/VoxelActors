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
	
    static TArray<FVector> Pentagon_3D_Points();
    static TArray<TTuple<int, int>> Pentagon_3D_Segs();

    static TArray<FVector> RTri_3D_Points();
    static TArray<TTuple<int, int>> RTri_3D_Segs();

    static TArray<FVector> Cube_Points();
    static TArray<TTuple<int, int>> Cube_Segs();

    static TArray<FVector> Dodecahedron_Points();
    static TArray<TTuple<int, int>> Dodecahedron_Segs();

    static TArray<FVector> Icosahedron_Points();
    static TArray<TTuple<int, int>> Icosahedron_Segs();
};
