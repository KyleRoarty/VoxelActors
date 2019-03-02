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

	trans = FVector(0);
	verts = { 20*FVector(-1,-1,-1), 20*FVector(1,-1,-1), 20*FVector(1,1,-1), 20*FVector(-1,1,-1), 20*FVector(-1,-1,1), 20*FVector(1,-1,1), 20*FVector(1,1,1), 20*FVector(-1,1,1) };
	verts = { 20*FVector(-1,-1,-1), 20*FVector(1,-1,-1), 20*FVector(0,0,-1), 20*FVector(-1,-1,1), 20*FVector(1,-1,1), 20*FVector(0,0,1) };
	verts = { FVector(0,-20,0), FVector(-19,-6,0), FVector(-12,16,0), FVector(12,16,0), FVector(19,-6,0), FVector(0,-20,20), FVector(-19,-6,20), FVector(-12,16,20), FVector(12,16,20), FVector(19,-6,20) };
	/*verts = { 20 * FVector(1,1,1), 20 * FVector(1,1,-1), 20 * FVector(1,-1,1), 20 * FVector(1,-1,-1), 20 * FVector(-1,1,1), 20 * FVector(-1,1,-1), 20 * FVector(-1,-1,1), 20 * FVector(-1,-1,-1),
			  20 * FVector(0,PHI,IPHI), 20 * FVector(0,PHI,-IPHI), 20 * FVector(0,-PHI,IPHI),20 * FVector(0,-PHI,-IPHI),
			  20 * FVector(IPHI,0,PHI), 20 * FVector(IPHI,0,-PHI),20 * FVector(-IPHI,0,PHI),20 * FVector(-IPHI,0,-PHI),
			  20 * FVector(PHI,IPHI,0),20 * FVector(PHI,-IPHI,0),20 * FVector(-PHI,IPHI,0),20 * FVector(-PHI,-IPHI, 0) };*/
	/*verts = { 20 * FVector(0,1,PHI), 20 * FVector(0,1,-PHI), 20 * FVector(0,-1,PHI), 20 * FVector(0,-1,-PHI),
			  20 * FVector(1,PHI,0), 20 * FVector(1,-PHI,0), 20 * FVector(-1,PHI,0), 20 * FVector(-1,-PHI,0),
			  20 * FVector(PHI,0,1), 20 * FVector(PHI,0,-1), 20 * FVector(-PHI,0,1), 20 * FVector(-PHI,0,-1)};*/

	// Get a vector of numbers that when subtracted from every vert, causes all verts to be > 0 in every dimension
	for (FVector v : verts) {
		if (v.X < trans.X)
			trans.X = v.X;
		if (v.Y < trans.Y)
			trans.Y = v.Y;
		if (v.Z < trans.Z)
			trans.Z = v.Z;
	}

	// Order from closest to furthest away from origin
	verts.Sort([this](const FVector A,  const FVector B){
		return (A-this->trans).Size() < (B-this->trans).Size();
	});

	for (FVector v : verts) {
		sort_verts.Emplace(v - trans);
	}

	num_v = verts.Num();
	mesh_made = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	mesh->bUseAsyncCooking = true;
	RootComponent = mesh;

	voxel.face_i = GetFaces();

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

TArray<TArray<int32>> ASimpleVoxel::GetFaces()
{
	TArray<bool> checked_tri;
	TArray<TArray<int32>> ret_face_i;
	int num_t;
	

	num_t = num_v*(num_v - 1)*(num_v - 2) / 6;
	checked_tri.Init(false, num_t);


	TArray<int> in_plane;
	FVector p_norm;
	bool pos, neg;
	float res;

	/*
		Generates a plane from all triplets of verts
		Checks if all of the verts are on one side of the plane (Including the plane itself)
		If they are, that plane is an outer plane on the polyhedral
		Ends up returning all faces and the indices associated with the verts on the faces
	*/
	for (int i = 0; i < num_v; i++) {
		for (int j = i + 1; j < num_v; j++) {
			for (int k = j + 1; k < num_v; k++) {
				// Speed up loop by ignoring triplets that were on previous faces
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

				for (int i : in_plane)
					UE_LOG(LogTemp, Warning, TEXT("%d"), i);

				ret_face_i.Emplace(in_plane);
			}
		}
	}

	return ret_face_i;
}

//Generates triangle triplets from indexes on a face passed in
TArray<int32> ASimpleVoxel::SimpleTris(TArray<int32> idxs) {
	TArray<int32> ret;

	TArray<int32> p_idxs;

	for (int i = 0; i < idxs.Num(); i++)
		p_idxs.Emplace(i);

	//First, second distances
	float f_dist, s_dist;
	FVector norm;

	while (idxs.Num() >= 3) {
		//ret.Append({ p_idxs[0], p_idxs[1], p_idxs[2], p_idxs[2], p_idxs[1], p_idxs[0] });
		ret.Append({ p_idxs[0], p_idxs[1], p_idxs[2] });
		norm = FVector::CrossProduct(sort_verts[idxs[1]] - sort_verts[idxs[0]], sort_verts[idxs[2]] - sort_verts[idxs[0]]).GetSafeNormal();
		// Meant to make it so that only one triplet needs to be appended, not both; Unsure if it works perfectly
		if (FVector::DotProduct(norm, FVector(0)) >= 0)
			ret.Append({ p_idxs[2], p_idxs[1], p_idxs[0] });
		else
			ret.Append({ p_idxs[0], p_idxs[1], p_idxs[2] });

		//Either the first or second point gets removed based off of which is closer to the third, as that one gets "covered up" by the other two forming a line segment
		f_dist = (sort_verts[idxs[2]] - sort_verts[idxs[0]]).Size();
		s_dist = (sort_verts[idxs[2]] - sort_verts[idxs[1]]).Size();
		if (f_dist < s_dist) {
			p_idxs.RemoveAt(0);
			idxs.RemoveAt(0);
		}
		else {
			p_idxs.RemoveAt(1);
			idxs.RemoveAt(1);
		}
	}

	return ret;
}

