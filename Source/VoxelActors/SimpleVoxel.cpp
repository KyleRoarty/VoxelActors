// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"


#define PHI (1+sqrt(5))/2.0
#define IPHI 2.0/(1+sqrt(5))
// Sets default values
ASimpleVoxel::ASimpleVoxel()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FVector trans = FVector(0);

	//verts = { 20*FVector(-1,-1,-1), 20*FVector(1,-1,-1), 20*FVector(1,1,-1), 20*FVector(-1,1,-1), 20*FVector(-1,-1,1), 20*FVector(1,-1,1), 20*FVector(1,1,1), 20*FVector(-1,1,1) };
	//verts = { 20*FVector(-1,-1,-1), 20*FVector(1,-1,-1), 20*FVector(0,0,-1), 20*FVector(-1,-1,1), 20*FVector(1,-1,1), 20*FVector(0,0,1) };
	//verts = { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0), FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
	/*verts = { 20 * FVector(1,1,1), 20 * FVector(1,1,-1), 20 * FVector(1,-1,1), 20 * FVector(1,-1,-1), 20 * FVector(-1,1,1), 20 * FVector(-1,1,-1), 20 * FVector(-1,-1,1), 20 * FVector(-1,-1,-1),
			  20 * FVector(0,PHI,IPHI), 20 * FVector(0,PHI,-IPHI), 20 * FVector(0,-PHI,IPHI),20 * FVector(0,-PHI,-IPHI),
			  20 * FVector(IPHI,0,PHI), 20 * FVector(IPHI,0,-PHI),20 * FVector(-IPHI,0,PHI),20 * FVector(-IPHI,0,-PHI),
			  20 * FVector(PHI,IPHI,0),20 * FVector(PHI,-IPHI,0),20 * FVector(-PHI,IPHI,0),20 * FVector(-PHI,-IPHI, 0) };*/
	verts = { 20 * FVector(0,1,PHI), 20 * FVector(0,1,-PHI), 20 * FVector(0,-1,PHI), 20 * FVector(0,-1,-PHI),
			  20 * FVector(1,PHI,0), 20 * FVector(1,-PHI,0), 20 * FVector(-1,PHI,0), 20 * FVector(-1,-PHI,0),
			  20 * FVector(PHI,0,1), 20 * FVector(PHI,0,-1), 20 * FVector(-PHI,0,1), 20 * FVector(-PHI,0,-1)};

	for (FVector v : verts) {
		if (v.X < trans.X)
			trans.X = v.X;
		if (v.Y < trans.Y)
			trans.Y = v.Y;
		if (v.Z < trans.Z)
			trans.Z = v.Z;
	}

	verts.Sort([trans](const FVector A,  const FVector B){
		return (A-trans).Size() < (B-trans).Size();
	});

	for (FVector v : verts) {
		sort_verts.Emplace(v - trans);
	}

	num_v = verts.Num();
	mesh_made = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	mesh->bUseAsyncCooking = true;
	RootComponent = mesh;

	voxel.verts = verts;
	voxel.colors = GetColors();
	voxel.normals = GetNormals();
	voxel.tans = GetTangents();
	voxel.tris = GetTris();

	static ConstructorHelpers::FObjectFinder<UMaterial> BlahMaterial(TEXT("Material'/Game/StarterContent/Materials/M_ColorGrid_LowSpec.M_ColorGrid_LowSpec'"));
	MyMaterial = BlahMaterial.Object;
}

int ASimpleVoxel::SegFromI(int a, int b)
{
	return SegFromI(a, b, num_v);
}

int ASimpleVoxel::SegFromI(int a, int b, int n)
{
	return a*n - a*(a + 1) / 2 + b - (a + 1);
}


//a < b < c
//Indexes linear array of triangles from the three points defining the triangle
int ASimpleVoxel::TriFromI(int a, int b, int c) {
	return TriFromI(a, b, c, num_v);
}

int ASimpleVoxel::TriFromI(int a, int b, int c, int n) {
	return ((n - 2)*(n - 1)*n - (n - 2 - a)*(n - 1 - a)*(n - a)) / 6 + ((n - 2 - a)*(n - 1 - a) - (n - 1 - b)*(n - b)) / 2 + c - (b + 1);
}

