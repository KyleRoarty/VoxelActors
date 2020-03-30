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
	grow = false;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
    mesh->bUseAsyncCooking = true;
	RootComponent = mesh;


	static ConstructorHelpers::FObjectFinder<UMaterial> BlahMaterial(TEXT("Material'/Game/StarterContent/Materials/M_ColorGrid_LowSpec.M_ColorGrid_LowSpec'"));
	MyMaterial = BlahMaterial.Object;

}

void ASimpleVoxel::SetVerts(const TArray<FVector>& verts_, const float& scale, const bool& grow_)
{
    uint32 index = 0;
    for (const FVector& vert : verts_){
       Points.Emplace(TSharedRef<FPoint>(new FPoint(scale * vert, index)));
       PointsPosition.Emplace(scale * vert);
       ++index;
    }
    this->grow = grow_;
}

void ASimpleVoxel::SetEdges(const TArray<TTuple<int, int>>& edges)
{
    for (const TTuple<int, int>& edge : edges) {
        uint32 StartPoint = edge.Get<0>();
        uint32 EndPoint = edge.Get<1>();
        auto Key = TBitArray<FDefaultBitArrayAllocator>(false, Points.Num());
        Key[StartPoint] = true;
        Key[EndPoint] = true;


        Points[StartPoint]->Connections.Emplace(Points[EndPoint]);
        Points[EndPoint]->Connections.Emplace(Points[StartPoint]);
        LineSegments.Add(Key, TSharedRef<FLine>(new FLine(Points[StartPoint], Points[EndPoint], Points.Num())));
    }
}

FVector ASimpleVoxel::GetBounds()
{
	return bounds;
}

