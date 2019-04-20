// Fill out your copyright notice in the Description page of Project Settings.

#include "Face.h"

Face::Face()
{

}

Face::Face(TArray<FVector> points, FVector offset)
{
	this->offset = offset;

	this->furthest = FVector::ZeroVector;
	for (FVector p : points)
		if (p.Size() > this->furthest.Size())
			this->furthest = p;

	this->ret_points = points;
	this->ret_points.Sort([this](const FVector A, const FVector B) {
		return (A - furthest).Size() < (B - furthest).Size();
	});

	this->points = ret_points;
	for (FVector& point : this->points)
		point -= offset;

	GenerateTris();
}

Face::~Face()
{
}

TArray<FVector> Face::GetPoints()
{
	return ret_points;
}

int32 Face::NumPoints()
{
	return points.Num();
}

TArray<int32> Face::GetTris()
{
	return tris;
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
	TArray<int32> idxs;
	FVector norm;
	float f_dist, s_dist;

	tris.Empty();

	for (int i = 0; i < points.Num(); i++)
		idxs.Emplace(i);


	while (idxs.Num() >= 3) {
		norm = FVector::CrossProduct(points[idxs[1]] - points[idxs[0]], points[idxs[2]] - points[idxs[0]]).GetSafeNormal();
		norm.Normalize();

		if ((offset + norm).Size() >= (offset - norm).Size())
			tris.Append({ idxs[2], idxs[1], idxs[0] });
		else
			tris.Append({ idxs[0], idxs[1], idxs[2] });

		f_dist = (points[idxs[2]] - points[idxs[0]]).Size();
		s_dist = (points[idxs[2]] - points[idxs[1]]).Size();
		if (f_dist < s_dist)
			idxs.RemoveAt(0);
		else
			idxs.RemoveAt(1);

	}
}
