// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "ParallelFor.h"




// Sets default values
ASimpleVoxel::ASimpleVoxel()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	trans = FVector(0);
	bounds = FVector(0);
	verts = { FVector(0) };
	grow = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	mesh->bUseAsyncCooking = false;
	RootComponent = mesh;


	static ConstructorHelpers::FObjectFinder<UMaterial> BlahMaterial(TEXT("Material'/Game/StarterContent/Materials/M_ColorGrid_LowSpec.M_ColorGrid_LowSpec'"));
	MyMaterial = BlahMaterial.Object;

}

void ASimpleVoxel::Init(TArray<FVector> verts, float scale, bool grow)
{
	this->verts = TArray<FVector>(verts);
	for (FVector& vert : this->verts)
		vert = scale * vert;
	this->grow = grow;
}

FVector ASimpleVoxel::GetBounds()
{
	return bounds;
}

TArray<TArray<FVector2D>> ASimpleVoxel::GetUVs()
{
	TArray<TArray<FVector2D>> ret;
	for (Face f : faces)
		ret.Emplace(f.GetUVs());
	return ret;
}

TArray<TArray<FVector>> ASimpleVoxel::GetVerts()
{
	TArray<TArray<FVector>> ret;
	for (Face f : faces)
		ret.Emplace(f.GetPoints());
	return ret;
}

TArray<TArray<FVector>> ASimpleVoxel::GetNormals()
{
	TArray<TArray<FVector>> ret;
	for (Face f : faces)
		ret.Emplace(f.GetNormals());
	return ret;
}

TArray<TArray<FProcMeshTangent>> ASimpleVoxel::GetTangents()
{
	TArray<TArray<FProcMeshTangent>> ret;
	for (Face f : faces)
		ret.Emplace(f.GetTangents());
	return ret;
}

TArray<TArray<FLinearColor>> ASimpleVoxel::GetColors()
{
	TArray<TArray<FLinearColor>> ret;
	for (Face f : faces)
		ret.Emplace(f.GetColors());
	return ret;
}

TArray<Face> ASimpleVoxel::GetFaces()
{
	return faces;
}

//a < b < c
//Indexes linear array of triangles from the three points defining the triangle
int ASimpleVoxel::TriFromI(int a, int b, int c) {
	return TriFromI(a, b, c, verts.Num());
}

int ASimpleVoxel::TriFromI(int a, int b, int c, int n) {
	if (a >= b || b >= c)
		return -1;
	return ((n - 2)*(n - 1)*n - (n - 2 - a)*(n - 1 - a)*(n - a)) / 6 + ((n - 2 - a)*(n - 1 - a) - (n - 1 - b)*(n - b)) / 2 + c - (b + 1);
}

void ASimpleVoxel::GenerateFaces()
{
	TArray<bool> checked_tri;
	int num_t;
	int32 num_v = verts.Num();

	num_t = num_v*(num_v - 1)*(num_v - 2) / 6;
	checked_tri.Init(false, num_t);


	TArray<int> in_plane;
	TArray<FVector> in_plane_p;
	FVector avg;
	FVector p_norm;
	bool pos, neg;
	float res;

	TArray<TArray<FVector>> points_arr;
	TArray<FVector> avg_arr;

	// Used for max distance a point can be away from a plane to be considered on plane
	float epsilon = 1;

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
					if (FMath::Abs(res) < epsilon)
						in_plane.Emplace(l);
					else if (res >= 0) pos = true;
					else if (res < 0) neg = true;

				}

				in_plane.Sort();
				
				ParallelFor(in_plane.Num(), [&](int l) {
					for (int m = l + 1; m < in_plane.Num(); m++) {
						for (int n = m + 1; n < in_plane.Num(); n++) {
							checked_tri[TriFromI(in_plane[l], in_plane[m], in_plane[n])] = true;
						}
					}
				}, false);

				if (pos && neg) {
					//UE_LOG(LogTemp, Warning, TEXT("Pos and neg true! %d %d %d"), i, j, k);
					continue;
				}

				in_plane_p.Empty();
				avg = FVector::ZeroVector;
				for (int i : in_plane) {
					in_plane_p.Emplace(verts[i]);
					avg += verts[i];
				}
				avg /= in_plane_p.Num();

				points_arr.Emplace(in_plane_p);
				avg_arr.Emplace(avg);
			}
		}
	}

	faces.Init(Face(), points_arr.Num());
	ParallelFor(faces.Num(), [&](int32 idx) {
		faces[idx] = Face(points_arr[idx], avg_arr[idx]);
	}, false);
	

}

