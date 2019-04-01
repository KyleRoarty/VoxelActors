// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVoxel.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"




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

void ASimpleVoxel::SetVerts(TArray<FVector> verts, float scale, bool grow)
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
	return uvs;
}

TArray<TArray<FVector>> ASimpleVoxel::GetVerts()
{
	return verts_arr;
}

TArray<TArray<FVector>> ASimpleVoxel::GetNormals()
{
	return normals;
}

TArray<TArray<FProcMeshTangent>> ASimpleVoxel::GetTangents()
{
	return tans;
}

TArray<TArray<FLinearColor>> ASimpleVoxel::GetColors()
{
	return colors;
}

TArray<TArray<int32>> ASimpleVoxel::GetFaces()
{
	return face_i;
}

//a < b < c
//Indexes linear array of triangles from the three points defining the triangle
int ASimpleVoxel::TriFromI(int a, int b, int c) {
	return TriFromI(a, b, c, num_v);
}

int ASimpleVoxel::TriFromI(int a, int b, int c, int n) {
	if (a >= b || b >= c)
		return -1;
	return ((n - 2)*(n - 1)*n - (n - 2 - a)*(n - 1 - a)*(n - a)) / 6 + ((n - 2 - a)*(n - 1 - a) - (n - 1 - b)*(n - b)) / 2 + c - (b + 1);
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
		norm = FVector::CrossProduct(sort_verts[idxs[1]] - sort_verts[idxs[0]], sort_verts[idxs[2]] - sort_verts[idxs[0]]).GetSafeNormal();
		norm.Normalize();
		/*	Only use the triplet that is facing outward
			sort_verts are all >= 0
			Need to normalize the point so that it's centered around 0,0,0
			one of norm_point + norm and norm_point - norm will be closer to 0,0,0
			The triangle of the one closer to 0,0,0 will be facing inwards
			We want the triangle that faces outwards, so we choose the one that is larger

			**Note: If this stops working, take the average of the points and use that as the point we compare against
			**Note 2: It is doubtful that this will work with concave shapes
		*/ 
		if ((sort_verts[idxs[0]] - avg_sort_vert + norm).Size() >= (sort_verts[idxs[0]] - avg_sort_vert - norm).Size())
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

void ASimpleVoxel::GenerateFaces()
{
	TArray<bool> checked_tri;
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
					//UE_LOG(LogTemp, Warning, TEXT("%d"), i);

				face_i.Emplace(in_plane);
			}
		}
	}
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
	uvs.Init(TArray<FVector2D>(), verts_arr.Num());

	for (int i = 0; i < verts_arr.Num(); ++i) {
		center = FVector2D((cnt % col)*uv_range.X, ((cnt / col) % row)*uv_range.Y);
		TArray<FVector2D> uvs_tmp;
		for (int j = 0; j < verts_arr[i].Num(); ++j) {
			uvs_tmp.Emplace(FVector2D((FMath::Abs(verts_arr[i][j].X) / point_range.X)*uv_range.X + center.X,
									  (FMath::Abs(verts_arr[i][j].Y) / point_range.Y)*uv_range.Y + center.Y));
		}
		uvs[i] = TArray<FVector2D>(uvs_tmp);
		++cnt;
	}
}

void ASimpleVoxel::GenerateVerts()
{
	verts_arr.Init(TArray<FVector>(), face_i.Num());

	for (int i = 0; i < face_i.Num(); ++i) {
		TArray<FVector> verts_arr_tmp;
		for (int j = 0; j < face_i[i].Num(); ++j)
			verts_arr_tmp.Emplace(verts[face_i[i][j]]);
		verts_arr[i] = TArray<FVector>(verts_arr_tmp);
	}
}

void ASimpleVoxel::GenerateNormalsAndTans()
{
	tans.Init(TArray<FProcMeshTangent>(), verts_arr.Num());
	normals.Init(TArray<FVector>(), verts_arr.Num());
	for (int i = 0; i < verts_arr.Num(); ++i)
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(verts_arr[i], face_t[i], uvs[i], normals[i], tans[i]);
}

//Colors don't even seem to be used, so I just throw random colors on the points
//Maybe this is related to vertex painting, but I doubt it
void ASimpleVoxel::GenerateColors()
{
	colors.Init(TArray<FLinearColor>(), verts_arr.Num());

	for (int i = 0; i < verts_arr.Num(); ++i) {
		TArray<FLinearColor> colors_tmp = TArray<FLinearColor>();
		for (int j = 0; j < verts_arr[i].Num(); ++j)
			colors_tmp.Emplace(FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand()));
		colors[i] = TArray<FLinearColor>(colors_tmp);
	}
}

void ASimpleVoxel::CreateVoxel()
{
	// Once you have the faces-as-indices, you can get the vertices needed
	// as well as the faces-as-triangles
	for (TArray<int32> face : face_i) {
		face_t.Emplace(SimpleTris(face));
	}
	GenerateVerts();

	GenerateUVs(FVector2D(0.125, 0.125), FVector2D(FMath::Abs(trans.X)+1, FMath::Abs(trans.Y)+1), 3, 6);
	GenerateColors();
	GenerateNormalsAndTans();

	mesh->bUseComplexAsSimpleCollision = false;

	for (int i = 0; i < verts_arr.Num(); i++) {
		mesh->SetMaterial(i, MyMaterial);
		mesh->CreateMeshSection_LinearColor(i, verts_arr[i], face_t[i], normals[i], uvs[i], TArray<FLinearColor>(), tans[i], true);
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

	// Order from closest to furthest away from origin
	verts.Sort([this](const FVector A,  const FVector B){
		return (A-this->trans).Size() < (B-this->trans).Size();
	});

	for (FVector v : verts) {
		sort_verts.Emplace(v - trans);
	}

	avg_sort_vert = FVector(0);
	for (FVector sv : sort_verts) {
		avg_sort_vert += sv;
	}
	avg_sort_vert /= sort_verts.Num();

	num_v = verts.Num();
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

