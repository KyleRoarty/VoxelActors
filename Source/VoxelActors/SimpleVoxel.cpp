// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

// Sets default values
ASimpleVoxel::ASimpleVoxel()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	verts = { FVector(0,0,0), FVector(10,0,0), FVector(10,10,0), FVector(0,10,0), FVector(0,0,10), FVector(10,0,10), FVector(10,10,10), FVector(0,10,10) };
	//verts = { FVector(0,0,0), FVector(10,0,0), FVector(5,5,0), FVector(0,0,10), FVector(10,0,10), FVector(5,5,10) };
	//verts = { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0), FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
	//verts = { 20 * FVector(1,1,1), 20 * FVector(1,1,-1), 20 * FVector(1,-1,1), 20 * FVector(1,-1,-1), 20 * FVector(-1,1,1), 20 * FVector(-1,1,-1), 20 * FVector(-1,-1,1), 20 * FVector(-1,-1,-1), 20 * FVector(0,1.618,1 / 1.618), 20 * FVector(0,1.618,-1 / 1.618), 20 * FVector(0,-1.618,1 / 1.618),20 * FVector(0,-1.618,-1 / 1.618),20 * FVector(1 / 1.618,0,1.618), 20 * FVector(1 / 1.618,0,-1.618),20 * FVector(-1 / 1.618,0,1.618),20 * FVector(-1 / 1.618,0,-1.618),20 * FVector(1.618,1 / 1.618,0),20 * FVector(1.618,-1 / 1.618,0),20 * FVector(-1.618,1 / 1.618,0),20 * FVector(-1.618,-1 / 1.618, 0) };

	//verts.Sort();

	num_v = verts.Num();
	mesh_made = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	RootComponent = mesh;

	voxel.verts = verts;
	voxel.colors = GetColors();
	voxel.normals = GetNormals();
	voxel.tans = GetTangents();
	voxel.tris = GetTris();

	static ConstructorHelpers::FObjectFinder<UMaterial> BlahMaterial(TEXT("Material'/Game/StarterContent/Materials/M_ColorGrid_LowSpec.M_ColorGrid_LowSpec'"));
	MyMaterial = BlahMaterial.Object;
	mesh->SetMaterial(0, MyMaterial);
}

bool ASimpleVoxel::Overlap(Seg_3 seg1, Seg_3 seg2)
{
	FVector int1, int2;
	FMath::SegmentDistToSegmentSafe(seg1.vert[0], seg1.vert[1], seg2.vert[0], seg2.vert[1], int1, int2);

	if ((int1 - int2).Size() > 0.00001)
		return false;
	if (int1 == seg1.vert[0] || int1 == seg1.vert[1])
		return false;
	if (int2 == seg2.vert[0] || int2 == seg2.vert[1])
		return false;

	return true;
}

void ASimpleVoxel::ResolveOverlap(int a, int b)
{
	int idx[2] = { a, b };
	bool safe_s[2] = { false, false };
	bool ignore_s[2] = { false, false };

	for (int i = 0; i < 2; i++) {
		if (safes.Contains(idx[i]))
			safe_s[i] = true;
		if (ignores.Contains(idx[i]))
			ignore_s[i] = true;
	}

	if ((safe_s[0] && ignore_s[0]) || (safe_s[1] && ignore_s[1]))
		return;
	if ((safe_s[0] && ignore_s[1]) || (ignore_s[0] && safe_s[1]) || (ignore_s[0] && ignore_s[1]))
		return;

	if (safe_s[0] && safe_s[1]) {
		safes.Remove(idx[1]);
		ignores.Emplace(idx[1]);
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		if (safe_s[i]) {
			ignores.Emplace(idx[i]);
			return;
		}
		if (ignore_s[i]) {
			safes.Emplace(idx[i]);
			return;
		}
	}

	ignores.Emplace(idx[0]);
	safes.Emplace(idx[1]);
	return;		
}