/*
	One of these next few doesn't really work. This one works fine for getting the uvs mapped from a grid material
	where you know the number of rows/cols of the grid, the width of a single item in the grid, and the max values
	the x and y vectors can take
*/
void ASimpleVoxel::GenerateUVs(FVector2D uv_range, FVector2D point_range, int row, int col)
{
	int cnt = 0;
	FVector2D center = FVector2D();

	for (Face& face : faces) {
		center = FVector2D((cnt % col)*uv_range.X, ((cnt / col) % row)*uv_range.Y);
		TArray<FVector2D> uvs_tmp;
		TArray<FVector> face_pts = face.GetPoints();
		for (FVector pt : face_pts) {
			uvs_tmp.Emplace(FVector2D((FMath::Abs(pt.X) / point_range.X)*uv_range.X + center.X,
									  (FMath::Abs(pt.Y) / point_range.Y)*uv_range.Y + center.Y));
		}
		face.SetUVs(uvs_tmp);
		++cnt;
	}
}

void ASimpleVoxel::GenerateNormalsAndTans()
{
	ParallelFor(faces.Num(), [&](int32 i) {
		TArray<FProcMeshTangent> tans_tmp = TArray<FProcMeshTangent>();
		TArray<FVector> norms_tmp = TArray<FVector>();
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(faces[i].GetPoints(), faces[i].GetTris(), faces[i].GetUVs(), norms_tmp, tans_tmp);
		faces[i].SetNormals(norms_tmp);
		faces[i].SetTangents(tans_tmp);
	}, false);
}

//Colors don't even seem to be used, so I just throw random colors on the points
//Maybe this is related to vertex painting, but I doubt it
void ASimpleVoxel::GenerateColors()
{
	ParallelFor(faces.Num(), [&](int32 i) {
		TArray<FLinearColor> colors_tmp = TArray<FLinearColor>();
		for (int i = 0; i < faces[i].NumPoints(); ++i)
			colors_tmp.Emplace(FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand()));
		faces[i].SetColors(colors_tmp);
	}, false);
}

void ASimpleVoxel::CreateVoxel()
{
	GenerateUVs(FVector2D(0.125, 0.125), FVector2D(FMath::Abs(trans.X)+1, FMath::Abs(trans.Y)+1), 3, 6);
	GenerateColors();
	GenerateNormalsAndTans();

	mesh->bUseComplexAsSimpleCollision = false;

	for (int i = 0; i < faces.Num(); i++) {
		mesh->SetMaterial(i, MyMaterial);
		mesh->CreateMeshSection_LinearColor(i, faces[i].GetPoints(), faces[i].GetTris(), faces[i].GetNormals(),
											   faces[i].GetUVs(), faces[i].GetColors(), faces[i].GetTangents(), true);
	}
	
	//These are all needed in order to have physics work on the object
	mesh->AddCollisionConvexMesh(verts);

	mesh->SetSimulatePhysics(true);
	mesh->SetEnableGravity(true);
}

// Called when the game starts or when spawned
void ASimpleVoxel::BeginPlay()
{
	Super::BeginPlay();

	// Get a vector of numbers that when subtracted from every vert, causes all verts to be > 0 in every dimension
	for (FVector v : verts) {
		if (v.X < trans.X)
			trans.X = v.X;
		if (v.Y < trans.Y)
			trans.Y = v.Y;
		if (v.Z < trans.Z)
			trans.Z = v.Z;

		if (v.X > bounds.X)
			bounds.X = v.X;
		if (v.Y > bounds.Y)
			bounds.Y = v.Y;
		if (v.Z > bounds.Z)
			bounds.Z = v.Z;

	}

	bounds -= trans;

	GenerateFaces();
	CreateVoxel();
	
}

// Called every frame
void ASimpleVoxel::Tick(float DeltaTime)
{
	static float time = 0;

	FVector scale;

	Super::Tick(DeltaTime);

	time += DeltaTime;

	// Scaling an object scales its size. Who knew? Not I
	if (grow) {
		scale = FVector(FMath::InterpSinInOut(0.5, 2.0, abs(sinf(time))));
		mesh->SetWorldScale3D(scale);
	}
}

