// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

// Sets default values
ASimpleVoxel::ASimpleVoxel()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//verts = { FVector(0,0,0), FVector(10,0,0), FVector(10,10,0), FVector(0,10,0), FVector(0,0,10), FVector(10,0,10), FVector(10,10,10), FVector(0,10,10) };
	//verts = { FVector(0,0,0), FVector(10,0,0), FVector(5,5,0), FVector(0,0,10), FVector(10,0,10), FVector(5,5,10) };
	verts = { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0), FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
	//verts = { 20 * FVector(1,1,1), 20 * FVector(1,1,-1), 20 * FVector(1,-1,1), 20 * FVector(1,-1,-1), 20 * FVector(-1,1,1), 20 * FVector(-1,1,-1), 20 * FVector(-1,-1,1), 20 * FVector(-1,-1,-1), 20 * FVector(0,1.618,1 / 1.618), 20 * FVector(0,1.618,-1 / 1.618), 20 * FVector(0,-1.618,1 / 1.618),20 * FVector(0,-1.618,-1 / 1.618),20 * FVector(1 / 1.618,0,1.618), 20 * FVector(1 / 1.618,0,-1.618),20 * FVector(-1 / 1.618,0,1.618),20 * FVector(-1 / 1.618,0,-1.618),20 * FVector(1.618,1 / 1.618,0),20 * FVector(1.618,-1 / 1.618,0),20 * FVector(-1.618,1 / 1.618,0),20 * FVector(-1.618,-1 / 1.618, 0) };
	num_v = verts.Num();

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

TArray<int32> ASimpleVoxel::GetTris()
{
	int iter;
	Seg_3 seg_tmp;
	Tri_3 tri_tmp;
	TArray<int> *tri_in_seg;
	int num_s, num_t;
	TArray<Tri_3> tris;


	num_s = num_v*(num_v - 1) / 2;
	num_t = num_v*(num_v - 1)*(num_v - 2) / 6;

	tri_in_seg = new TArray<int>[num_s];

	iter = 0;
	for (int i = 0; i < num_v; i++) {
		for (int j = i + 1; j < num_v; j++) {
			for (int k = 0, vfs[2] = { i, j }; k < 2; k++) {
				seg_tmp.vert[k] = verts[vfs[k]];
				seg_tmp.idx[k] = vfs[k];
			}
			segs.Emplace(seg_tmp);
		}
	}

	if (segs.Num() != num_s) {
		//Do something
		UE_LOG(LogTemp, Warning, TEXT("SEGS: Actual: %d, Expected: %d"), segs.Num(), num_s);
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

	num_t -= ignores.Num()*(num_v - 2);

	for (int i = 0, tmp = 0; i < num_v; i++) {
		tmp = 0;
		for (int ignore : ignores) {
			if (segs[ignore].vert[0] == verts[i] || segs[ignore].vert[1] == verts[i])
				tmp++;
		}
		num_t += ((tmp)*(tmp - 1) / 2);
	}


	for (int i = 0; i < ignores.Num(); i++)
		for (int j = i + 1; j < ignores.Num(); j++)
			if (CheckIgTri(ignores[i], ignores[j]))
				num_t--;

	UE_LOG(LogTemp, Warning, TEXT("Num tri's: %d"), num_t);

	int currTri = 0;
	for (int i = 0; i < num_v; i++) {
		for (int j = i + 1; j < num_v; j++) {
			if (ignores.Contains(SegFromI(i, j)))
				continue;
			for (int k = j + 1; k < num_v; k++) {
				if (ignores.Contains(SegFromI(i, k)) || ignores.Contains(SegFromI(j, k)))
					continue;
				for (int l = 0, m = 0, vft[3] = { i,j,k }; l < 3; l++)
				{
					tri_tmp.vert[l] = verts[vft[l]];
					tri_tmp.idx[l] = vft[l];
					m = (vft[l] < vft[(l + 1) % 3]) ? SegFromI(vft[l], vft[(l + 1) % 3]) : SegFromI(vft[(l + 1) % 3], vft[l]);
					tri_in_seg[m].Emplace(currTri);
				}
				tris.Emplace(tri_tmp);
				currTri++;
			}
		}
	}

	if (currTri != num_t) {
		//Do something
		UE_LOG(LogTemp, Warning, TEXT("TRI Actual: %d; Expected: %d"), currTri, num_t);
	}

	UE_LOG(LogTemp, Warning, TEXT("Use these triangles: "), tris.Num());
	for (Tri_3 tri : tris)
		UE_LOG(LogTemp, Warning, TEXT("(%d %d %d)"), tri.idx[0], tri.idx[1], tri.idx[2]);

	TArray<int32> ret_tris;
	for (Tri_3 tri : tris)
		ret_tris.Append({ tri.idx[0], tri.idx[1], tri.idx[2], tri.idx[2], tri.idx[1], tri.idx[0] });
	return ret_tris;
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
	voxel.uvs = GetUV(voxel.verts, uv_center, FVector2D(0.0625, 0.0625), FVector2D(50, 50));
	mesh->CreateMeshSection_LinearColor(0, voxel.verts, voxel.tris, voxel.normals, voxel.uvs, voxel.colors, voxel.tans, true);

	mesh->bUseComplexAsSimpleCollision = false;
	mesh->AddCollisionConvexMesh(voxel.verts);

	mesh->SetSimulatePhysics(true);
	mesh->SetEnableGravity(true);
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
	Super::Tick(DeltaTime);

}

