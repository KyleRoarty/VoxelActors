// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct FPoint
{
    int32 Number;

    FVector Coordinate;

    TArray<TSharedRef<FPoint>> Connections;

    explicit FPoint(FVector InFVector, uint32 Number)
        : Number(Number), Coordinate(InFVector), Connections()
    {
    }
};

class FLine
{

public:
    FLine(TSharedRef<FPoint> Start, TSharedRef<FPoint> End, uint32 NumPoints);
    ~FLine();

    void SawPoint(TSharedRef<FPoint> SeenPoint);
    void SawPoints(TArray<TSharedRef<FPoint>> SeenPoints);

    bool HasSeenPoint(TSharedRef<FPoint> CheckPoint) ;

    bool HaveSeenAllPoints() ;

    void IncNumFacesConnected();
    uint8 GetNumFacesConnected() ;

private:
    TSharedRef<FPoint> StartPoint;
    TSharedRef<FPoint> EndPoint;

    uint8 NumFacesConnected;

    TBitArray<FDefaultBitArrayAllocator> bPointsNotSeen;
};

class FFace
{
public:
    FFace(TArray<TSharedRef<FPoint>> PointsList,  TArray<TSharedRef<FLine>> LineList);
    ~FFace();

    bool IsExternalFace() ;
    TArray< TSharedRef<FPoint>> GetOrderedPoints();

private:
    TArray<TSharedRef<FPoint>> OrderedPoints;
    TArray<TSharedRef<FLine>> Perimeter;
};