TArray<FVector> ASimpleVoxel::GetVerts(TArray<int32> idx)
{
	TArray<FVector> ret;
	
	for (int i : idx)
		ret.Emplace(verts[i]);

	return ret;
}

/*
	One of these next few doesn't really work. This one works fine for getting the uvs mapped from a grid material
	where you know the number of rows/cols of the grid, the width of a single item in the grid, and the max values
	the x and y vectors can take
*/
TArray<FVector2D> ASimpleVoxel::GetUV(TArray<FVector> pos, FVector2D uv_range, FVector2D point_range, int row, int col)
{
	static int cnt = 0;
	TArray<FVector2D> ret_uvs;

	FVector2D center = FVector2D((cnt % col)*uv_range.X, ((cnt / col) % row)*uv_range.Y);

	for (FVector p : pos)
		ret_uvs.Emplace(FVector2D((FMath::Abs(p.X) / point_range.X)*uv_range.X + center.X, (FMath::Abs(p.Y) / point_range.Y)*uv_range.Y + center.Y));

	cnt++;

	return ret_uvs;
}

//Totally works, right?
TArray<FVector> ASimpleVoxel::GetNormals(TArray<FVector> points)
{
	TArray<FVector> ret_norms;
	FVector norm = FVector::CrossProduct(points[1] - points[0], points[2] - points[0]).GetSafeNormal();

	for (int i = 0; i < points.Num(); i++)
		ret_norms.Emplace(norm);

	return ret_norms;
}

//Again, totally works, right?
TArray<FProcMeshTangent> ASimpleVoxel::GetTangents(TArray<FVector> points)
{
	TArray<FProcMeshTangent> ret_tans;
	for (int i = 0; i < points.Num(); i++)
		ret_tans.Emplace(FProcMeshTangent((points[0] - points[1]).GetSafeNormal(), true));
	return ret_tans;
}

//Colors don't even seem to be used, so I just throw random colors on the points
//Maybe this is related to vertex painting, but I doubt it
TArray<FLinearColor> ASimpleVoxel::GetColors(TArray<FVector> points)
{
	TArray<FLinearColor> ret_colors;
	for (int i = 0; i < points.Num(); i++)
		ret_colors.Emplace(FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand()));
	return ret_colors;
}

void ASimpleVoxel::CreateVoxel(FVector2D uv_center)
{
	cnt = 0;

	UE_LOG(LogTemp, Warning, TEXT("Triangle sets: %d"), voxel.face_t.Num());

	// Once you have the faces-as-indices, you can get the vertices needed
	// as well as the faces-as-triangles
	for (TArray<int32> face : voxel.face_i) {
		voxel.face_t.Emplace(SimpleTris(face));
		voxel.verts.Emplace(GetVerts(face));
	}

	// Get the rest of the parameters that may or may not be needed
	for (TArray<FVector> v : voxel.verts) {
		voxel.uvs.Emplace(GetUV(v, FVector2D(0.125, 0.125), FVector2D(FMath::Abs(trans.X)+1, FMath::Abs(trans.Y)+1), 3, 6));
		voxel.normals.Emplace(GetNormals(v));
		voxel.tans.Emplace(GetTangents(v));
		voxel.colors.Emplace(GetColors(v));
	}
	
	//Do this for the kismet functions
	voxel.tans.Init(TArray<FProcMeshTangent>(), voxel.verts.Num());
	voxel.normals.Init(TArray<FVector>(), voxel.verts.Num());

	for (int i = 0; i < voxel.verts.Num(); i++) {
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(voxel.verts[i], voxel.face_t[i], voxel.uvs[i], voxel.normals[i], voxel.tans[i]);
		mesh->SetMaterial(i, MyMaterial);
		mesh->CreateMeshSection_LinearColor(i, voxel.verts[i], voxel.face_t[i], voxel.normals[i], voxel.uvs[i], TArray<FLinearColor>(), voxel.tans[i], false);
	}

	
	//These are all needed in order to have physics work on the object
	mesh->bUseComplexAsSimpleCollision = false;
	mesh->AddCollisionConvexMesh(verts);

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
	
	mesh_made = false;

	FVector scale;

	Super::Tick(DeltaTime);

	time += DeltaTime;

	// Scaling an object scales its size. Who knew? Not I
	if (mesh_made) {
		scale = FVector(FMath::InterpSinInOut(1.0, 2.0, abs(sinf(time))));
		mesh->SetWorldScale3D(scale);
	}
}

