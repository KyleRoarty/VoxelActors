// Fill out your copyright notice in the Description page of Project Settings.

#include "Shapes.h"

#define PHI (1+sqrt(5))/2.0
#define IPHI 2.0/(1+sqrt(5))

Shapes::Shapes()
{
}

Shapes::~Shapes()
{
}

TArray<FVector> Shapes::Pentagon_3D_Points()
{
	return { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0),
			 FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
}

TArray<TTuple<int, int>> Shapes::Pentagon_3D_Segs()
{
    return {};
}

TArray<FVector> Shapes::RTri_3D_Points()
{
	return { FVector(-1,-1,-1), FVector(1,-1,-1), FVector(0,0,-1),
			 FVector(-1,-1,1), FVector(1,-1,1), FVector(0,0,1) };
}

TArray<TTuple<int, int>> Shapes::RTri_3D_Segs()
{
    return {};

}
TArray<FVector> Shapes::Cube_Points()
{
	return { FVector(-1,-1,-1), FVector(1,-1,-1), FVector(1,1,-1), FVector(-1,1,-1),
			 FVector(-1,-1,1), FVector(1,-1,1), FVector(1,1,1), FVector(-1,1,1) };
}

TArray<TTuple<int, int>> Shapes::Cube_Segs()
{
    return {  TTuple<int, int>(0, 1),
              TTuple<int, int>(0, 4),
              TTuple<int, int>(0, 3),
              TTuple<int, int>(1, 2),
              TTuple<int, int>(1, 5),
              TTuple<int, int>(2, 3),
              TTuple<int, int>(2, 6),
              TTuple<int, int>(3, 7),
              TTuple<int, int>(4, 5),
              TTuple<int, int>(4, 7),
              TTuple<int, int>(5, 6),
              TTuple<int, int>(6, 7) };
}
TArray<FVector> Shapes::Dodecahedron_Points()
{
	return { FVector(1,1,1), FVector(1,1,-1), FVector(1,-1,1), FVector(1,-1,-1), FVector(-1,1,1),
		     FVector(-1,1,-1), FVector(-1,-1,1), FVector(-1,-1,-1), FVector(0,PHI,IPHI), FVector(0,PHI,-IPHI),
			 FVector(0,-PHI,IPHI), FVector(0,-PHI,-IPHI), FVector(IPHI,0,PHI), FVector(IPHI,0,-PHI), FVector(-IPHI,0,PHI),
			 FVector(-IPHI,0,-PHI), FVector(PHI,IPHI,0), FVector(PHI,-IPHI,0), FVector(-PHI,IPHI,0), FVector(-PHI,-IPHI, 0) };
}

TArray<TTuple<int, int>> Shapes::Dodecahedron_Segs()
{
    return {};

}
TArray<FVector> Shapes::Icosahedron_Points()
{
	return { FVector(0,1,PHI), FVector(0,1,-PHI), FVector(0,-1,PHI), FVector(0,-1,-PHI),
			 FVector(1,PHI,0), FVector(1,-PHI,0), FVector(-1,PHI,0), FVector(-1,-PHI,0),
			 FVector(PHI,0,1), FVector(PHI,0,-1), FVector(-PHI,0,1), FVector(-PHI,0,-1) };
}

TArray<TTuple<int, int>> Shapes::Icosahedron_Segs()
{
    return {};
}
