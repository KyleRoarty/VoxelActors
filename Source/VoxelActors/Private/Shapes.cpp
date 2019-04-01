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

TArray<FVector> Shapes::Pentagon_3D()
{
	return { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0),
			 FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
}

TArray<FVector> Shapes::RTri_3D()
{
	return { FVector(-1,-1,-1), FVector(1,-1,-1), FVector(0,0,-1),
			 FVector(-1,-1,1), FVector(1,-1,1), FVector(0,0,1) };
}

TArray<FVector> Shapes::Cube()
{
	return { FVector(-1,-1,-1), FVector(1,-1,-1), FVector(1,1,-1), FVector(-1,1,-1),
			 FVector(-1,-1,1), FVector(1,-1,1), FVector(1,1,1), FVector(-1,1,1) };
}

TArray<FVector> Shapes::Dodecahedron()
{
	return { FVector(1,1,1), FVector(1,1,-1), FVector(1,-1,1), FVector(1,-1,-1), FVector(-1,1,1),
		     FVector(-1,1,-1), FVector(-1,-1,1), FVector(-1,-1,-1), FVector(0,PHI,IPHI), FVector(0,PHI,-IPHI),
			 FVector(0,-PHI,IPHI), FVector(0,-PHI,-IPHI), FVector(IPHI,0,PHI), FVector(IPHI,0,-PHI), FVector(-IPHI,0,PHI),
			 FVector(-IPHI,0,-PHI), FVector(PHI,IPHI,0), FVector(PHI,-IPHI,0), FVector(-PHI,IPHI,0), FVector(-PHI,-IPHI, 0) };
}

TArray<FVector> Shapes::Icosahedron()
{
	return { FVector(0,1,PHI), FVector(0,1,-PHI), FVector(0,-1,PHI), FVector(0,-1,-PHI),
			 FVector(1,PHI,0), FVector(1,-PHI,0), FVector(-1,PHI,0), FVector(-1,-PHI,0),
			 FVector(PHI,0,1), FVector(PHI,0,-1), FVector(-PHI,0,1), FVector(-PHI,0,-1) };
}