TArray<TArray<FVector2D>> ASimpleVoxel::GetUVs()
{
	return uvs;
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

TBitArray<FDefaultBitArrayAllocator> ASimpleVoxel::MakeSegmentKey(TSharedRef<FPoint>& Start,
                                                                  TSharedRef<FPoint>& End)
{
    auto Key = TBitArray<FDefaultBitArrayAllocator>(false, Points.Num());
    Key[Start->Number] = true;
    Key[End->Number] = true;

    return Key;
}

void ASimpleVoxel::GenerateFaces()
{
    TArray<FFace> TmpFaces;
    for (TSharedRef<FPoint> Point : Points) {
        for (TSharedRef<FPoint> Connection : Point->Connections) {
            // Verify segment still has points to search through
            auto InitialSegKey = MakeSegmentKey(Point, Connection);
            TSharedRef<FLine> InitialSeg = LineSegments[InitialSegKey];

            if (InitialSeg->HaveSeenAllPoints())
                continue;

            TQueue<TArray<TSharedRef<FPoint>>> BfsQueue;
            do{
                BfsQueue.Empty();
                bool bFoundFace = false;

                for (TSharedRef<FPoint> Adjacent1 : Connection->Connections) {
                    if (!InitialSeg->HasSeenPoint(Adjacent1)){
                        BfsQueue.Enqueue({Point, Connection, Adjacent1});
                    }
                }

                // Start BFS
                while(!BfsQueue.IsEmpty() && !bFoundFace){
                    TArray<TSharedRef<FPoint>> Path;
                    BfsQueue.Dequeue(Path);

                    TSharedRef<FPoint> Leaf = Path.Last();
                    for (TSharedRef<FPoint> Adjacent : Leaf->Connections){
                        if (Adjacent == Point){
                            // Cycle, do the thing
                            TArray<TSharedRef<FLine>> FacePerimeter;

                            for(int i = 0; i < Path.Num() - 1; i++){
                                auto SegKey = MakeSegmentKey(Path[i], Path[i+1]);
                                FacePerimeter.Emplace(LineSegments[SegKey]);
                            }
                            // Also do wraparound segment
                            {
                                auto SegKey = MakeSegmentKey(Path[0], Path[Path.Num()-1]);
                                FacePerimeter.Emplace(LineSegments[SegKey]);
                            }

                            for(TSharedRef<FLine> FaceSegment : FacePerimeter){
                                FaceSegment->SawPoints(Path);
                                FaceSegment->IncNumFacesConnected();
                            }

                            TmpFaces.Emplace(FFace(Path, FacePerimeter));
                            bFoundFace = true;
                            break;
                        } else if (Path.Find(Adjacent) != INDEX_NONE){
                            // Don't add 'parent' in this path back into the list
                            continue;
                        } else if (InitialSeg->HasSeenPoint(Adjacent)){
                            // Already used this point, don't use it again
                            continue;
                        } else {
                            TArray<TSharedRef<FPoint>> NewPath = Path;
                            NewPath.Emplace(Adjacent);
                            BfsQueue.Enqueue(NewPath);
                        }

                    }
                }
            } while(!InitialSeg->HaveSeenAllPoints() && !BfsQueue.IsEmpty());
        }
    }

    for(FFace Face : TmpFaces){
        if (Face.IsExternalFace()){
            Faces.Emplace(TSharedRef<FFace>(new FFace(Face)));
        }
    }

}

void ASimpleVoxel::Triangulate()
{
    for(TSharedRef<FFace> Face : Faces){
        TArray<TSharedRef<FPoint>> TmpPoints = Face->GetOrderedPoints();
        face_t.Emplace(TArray<int32>{TmpPoints[1]->Number, TmpPoints[2]->Number, TmpPoints[0]->Number,
                                     TmpPoints[1]->Number, TmpPoints[0]->Number, TmpPoints[2]->Number,
                                     TmpPoints[2]->Number, TmpPoints[0]->Number, TmpPoints[3]->Number,
                                     TmpPoints[2]->Number, TmpPoints[3]->Number, TmpPoints[0]->Number});
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
    uvs.Init(TArray<FVector2D>(), Faces.Num());

    for (int i = 0; i < Faces.Num(); ++i) {
		center = FVector2D((cnt % col)*uv_range.X, ((cnt / col) % row)*uv_range.Y);
		TArray<FVector2D> uvs_tmp;
        for (int j = 0; j < PointsPosition.Num(); ++j) {
            uvs_tmp.Emplace(FVector2D((FMath::Abs(PointsPosition[j].X) / point_range.X)*uv_range.X + center.X,
                                      (FMath::Abs(PointsPosition[j].Y) / point_range.Y)*uv_range.Y + center.Y));
		}
		uvs[i] = TArray<FVector2D>(uvs_tmp);
		++cnt;
	}
}

void ASimpleVoxel::GenerateNormalsAndTans()
{
    tans.Init(TArray<FProcMeshTangent>(), Faces.Num());
    normals.Init(TArray<FVector>(), Faces.Num());
    for (int i = 0; i < Faces.Num(); ++i)
        UKismetProceduralMeshLibrary::CalculateTangentsForMesh(PointsPosition, face_t[i], uvs[i], normals[i], tans[i]);
}

//Colors don't even seem to be used, so I just throw random colors on the points
//Maybe this is related to vertex painting, but I doubt it
void ASimpleVoxel::GenerateColors()
{
    colors.Init(TArray<FLinearColor>(), Faces.Num());

    for (int i = 0; i < Faces.Num(); ++i) {
		TArray<FLinearColor> colors_tmp = TArray<FLinearColor>();
        for (int j = 0; j < PointsPosition.Num(); ++j)
			colors_tmp.Emplace(FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand()));
		colors[i] = TArray<FLinearColor>(colors_tmp);
	}
}

void ASimpleVoxel::CreateVoxel()
{
	GenerateUVs(FVector2D(0.125, 0.125), FVector2D(FMath::Abs(trans.X)+1, FMath::Abs(trans.Y)+1), 3, 6);
    //GenerateColors();
	GenerateNormalsAndTans();


    for (int i = 0; i < Faces.Num(); i++) {
		mesh->SetMaterial(i, MyMaterial);
        mesh->CreateMeshSection_LinearColor(i, PointsPosition, face_t[i], normals[i], uvs[i], TArray<FLinearColor>(), tans[i], false);
        //mesh->CreateMeshSection_LinearColor(i, PointsPosition, face_t[i], TArray<FVector>(), uvs[i], TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
    }
	
	//These are all needed in order to have physics work on the object
    mesh->bUseComplexAsSimpleCollision = false;
    mesh->AddCollisionConvexMesh(PointsPosition);

    mesh->SetSimulatePhysics(true);
    mesh->SetEnableGravity(true);
}

// Called when the game starts or when spawned
void ASimpleVoxel::BeginPlay()
{
	Super::BeginPlay();

	// Get a vector of numbers that when subtracted from every vert, causes all verts to be > 0 in every dimension
    for (FVector v : PointsPosition) {
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
    Triangulate();
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

