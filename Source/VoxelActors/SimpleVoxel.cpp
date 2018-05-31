// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

// Sets default values
ASimpleVoxel::ASimpleVoxel()
{
	int iter;
	Seg_3 seg_tmp;
	Tri_3 tri_tmp;
	TArray<int> *tri_in_seg;
	int num_t_use = 0;
	int tps, ips;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->vert = { FVector(0,0,0), FVector(10,0,0), FVector(10,10,0), FVector(0,10,0), FVector(0,0,10), FVector(10,0,10), FVector(10,10,10), FVector(0,10,10) };
	this->num_v = vert.Num();
	this->num_s = num_v*(num_v - 1) / 2;
	this->num_t = num_v*(num_v - 1)*(num_v - 2) / 6;

	tri_in_seg = new TArray<int>[this->num_s];

	iter = 0;
	for (int i = 0; i < this->num_v; i++) {
		for (int j = i + 1; j < this->num_v; j++) {
			for (int k = 0, vfs[2] = { i, j }; k < 2; k++) {
				seg_tmp.vert[k] = this->vert[vfs[k]];
				seg_tmp.idx[k] = vfs[k];
			}
			this->seg.Emplace(seg_tmp);
		}
	}

	if (this->seg.Num() != this->num_s) {
		//Do something
		UE_LOG(LogTemp, Warning, TEXT("SEGS: Actual: %d, Expected: %d"), this->seg.Num(), this->num_s);
	}

	for (int i = 0; i < this->num_s; i++) {
		for (int j = i + 1; j < this->num_s; j++) {
			if (this->Overlap(this->seg[i], this->seg[j])) {
				this->ResolveOverlap(i, j);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Ignore:"));
	for (int i = 0; i < this->ignore.Num(); i++)
		UE_LOG(LogTemp, Warning, TEXT("%d (%d %d)"), ignore[i], seg[ignore[i]].idx[0], seg[ignore[i]].idx[1]);

	this->num_t -= this->ignore.Num()*(num_v - 2);

	for (int i = 0, tmp = 0; i < this->num_v; i++) {
		tmp = 0;
		for (int j = 0; j < this->ignore.Num(); j++) {
			if (this->seg[ignore[j]].vert[0] == this->vert[i] || this->seg[ignore[j]].vert[1] == this->vert[i])
				tmp++;
		}
		this->num_t += ((tmp)*(tmp - 1) / 2);
	}


	for (int i = 0; i < this->ignore.Num(); i++)
		for (int j = i + 1; j < this->ignore.Num(); j++)
			if (this->CheckIgTri(this->ignore[i], this->ignore[j]))
				this->num_t--;

	UE_LOG(LogTemp, Warning, TEXT("Num tri's: %d"), this->num_t);

	tri_tmp.use = false;
	tri_tmp.ignore = false;
	int currTri = 0;
	for (int i = 0; i < this->num_v; i++) {
		for (int j = i + 1; j < this->num_v; j++) {
			if (this->ignore.Contains(this->SegFromI(i, j)))
				continue;
			for (int k = j + 1; k < this->num_v; k++) {
				if (this->ignore.Contains(this->SegFromI(i, k)) || this->ignore.Contains(this->SegFromI(j, k)))
					continue;
				for (int l = 0, m = 0, vft[3] = { i,j,k }; l < 3; l++)
				{
					tri_tmp.vert[l] = this->vert[vft[l]];
					tri_tmp.idx[l] = vft[l];
					m = (vft[l] < vft[(l + 1) % 3]) ? this->SegFromI(vft[l], vft[(l + 1) % 3]) : this->SegFromI(vft[(l + 1) % 3], vft[l]);
					tri_in_seg[m].Emplace(currTri);
				}
				this->tri.Emplace(tri_tmp);
				currTri++;
			}
		}
	}

	if (currTri != this->num_t) {
		//Do something
		UE_LOG(LogTemp, Warning, TEXT("TRI Actual: %d; Expected: %d"), currTri, this->num_t);
	}

	for (int i = 0; i < this->num_s; i++) {
		if (tri_in_seg[i].Num() != 2)
			continue;
		UE_LOG(LogTemp, Warning, TEXT("SEG %d (%d %d) has 2 tri: %d, %d"), i, seg[i].idx[0], seg[i].idx[1], tri_in_seg[i][0], tri_in_seg[i][1]);
		for (int j = 0; j < tri_in_seg[i].Num(); j++)
			tri[tri_in_seg[i][j]].use = true;
	}

	for (int i = 0; i < tri.Num(); i++)
		if (tri[i].use)
			num_t_use++;

	while (num_t_use != (this->num_v - 2) * 2) {
		for (int i = 0; i < this->num_s; i++) {
			tps = 0;
			ips = 0;
			if (tri_in_seg[i].Num() == 2 || ignore.Contains(i))
				continue;

			for (int j = 0; j < tri_in_seg[i].Num(); j++) {
				if (this->tri[tri_in_seg[i][j]].use)
					tps++;
				if (this->tri[tri_in_seg[i][j]].ignore)
					ips++;
			}
			UE_LOG(LogTemp, Warning, TEXT("seg %d (%d %d): tps %d; ips %d"), i, seg[i].idx[0], seg[i].idx[1], tps, ips);

			if (tps < 2 && tps != 0)
				for (int j = 0; j < tri_in_seg[i].Num(); j++)
					if (!tri[tri_in_seg[i][j]].ignore)
						tri[tri_in_seg[i][j]].use = true;
			if (ips < (tri_in_seg[i].Num() - 2))
				for (int j = 0; j < tri_in_seg[i].Num(); j++)
					if (!tri[tri_in_seg[i][j]].use)
						tri[tri_in_seg[i][j]].ignore = true;
			if (ips > (tri_in_seg[i].Num() - 2))
				for (int j = 0; j < tri_in_seg[i].Num(); j++)
					tri[tri_in_seg[i][j]].ignore = false;
			if (tps > 2)
				for (int j = 0; j < tri_in_seg[i].Num(); j++)
					tri[tri_in_seg[i][j]].use = false;
		}
		num_t_use = 0;
		for (int i = 0; i < this->tri.Num(); i++)
			if (tri[i].use)
				num_t_use++;
	}

	UE_LOG(LogTemp, Warning, TEXT("Use these triangles: "), tri.Num());
	for (int i = 0; i < this->tri.Num(); i++)
		if (tri[i].use)
			UE_LOG(LogTemp, Warning, TEXT("%d (%d %d %d)"), i, tri[i].idx[0], tri[i].idx[1], tri[i].idx[2]);


	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	RootComponent = mesh;

	voxel.verts = this->vert;
	voxel.colors = this->GetColors();
	voxel.normals = this->GetNormals();
	voxel.tans = this->GetTangents();
	voxel.tris = this->GetTris();

	static ConstructorHelpers::FObjectFinder<UMaterial> BlahMaterial(TEXT("Material'/Game/StarterContent/Materials/M_ColorGrid_LowSpec.M_ColorGrid_LowSpec'"));
	MyMaterial = BlahMaterial.Object;
	mesh->SetMaterial(0, MyMaterial);
}

bool ASimpleVoxel::Overlap(Seg_3 seg1, Seg_3 seg2)
{
	FVector int1, int2;
	FMath::SegmentDistToSegmentSafe(seg1.vert[0], seg1.vert[1], seg2.vert[0], seg2.vert[1], int1, int2);

	if (int1 != int2)
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
		if (this->safe.Contains(idx[i]))
			safe_s[i] = true;
		if (this->ignore.Contains(idx[i]))
			ignore_s[i] = true;
	}

	if ((safe_s[0] && ignore_s[0]) || (safe_s[1] && ignore_s[1]))
		return;
	if ((safe_s[0] && ignore_s[1]) || (ignore_s[0] && safe_s[1]) || (ignore_s[0] && ignore_s[1]))
		return;

	if (safe_s[0] && safe_s[1]) {
		this->safe.Remove(idx[1]);
		this->ignore.Emplace(idx[1]);
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		if (safe_s[i]) {
			this->ignore.Emplace(idx[i]);
			return;
		}
		if (ignore_s[i]) {
			this->safe.Emplace(idx[i]);
			return;
		}
	}

	this->ignore.Emplace(idx[0]);
	this->safe.Emplace(idx[1]);
	return;		
}

bool ASimpleVoxel::CheckIgTri(int a, int b)
{
	int idx = -1;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (this->seg[a].vert[i] == this->seg[b].vert[j]) {
				idx = (this->seg[a].idx[(i + 1) % 2] < this->seg[b].idx[(j + 1) % 2])?
					   this->SegFromI(this->seg[a].idx[(i + 1) % 2], this->seg[b].idx[(j + 1) % 2]):
					   this->SegFromI(this->seg[b].idx[(j + 1) % 2], this->seg[a].idx[(i + 1) % 2]);
			}
		}
	}

	if (idx == -1 || idx <= b)
		return false;

	if (this->ignore.Contains(idx))
		return true;

	return false;
}