bool ASimpleVoxel::CheckIgTri(int a, int b)
{
	int idx = -1;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (segs[a].vert[i] == segs[b].vert[j]) {
				idx = (segs[a].idx[(i + 1) % 2] < segs[b].idx[(j + 1) % 2])?
					   SegFromI(segs[a].idx[(i + 1) % 2], segs[b].idx[(j + 1) % 2]):
					   SegFromI(segs[b].idx[(j + 1) % 2], segs[a].idx[(i + 1) % 2]);
			}
		}
	}

	if (idx == -1 || idx <= b)
		return false;

	if (ignores.Contains(idx))
		return true;

	return false;
}

int ASimpleVoxel::SegFromI(int a, int b)
{
	return a*num_v - a*(a + 1) / 2 + b - (a + 1);
}

int ASimpleVoxel::SegFromI(int a, int b, int n)
{
	return a*n - a*(a + 1) / 2 + b - (a + 1);
}


//a < b < c
//Indexes linear array of triangles from the three points defining the triangle
int ASimpleVoxel::TriFromI(int a, int b, int c) {
	return ((num_v - 2)*(num_v - 1)*num_v - (num_v - 2 - a)*(num_v - 1 - a)*(num_v - a)) / 6 + ((num_v - 2 - a)*(num_v - 1 - a) - (num_v - 1 - b)*(num_v - b)) / 2 + c - (b + 1);
}

TArray<int32> ASimpleVoxel::GetTris()
{
	TArray<bool> checked_tri;
	TArray<int32> ret_tris;
	int num_t;


	num_t = num_v*(num_v - 1)*(num_v - 2) / 6;
	checked_tri.Init(false, num_t);


	TArray<int> in_plane;
	FVector p_norm;
	bool pos, neg;
	float res;

	for (int i = 0; i < num_v; i++) {
		for (int j = i + 1; j < num_v; j++) {
			for (int k = j + 1; k < num_v; k++) {
				if (checked_tri[TriFromI(i, j, k)])
					continue;

				pos = neg = false;
				in_plane.Empty();
				in_plane.Append({ i, j, k });
				p_norm = FVector::CrossProduct(verts[k] - verts[i], verts[j] - verts[i]);

				for (int l = 0; l < num_v; l++) {
					if (l == i || l == j || l == k)
						continue;
					res = FVector::PointPlaneDist(verts[l], verts[i], p_norm);
					if (FMath::Abs(res) < .0001)
						in_plane.Emplace(l);
					else if (res >= 0) pos = true;
					else if (res < 0) neg = true;

				}
				
				for (int l = 0; l < in_plane.Num(); l++) {
					for (int m = l + 1; m < in_plane.Num(); m++) {
						for (int n = m + 1; n < in_plane.Num(); n++) {
							checked_tri[TriFromI(in_plane[l], in_plane[m], in_plane[n])] = true;
						}
					}
				}

				if (pos && neg) {
					UE_LOG(LogTemp, Warning, TEXT("Pos and neg true! %d %d %d"), i, j, k);
					continue;
				}

				ret_tris.Append(PlaneTris(in_plane));
				

			}
		}
	}

	return ret_tris;
}