TArray<TArray<int32>> ASimpleVoxel::GetTris()
{
	TArray<bool> checked_tri;
	TArray<TArray<int32>> ret_tris;
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
				if (checked_tri[TriFromI(i, j, k)]) {
					//UE_LOG(LogTemp, Warning, TEXT("Skipping %d %d %d"), i, j, k);
					continue;
				}

				pos = neg = false;
				in_plane.Empty();
				in_plane.Append({ i, j, k });
				p_norm = FVector::CrossProduct(verts[k] - verts[i], verts[j] - verts[i]);

				for (int l = 0; l < num_v; l++) {
					if (l == i || l == j || l == k)
						continue;
					res = FVector::PointPlaneDist(verts[l], verts[i], p_norm);
					if (FMath::Abs(res) < 1)
						in_plane.Emplace(l);
					else if (res >= 0) pos = true;
					else if (res < 0) neg = true;

				}

				in_plane.Sort();
				
				for (int l = 0; l < in_plane.Num(); l++) {
					for (int m = l + 1; m < in_plane.Num(); m++) {
						for (int n = m + 1; n < in_plane.Num(); n++) {
							checked_tri[TriFromI(in_plane[l], in_plane[m], in_plane[n])] = true;
						}
					}
				}

				if (pos && neg) {
					//UE_LOG(LogTemp, Warning, TEXT("Pos and neg true! %d %d %d"), i, j, k);
					continue;
				}

				UE_LOG(LogTemp, Warning, TEXT("Generating triangles for %d %d %d"), i, j, k);
				for (int i : in_plane)
					UE_LOG(LogTemp, Warning, TEXT("%d"), i);

				ret_tris.Emplace(SimpleTris(in_plane));
				

			}
		}
	}

	return ret_tris;
}

TArray<int32> ASimpleVoxel::SimpleTris(TArray<int> points) {
	TArray<int32> ret;

	//First, second distances
	float f_dist, s_dist;

	while (points.Num() >= 3) {
		UE_LOG(LogTemp, Warning, TEXT("Adding tri %d %d %d"), points[0], points[1], points[2]);
		ret.Append({ points[0], points[1], points[2], points[2], points[1], points[0] });
		f_dist = (sort_verts[points[1]] - sort_verts[points[0]]).Size() + (sort_verts[points[2]] - sort_verts[points[0]]).Size();
		s_dist = (sort_verts[points[0]] - sort_verts[points[1]]).Size() + (sort_verts[points[2]] - sort_verts[points[1]]).Size();
		if (f_dist < s_dist)
			points.RemoveAt(0);
		else
			points.RemoveAt(1);
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
	return GetNormals(num_v);
}
TArray<FVector> ASimpleVoxel::GetNormals(int n)
{
	TArray<FVector> ret_norms;
	for (int i = 0; i < num_v; i++)
		ret_norms.Emplace(FVector(0));
	return ret_norms;
}

TArray<FProcMeshTangent> ASimpleVoxel::GetTangents()
{
	return GetTangents(num_v);
}
TArray<FProcMeshTangent> ASimpleVoxel::GetTangents(int n)
{
	TArray<FProcMeshTangent> ret_tans;
	for (int i = 0; i < num_v; i++)
		ret_tans.Emplace(FProcMeshTangent());
	return ret_tans;
}

TArray<FLinearColor> ASimpleVoxel::GetColors()
{
	return GetColors(num_v);
}
TArray<FLinearColor> ASimpleVoxel::GetColors(int n)
{
	TArray<FLinearColor> ret_colors;
	for (int i = 0; i < num_v; i++)
		ret_colors.Emplace(FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand()));
	return ret_colors;
}

void ASimpleVoxel::CreateVoxel(FVector2D uv_center)
{
	cnt = 0;

	UE_LOG(LogTemp, Warning, TEXT("Triangle sets: %d"), voxel.tris.Num());
	for (TArray<int32> tri : voxel.tris) {
		mesh->SetMaterial(cnt, MyMaterial);
		voxel.uvs.Emplace(GetUV(voxel.verts, FVector2D((cnt % 6)*0.125 + 0.0625, ((cnt / 6) % 3)*0.125 + 0.0625), FVector2D(0.0625, 0.0625), FVector2D(33, 33)));
		mesh->CreateMeshSection_LinearColor(cnt, voxel.verts, tri, voxel.normals, voxel.uvs[cnt], voxel.colors, voxel.tans, false);
		cnt++;
	}

	
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
	CreateVoxel(FVector2D(.25, .25));
	
}

// Called every frame
void ASimpleVoxel::Tick(float DeltaTime)
{
	static float time = 0;
	
	FVector scale;

	Super::Tick(DeltaTime);

	time += DeltaTime;

	if (mesh_made) {
		scale = FVector(FMath::InterpSinInOut(1.0, 2.0, abs(sinf(time))));
		mesh->SetWorldScale3D(scale);
	}
}

