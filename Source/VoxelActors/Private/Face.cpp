// Fill out your copyright notice in the Description page of Project Settings.

#include "Face.h"

Face::Face(TArray<FVector> points, FVector offset)
{
	this->furthest = FVector::ZeroVector;
	for (FVector p : points)
		if (p.Size() > this->furthest.Size())
			this->furthest = p;

	this->points = points;
	this->points.Sort([this](const FVector A, const FVector B) {
		return (A - furthest).Size() < (B - furthest).Size();
	});

	this->offset = offset;
	GenerateTris();
}

Face::~Face()
{
}

TArray<FVector> Face::GetNormals()
{
	return normals;
}

void Face::SetNormals(TArray<FVector> normals)
{
	this->normals = normals;
}

TArray<FProcMeshTangent> Face::GetTangents()
{
	return tangents;
}

void Face::SetTangents(TArray<FProcMeshTangent> tangents)
{
	this->tangents = tangents;
}

TArray<FVector2D> Face::GetUVs()
{
	return uvs;
}

void Face::SetUVs(TArray<FVector2D> uvs)
{
	this->uvs = uvs;
}

TArray<FLinearColor> Face::GetColors()
{
	return colors;
}

void Face::SetColors(TArray<FLinearColor> colors)
{
	this->colors = colors;
}

// Internal function
void Face::GenerateTris()
{
}
