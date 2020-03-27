// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct FPoint
{
    uint32 Number;

    FVector Coordinate;

    TArray<FPoint*> Connections;

    explicit FPoint(FVector InFVector)
        : Coordinate(InFVector), Connections()
    {
    }
};

class FLine
{

public:
    FLine(FPoint* Start, FPoint* End, uint32 NumPoints);
    ~Fline();

    void SawPoint(const FPoint& SeenPoint);

    const bool HaveSeenAllPoints();

    void IncNumFacesConnected();
    const uint8 GetNumFacesConnected();

private:
    FPoint* StartPoint;
    FPoint* EndPoint;

    uint8 NumFacesConnected;

    TBitArray bPointsNotSeen;
};

class FFace
{
public:
    FFace(const TArray<FPoint*>& PointsList, const TArray<FLine*>& LineList);
    ~FFace();

    const bool IsExternalFace();

private:
    TArray<FPoint*> OrderedPoints;
    TArray<FLine*> Perimeter;
};