TArray<int32> ASimpleVoxel::PlaneTris(TArray<int> points) {

	Seg_3 seg_tmp;
	int num_s;

	TArray<int32> ret;

	ignores.Empty();
	safes.Empty();
	segs.Empty();

	points.Sort();

	num_s = points.Num()*(points.Num() - 1) / 2;

	for (int i = 0; i < points.Num(); i++) {
		for (int j = i + 1; j < points.Num(); j++) {
			for (int k = 0, vfs[2] = { i, j }; k < 2; k++) {
				seg_tmp.vert[k] = verts[points[vfs[k]]];
				seg_tmp.idx[k] = points[vfs[k]];
			}
			segs.Emplace(seg_tmp);
		}
	}

	for (int i = 0; i < num_s; i++) {
		for (int j = i + 1; j < num_s; j++) {
			if (Overlap(segs[i], segs[j])) {
				ResolveOverlap(i, j);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Ignore:"));
	for (int ignore : ignores)
		UE_LOG(LogTemp, Warning, TEXT("%d (%d %d)"), ignore, segs[ignore].idx[0], segs[ignore].idx[1]);

	for (int i = 0; i < points.Num(); i++) {
		for (int j = i + 1; j < points.Num(); j++) {
			if (ignores.Contains(SegFromI(i, j, points.Num()))) {
				UE_LOG(LogTemp, Warning, TEXT("Ignoring %d %d (%d)"), points[i], points[j], SegFromI(i, j, points.Num()));
				continue;
			}
			for (int k = j + 1; k < points.Num(); k++) {
				if (ignores.Contains(SegFromI(i, k, points.Num()))){
					UE_LOG(LogTemp, Warning, TEXT("Ignoring %d %d (%d)"), points[i], points[k], SegFromI(i, k, points.Num()));
					continue;
				}
				if (ignores.Contains(SegFromI(j, k, points.Num()))) {
					UE_LOG(LogTemp, Warning, TEXT("Ignoring %d %d (%d)"), points[j], points[k], SegFromI(j, k, points.Num()));
					continue;
				}

				UE_LOG(LogTemp, Warning, TEXT("Adding %d %d %d"), points[i], points[j], points[k]);
				ret.Append({ points[i], points[j], points[k], points[k], points[j], points[i] });
			}
		}
	}

	return ret;


}

TArray<FVector2D> ASimpleVoxel::GetUV(TArray<FVector> pos, FVector2D center, FVector2D uv_range, FVector2D point_range)
{
	TArray<FVector2D> ret_uvs;
	for (FVector p : pos)
		ret_uvs.Emplace(FVector2D((p.X / point_range.X)*uv_range.X + center.X, (p.Y / point_range.Y)*uv_range.Y + center.Y));
	return ret_uvs;
}

TArray<FVector> ASimpleVoxel::GetNormals()
{
	TArray<FVector> ret_norms;
	for (int i = 0; i < num_v; i++)
		ret_norms.Emplace(FVector(0));
	return ret_norms;
}

TArray<FProcMeshTangent> ASimpleVoxel::GetTangents()
{
	TArray<FProcMeshTangent> ret_tans;
	for (int i = 0; i < num_v; i++)
		ret_tans.Emplace(FProcMeshTangent());
	return ret_tans;
}

TArray<FLinearColor> ASimpleVoxel::GetColors()
{
	TArray<FLinearColor> ret_colors;
	for (int i = 0; i < num_v; i++)
		ret_colors.Emplace(FLinearColor(ForceInitToZero));
	return ret_colors;
}

void ASimpleVoxel::CreateVoxel(FVector2D uv_center)
{
	voxel.uvs = GetUV(voxel.verts, uv_center, FVector2D(0.0625, 0.0625), FVector2D(1000, 1000));
	mesh->CreateMeshSection_LinearColor(0, voxel.verts, voxel.tris, voxel.normals, voxel.uvs, voxel.colors, voxel.tans, false);
	
	mesh->bUseComplexAsSimpleCollision = false;
	mesh->AddCollisionConvexMesh(voxel.verts);

	mesh->SetSimulatePhysics(true);
	mesh->SetEnableGravity(true);

	mesh_made = true;
}

// Called when the game starts or when spawned
void ASimpleVoxel::BeginPlay()
{
	Super::BeginPlay();
	CreateVoxel(FVector2D(.0625, .0625));
	
}

// Called every frame
void ASimpleVoxel::Tick(float DeltaTime)
{
	static float time = 0;
	TArray<FVector> new_verts;
	TArray<TArray<FVector>> nv_arr;

	Super::Tick(DeltaTime);

	new_verts.Append(voxel.verts);
	time += DeltaTime;

	if (mesh_made) {
		for (int i = 0; i < voxel.verts.Num(); i++) {
			new_verts[i] = FMath::InterpSinInOut(voxel.verts[i], 2 * voxel.verts[i], abs(sinf(time)));
		}

		mesh->UpdateMeshSection_LinearColor(0, new_verts, voxel.normals, voxel.uvs, voxel.colors, voxel.tans);
		nv_arr.Emplace(new_verts);
		mesh->SetCollisionConvexMeshes(nv_arr);
		//		mesh->ClearCollisionConvexMeshes();
		//		mesh->AddCollisionConvexMesh(new_verts);

	}
}

