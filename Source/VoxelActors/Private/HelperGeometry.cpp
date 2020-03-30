// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperGeometry.h"

FLine::FLine(TSharedRef<FPoint> Start, TSharedRef<FPoint> End, uint32 NumPoints) :
    StartPoint(Start), EndPoint(End), NumFacesConnected(0),
    bPointsNotSeen(TBitArray<FDefaultBitArrayAllocator>(true, NumPoints))
{
    bPointsNotSeen[StartPoint->Number] = false;
    bPointsNotSeen[EndPoint->Number] = false;
}

void FLine::SawPoint(TSharedRef<FPoint> SeenPoint)
{
    bPointsNotSeen[SeenPoint->Number] = false;
}

void FLine::SawPoints(TArray<TSharedRef<FPoint>> SeenPoints)
{
    for( TSharedRef<FPoint> Point : SeenPoints) {
        bPointsNotSeen[Point->Number] = false;
    }
}

bool FLine::HasSeenPoint(TSharedRef<FPoint> CheckPoint)
{
    return !bPointsNotSeen[CheckPoint->Number];
}

bool FLine::HaveSeenAllPoints()
{
    return (bPointsNotSeen.Find(true) == INDEX_NONE);
}

void FLine::IncNumFacesConnected()
{
    // We don't want a wraparound. Only really care if NFC > 2, as then there is probably
    // An internal face connected to this edge
    NumFacesConnected = NumFacesConnected==UINT8_MAX ? UINT8_MAX : NumFacesConnected+1;
}

uint8 FLine::GetNumFacesConnected()
{
    return NumFacesConnected;
}

FLine::~FLine()
{

}

FFace::FFace(TArray< TSharedRef<FPoint>> PointsList,  TArray<TSharedRef<FLine>> LinesList) :
    OrderedPoints(PointsList), Perimeter(LinesList)
{

}

bool FFace::IsExternalFace()
{
    for (TSharedPtr<FLine> Segment : Perimeter){
        if (Segment->GetNumFacesConnected() > 2)
            return false;
    }
    return true;
}

TArray<TSharedRef<FPoint>> FFace::GetOrderedPoints()
{
    return OrderedPoints;
}

FFace::~FFace()
{

}
