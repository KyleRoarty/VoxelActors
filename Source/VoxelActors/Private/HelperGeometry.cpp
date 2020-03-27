// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperGeometry.h"

FLine::FLine(FPoint* Start, FPoint* End, uint32 NumPoints) :
    StartPoint(Start), EndPoint(End), NumFacesConnected(0), bPointsNotSeen(TBitArray(true, NumPoints))
{
    bPointsNotSeen[StartPoint->Number] = false;
    bPointsNotSeen[EndPoint->Number] = false;
}

void FLine::SawPoint(const FPoint &SeenPoint)
{
    bPointsNotSeen[SeenPoint.Number] = false;
}

bool FLine::HaveSeenAllPoints() const
{
    return static_cast<bool>(bPointsNotSeen.GetData());
}

void FLine::IncNumFacesConnected()
{
    // We don't want a wraparound. Only really care if NFC > 2, as then there is probably
    // An internal face connected to this edge
    NumFacesConnected = NumFacesConnected==UINT8_MAX ? UINT8_MAX : NumFacesConnected++;
}

uint8 FLine::GetNumFacesConnected() const
{
    return NumFacesConnected;
}

FLine::~Fline()
{

}

FFace::FFace(const TArray<FPoint*>& PointsList, const TArray<FLine*>& LinesList) :
    OrderedPoints(PointsList), Perimeter(LinesList)
{

}

bool FFace::IsExternalFace() const
{
    for (const FLine* Segment : Perimeter){
        if (Segment->GetNumFacesConnected() > 2)
            return false;
    }
    return true;
}

FFace::~FFace()
{

}