int ASimpleVoxel::SegFromI(int a, int b)
{
	return a*this->num_v - a*(a + 1) / 2 + b - (a + 1);
}

TArray<int32> ASimpleVoxel::GetTris()
{
	TArray<int32> ret_tris;
	for (Tri_3 t : this->tri)
		if(t.use)
			ret_tris.Append({ t.idx[0], t.idx[1], t.idx[2], t.idx[2], t.idx[1], t.idx[0] });
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
	for (int i = 0; i < this->num_v; i++)
		ret_norms.Emplace(FVector(0));
	return ret_norms;
}

TArray<FProcMeshTangent> ASimpleVoxel::GetTangents()
{
	TArray<FProcMeshTangent> ret_tans;
	for (int i = 0; i < this->num_v; i++)
		ret_tans.Emplace(FProcMeshTangent());
	return ret_tans;
}

TArray<FLinearColor> ASimpleVoxel::GetColors()
{
	TArray<FLinearColor> ret_colors;
	for (int i = 0; i < this->num_v; i++)
		ret_colors.Emplace(FLinearColor(ForceInitToZero));
	return ret_colors;
}

void ASimpleVoxel::CreateVoxel(FVector2D uv_center)
{
	voxel.uvs = GetUV(voxel.verts, uv_center, FVector2D(0.0625, 0.0625), FVector2D(10, 10));
	mesh->CreateMeshSection_LinearColor(0, voxel.verts, voxel.tris, voxel.normals, voxel.uvs, voxel.colors, voxel.tans, true);
	mesh->ContainsPhysicsTriMeshData(true);
	mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
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

