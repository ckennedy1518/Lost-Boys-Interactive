// Fill out your copyright notice in the Description page of Project Settings.


#include "PGMaze.h"
#include "Math/IntPoint.h"
#include "Kismet/GameplayStatics.h"
#include "PGPlayerController.h"

// Sets default values
APGMaze::APGMaze()
{
	FloorMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorMesh"));
	WallMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallMesh"));

	RootComponent = FloorMesh;
	WallMesh->SetupAttachment(FloorMesh);

	MazeSize = FIntPoint(20, 20);
	Seed = 0;
	TileSize = 100.f;
	MazeChunkOffset.X = 0;
	MazeChunkOffset.Y = 0;
	AddSoStartIsZeroZero.X = 25;
	AddSoStartIsZeroZero.Y = 25;
	OneBlockNoiseChance = 25;
	TwoBlocksNoiseChance = 10;
	ThreeBlocksNoiseChance = 5;
	ScaleUnitNavMesh = 20;
}

// Called when the game starts or when spawned
void APGMaze::BeginPlay()
{
	Super::BeginPlay();
}


void APGMaze::NewMaze(int32 SeedForGeneration, FIntPoint Offset, TArray<FIntPoint> PointsToConnect)
{
	PointsConnecting = PointsToConnect; // this is then exposed for when we are checking to add walls

	FIndexAndLengthInMap Temp = FIndexAndLengthInMap();
	Temp.StartingIndexMeshes = -1; // to tell when we're setting the first index
	Temp.LengthOfIndexes = 0; // to keep track of how many instances have been added to viewport
	FloorInstancesMap.Add(Offset, Temp);
	WallInstancesMap.Add(Offset, Temp);

	Seed = SeedForGeneration;
	FRandomStream RandStream(Seed);
	MazeChunkOffset = Offset * 2000; // 4000 / 2

	TArray<FIntPoint> ToAdd = TArray<FIntPoint>();
	for (FIntPoint Point : PointsToConnect)
	{ // this for loop connects the two points across from each other, gives some more options and a more interesting maze IMO
		if (Point.X == 0)
		{
			for (FIntPoint Point2 : PointsToConnect)
			{
				if (Point2.X == MazeSize.X - 1)
				{
					ToAdd.Append(GetLineBetweenTwoPoints(RandStream, Point, Point2));
				}
			}
		}
		if (Point.Y == 0)
		{
			for (FIntPoint Point2 : PointsToConnect)
			{
				if (Point2.Y == MazeSize.Y - 1)
				{
					ToAdd.Append(GetLineBetweenTwoPoints(RandStream, Point, Point2));
				}
			}
		}
	}
	
	for (FIntPoint TileLoc : ToAdd)
	{ // for loop sets floors and tiles
		AddFloor(TileLoc);

		bool North = false;
		bool East = false;
		bool South = false;
		bool West = false;
		GetNeighborsAround(TileLoc, ToAdd, North, East, South, West);

		if (!North)
		{
			AddNorthWall(TileLoc);
		}
		if (!East)
		{
			AddEastWall(TileLoc);
		}
		if (!South)
		{
			AddSouthWall(TileLoc);
		}
		if (!West)
		{
			AddWestWall(TileLoc);
		}
	}
}


void APGMaze::GetNeighborsAround(FIntPoint CurrentLocation, TArray<FIntPoint> AllTiles, bool& North, bool& East, bool& South, bool& West)
{
	if (PointsConnecting.Contains(CurrentLocation))
	{
		if (CurrentLocation.X == MazeSize.X - 1)
		{
			North = true;
		}
		if (CurrentLocation.X == 0)
		{
			South = true;
		}
		if (CurrentLocation.Y == MazeSize.Y - 1)
		{
			West = true;
		}
		if (CurrentLocation.Y == 0)
		{
			East = true;
		}
	}

	for (FIntPoint Tile : AllTiles)
	{
		if (Tile.X == CurrentLocation.X + 1 && Tile.Y == CurrentLocation.Y)
		{ // has neighbor to the north
			North = true;
		}
		if (Tile.X == CurrentLocation.X && Tile.Y == CurrentLocation.Y - 1)
		{
			East = true;
		}
		if (Tile.X == CurrentLocation.X - 1 && Tile.Y == CurrentLocation.Y)
		{
			South = true;
		}
		if (Tile.X == CurrentLocation.X && Tile.Y == CurrentLocation.Y + 1)
		{
			West = true;
		}
	}
}


void APGMaze::AddFloor(FIntPoint TileLoc)
{
	float LocX = TileLoc.X * TileSize;
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileLoc.Y * TileSize;
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FTransform MyTransform(FRotator(0.f, 0.f, 0.f), FVector(LocX, LocY, 0.f), FVector(1.f, 1.f, .1f));

	if (FloorInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes == -1)
	{
		int32 FloorRecentlyAdded = FloorMesh->AddInstance(MyTransform);
		FloorInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes = FloorRecentlyAdded;
		FloorInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
	else
	{
		FloorMesh->AddInstance(MyTransform);
		FloorInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
}


void APGMaze::AddNorthWall(FIntPoint TileLoc)
{
	float LocX = TileLoc.X * TileSize;
	LocX += TileSize / 2;
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileLoc.Y * TileSize;
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FTransform MyTransform(FRotator(0.f, 0.f, 0.f), FVector(LocX, LocY, 200.f), FVector(.1f, 10.f, 50.f) * .1);

	if (WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes == -1)
	{
		int32 WallRecentlyAdded = WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes = WallRecentlyAdded;
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
	else
	{
		WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
}


void APGMaze::AddEastWall(FIntPoint TileLoc)
{
	float LocX = TileLoc.X * TileSize;
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileLoc.Y * TileSize;
	LocY -= TileSize / 2; // distance offset to edge of path
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FVector InstanceLoc(LocX, LocY, 0.5f);
	FTransform MyTransform(FRotator(0.f, 90.f, 0.f), FVector(LocX, LocY, 200.f), FVector(.1f, 10.f, 50.f) * .1);

	if (WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes == -1)
	{
		int32 WallRecentlyAdded = WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes = WallRecentlyAdded;
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
	else
	{
		WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
}


void APGMaze::AddSouthWall(FIntPoint TileLoc)
{
	float LocX = TileLoc.X * TileSize;
	LocX -= TileSize / 2;
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileLoc.Y * TileSize;
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FTransform MyTransform(FRotator(0.f, 0.f, 0.f), FVector(LocX, LocY, 200.f), FVector(.1f, 10.f, 50.f) * .1);

	if (WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes == -1)
	{
		int32 WallRecentlyAdded = WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes = WallRecentlyAdded;
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
	else
	{
		WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
}


void APGMaze::AddWestWall(FIntPoint TileLoc)
{
	float LocX = TileLoc.X * TileSize;
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileLoc.Y * TileSize;
	LocY += TileSize / 2;
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FTransform MyTransform(FRotator(0.f, 90.f, 0.f), FVector(LocX, LocY, 200.f), FVector(.1f, 10.f, 50.f) * .1);

	if (WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes == -1)
	{
		int32 WallRecentlyAdded = WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].StartingIndexMeshes = WallRecentlyAdded;
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
	else
	{
		WallMesh->AddInstance(MyTransform);
		WallInstancesMap[MazeChunkOffset / 2000].LengthOfIndexes++;
	}
}

// function returns an array of points that draws a line from one point to another
TArray<FIntPoint> APGMaze::GetLineBetweenTwoPoints(FRandomStream& RandStream, FIntPoint StartingLoc, FIntPoint EndingLoc)
{
	int32 EndingXOver2 = (EndingLoc.X - StartingLoc.X) / 2;
	int32 EndingYOver2 = (EndingLoc.Y - StartingLoc.Y) / 2;

	TArray<FIntPoint> TilesOnLine;
	TilesOnLine.Add(StartingLoc);

	int32 XLoc = StartingLoc.X;
	int32 YLoc = StartingLoc.Y;

	int32 XGrows = 0; // keep track of how many times grown in each direction, don't want to move too far
	int32 YGrows = 0;

	for (int32 Iteration = 0; Iteration < 4; Iteration++) // grow 1/2 of the way in the x or y directions each time
	{
		if (RandStream.RandRange(0, 1) == 0 && XGrows < 2 || YGrows == 2)
		{ // give 50% chance of growing in X direction
			if (EndingXOver2 >= 0) // want to make sure we're growing in the right direction
			{
				for (int32 i = 0; i < EndingXOver2; i++)
				{ // go 1/2 of the way there on the x
					if (EndingXOver2 - 5 > i && i != 0)
					{
						TArray<FIntPoint> MayAppend = XNoisePos(RandStream, FIntPoint(XLoc + 1, YLoc));
						if (MayAppend.Num() > 0)
						{
							i += 4;
							XLoc += 4;
							TilesOnLine.Append(MayAppend);
						}
						else
						{
							XLoc++;
							TilesOnLine.Add(FIntPoint(XLoc, YLoc));
						}
					}
					else
					{
						XLoc++;
						TilesOnLine.Add(FIntPoint(XLoc, YLoc));
					}
				}
			}
			else
			{
				for (int32 i = 0; i > EndingXOver2; i--) // do everything backwards
				{ // go 1/2 of the way there on the x
					if (EndingXOver2 + 5 < i && i != 0)
					{
						TArray<FIntPoint> MayAppend = XNoiseNeg(RandStream, FIntPoint(XLoc - 1, YLoc));
						if (MayAppend.Num() > 0)
						{
							i -= 4;
							XLoc -= 4;
							TilesOnLine.Append(MayAppend);
						}
						else
						{
							XLoc--;
							TilesOnLine.Add(FIntPoint(XLoc, YLoc));
						}
					}
					else
					{
						XLoc--;
						TilesOnLine.Add(FIntPoint(XLoc, YLoc));
					}
				}
			}
			XGrows++;
		}
		else
		{
			if (EndingYOver2 >= 0)
			{
				for (int32 i = 0; i < EndingYOver2; i++)
				{ // go 1/2 of the way there on the y
					if (EndingYOver2 - 5 > i && i != 0)
					{
						TArray<FIntPoint> MayAppend = YNoisePos(RandStream, FIntPoint(XLoc, YLoc + 1));
						if (MayAppend.Num() > 0)
						{
							i += 4;
							YLoc += 4;
							TilesOnLine.Append(MayAppend);
						}
						else
						{
							YLoc++;
							TilesOnLine.Add(FIntPoint(XLoc, YLoc));
						}
					}
					else
					{
						YLoc++;
						TilesOnLine.Add(FIntPoint(XLoc, YLoc));
					}
				}
			}
			else
			{
				for (int32 i = 0; i > EndingYOver2; i--)
				{ // go 1/2 of the way there on the y
					if (EndingYOver2 + 5 < i && i != 0)
					{
						TArray<FIntPoint> MayAppend = YNoiseNeg(RandStream, FIntPoint(XLoc, YLoc - 1));
						if (MayAppend.Num() > 0)
						{
							i -= 4;
							YLoc -= 4;
							TilesOnLine.Append(MayAppend);
						}
						else
						{
							YLoc--;
							TilesOnLine.Add(FIntPoint(XLoc, YLoc));
						}
					}
					else
					{
						YLoc--;
						TilesOnLine.Add(FIntPoint(XLoc, YLoc));
					}
				}
			}
			YGrows++;
		}
	}

	// next we just make it to the end, should be pretty close. No more noise is necessary
	if (RandStream.RandRange(0, 1) == 0 && XLoc != EndingLoc.X || YLoc == EndingLoc.Y)
	{ // choose which direction to grow first (if necessary)
		int32 XTilesLeft = EndingLoc.X - XLoc;
		if (XTilesLeft >= 0)
		{
			for (int32 i = 0; i < XTilesLeft; i++)
			{ // grow until end for x
				XLoc++;
				TilesOnLine.Add(FIntPoint(XLoc, YLoc));
			}
		}
		else
		{
			for (int32 i = 0; i > XTilesLeft; i--)
			{ // grow until end for x
				XLoc--;
				TilesOnLine.Add(FIntPoint(XLoc, YLoc));
			}
		}

		if (YLoc != EndingLoc.Y)
		{
			int32 YTilesLeft = EndingLoc.Y - YLoc;
			if (YTilesLeft >= 0)
			{
				for (int32 i = 0; i < YTilesLeft; i++)
				{
					YLoc++;
					TilesOnLine.Add(FIntPoint(XLoc, YLoc));
				}
			}
			else
			{
				for (int32 i = 0; i > YTilesLeft; i--)
				{
					YLoc--;
					TilesOnLine.Add(FIntPoint(XLoc, YLoc));
				}
			}
		}
	}
	else
	{
		int32 YTilesLeft = EndingLoc.Y - YLoc;
		if (YTilesLeft >= 0)
		{
			for (int32 i = 0; i < YTilesLeft; i++)
			{
				YLoc++;
				TilesOnLine.Add(FIntPoint(XLoc, YLoc));
			}
		}
		else
		{
			for (int32 i = 0; i > YTilesLeft; i--)
			{
				YLoc--;
				TilesOnLine.Add(FIntPoint(XLoc, YLoc));
			}
		}

		if (XLoc != EndingLoc.X)
		{
			int32 XTilesLeft = EndingLoc.X - XLoc;
			if (XTilesLeft >= 0)
			{
				for (int32 i = 0; i < XTilesLeft; i++)
				{ // grow until end for x
					XLoc++;
					TilesOnLine.Add(FIntPoint(XLoc, YLoc));
				}
			}
			else
			{
				for (int32 i = 0; i > XTilesLeft; i--)
				{ // grow until end for x
					XLoc--;
					TilesOnLine.Add(FIntPoint(XLoc, YLoc));

				}
			}
		}
	}

	ErrantPaths(RandStream, TilesOnLine, RandStream.RandRange(2, 4));

	return TilesOnLine;
}


TArray<FIntPoint> APGMaze::XNoisePos(FRandomStream& RandStream, FIntPoint CurrTile)
{
	TArray<FIntPoint> ToRet = TArray<FIntPoint>();
	if (RandStream.RandRange(1, 100) <= ThreeBlocksNoiseChance) // 5% chance
	{ // move three squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 3 < MazeSize.Y || CurrTile.Y - 3 < 0)
		{ // 50% chance of going left
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // over three
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 3)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 3));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
		else
		{ // go right
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // over three
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 3)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 3));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= TwoBlocksNoiseChance) // 10% chance
	{ // move two squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 2 < MazeSize.Y || CurrTile.Y - 2 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // over two
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // over two
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= OneBlockNoiseChance) // 25% chance
	{ // move one square
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 1 < MazeSize.Y || CurrTile.Y - 1 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1)); // over one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1)); // over one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // up one
		}
	}
	// else no noise, 60% chance

	return ToRet;
}


TArray<FIntPoint> APGMaze::XNoiseNeg(FRandomStream& RandStream, FIntPoint CurrTile)
{
	TArray<FIntPoint> ToRet = TArray<FIntPoint>();
	if (RandStream.RandRange(1, 100) <= ThreeBlocksNoiseChance) // 5% chance
	{ // move three squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 3 < MazeSize.Y || CurrTile.Y - 3 < 0)
		{ // 50% chance of going left
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // over three
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 3)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 3));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
		else
		{ // go right
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // over three
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 3)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 3));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= TwoBlocksNoiseChance) // 10% chance
	{ // move two squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 2 < MazeSize.Y || CurrTile.Y - 2 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // over two
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // over two
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= OneBlockNoiseChance) // 25% chance
	{ // move one square
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.Y + 1 < MazeSize.Y || CurrTile.Y - 1 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 1)); // over one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 1)); // over one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 1));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // up one
		}
	}
	// else no noise, 60% chance

	return ToRet;
}


TArray<FIntPoint> APGMaze::YNoisePos(FRandomStream& RandStream, FIntPoint CurrTile)
{
	TArray<FIntPoint> ToRet = TArray<FIntPoint>();
	if (RandStream.RandRange(1, 100) <= ThreeBlocksNoiseChance) // variable edited in level
	{ // move three squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 3 < MazeSize.X || CurrTile.X - 3 < 0)
		{ // 50% chance of going left
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // over three
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
		else
		{ // go right
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // over three
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= TwoBlocksNoiseChance)
	{ // move two squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 2 < MazeSize.X || CurrTile.X - 2 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // over two
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // over two
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= OneBlockNoiseChance)
	{ // move one square
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 1 < MazeSize.X || CurrTile.X - 1 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y)); // over one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y + 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y)); // over one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y + 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y + 3)); // up one
		}
	}
	// else stay put, 70% chance

	return ToRet;
}


TArray<FIntPoint> APGMaze::YNoiseNeg(FRandomStream& RandStream, FIntPoint CurrTile)
{
	TArray<FIntPoint> ToRet = TArray<FIntPoint>();
	if (RandStream.RandRange(1, 100) <= ThreeBlocksNoiseChance) // variable edited in level
	{ // move three squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 3 < MazeSize.X || CurrTile.X - 3 < 0)
		{ // 50% chance of going left
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y)); // over three
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 3, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
		else
		{ // go right
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y)); // over three
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 3, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= TwoBlocksNoiseChance)
	{ // move two squares
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 2 < MazeSize.X || CurrTile.X - 2 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y)); // over two
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y)); // over two
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 2, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 2));
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
	}
	else if (RandStream.RandRange(1, 100) <= OneBlockNoiseChance)
	{ // move one square
		if (RandStream.RandRange(1, 2) == 2 && CurrTile.X + 1 < MazeSize.X || CurrTile.X - 1 < 0)
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y)); // over one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X + 1, CurrTile.Y - 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
		else
		{
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y));
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y)); // over one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 1)); // up one
			ToRet.Add(FIntPoint(CurrTile.X - 1, CurrTile.Y - 2)); // up one
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 2)); // back to center
			ToRet.Add(FIntPoint(CurrTile.X, CurrTile.Y - 3)); // up one
		}
	}
	// else stay put, 70% chance

	return ToRet;
}


void APGMaze::ErrantPaths(FRandomStream& RandStream, TArray<FIntPoint>& PathTiles, int32 NumPaths)
{
	for (int i = 0; i < NumPaths; i++)
	{
		FIntPoint CurrLoc;
		if (PathTiles.Num() - 3 >= 2)
		{
			CurrLoc = PathTiles[RandStream.RandRange(2, PathTiles.Num() - 3)]; // don't want to start too close to the ends
		}
		else
		{ // crazy short path
			UE_LOG(LogTemp, Warning, TEXT("Short path in ErrantPaths!"));
			return; // doesn't make a lot of sense to make long winding errant paths when it's only 5 or so blocks
		}

		FIntPoint Direction(0, 0);
		for (int j = 0; j < RandStream.RandRange(2, 4); j++)
		{
			Direction = PickDirection(RandStream, Direction, CurrLoc);
			PathTiles.Add(CurrLoc + Direction);
			PathTiles.Add(CurrLoc + Direction * 2);
			PathTiles.Add(CurrLoc + Direction * 3);
			CurrLoc += Direction * 3;
		}
	}
}


FIntPoint APGMaze::PickDirection(FRandomStream& RandStream, FIntPoint PrevDirection, FIntPoint Location)
{
	FIntPoint ToRet(0, 0);

	while (ToRet == FIntPoint(0, 0))
	{
		WhileLoopStopper++;
		if (WhileLoopStopper >= 50)
		{
			return FIntPoint(0, 0);
		}

		if (RandStream.RandRange(1, 2) == 2) // fifty percent chance doing one or the other
			ToRet.X = RandStream.RandRange(-1, 1);
		else
			ToRet.Y = RandStream.RandRange(-1, 1);

		if (ToRet == PrevDirection * -1)
			ToRet = FIntPoint(0, 0);
		else if (Location.X + ToRet.X * 3 >= MazeSize.X || Location.X + ToRet.X * 3 < 0 || Location.Y + ToRet.Y * 3 >= MazeSize.Y || Location.Y + ToRet.Y * 3 < 0)
			ToRet = FIntPoint(0, 0);
	}
	return ToRet;
}


void APGMaze::RemoveMazeFromViewport(FIntPoint MazeChunkToRemove, bool NeedToDespawnMonster)
{
	if (!FloorInstancesMap.Contains(MazeChunkToRemove))
		return;

	int32 FirstFloorIndex = FloorInstancesMap[MazeChunkToRemove].StartingIndexMeshes; // this is the int we'll check against to shift the other arrays
	int32 OrigFloorsLength = FloorInstancesMap[MazeChunkToRemove].LengthOfIndexes; // array is shrunk when RemoveInstance() is called
	for (int32 i = 0; i < OrigFloorsLength; i++)
	{ // remove floors from viewport
		FloorMesh->RemoveInstance(FirstFloorIndex); // the instance array is updated so we can just remove this index every time
	}

	int32 FirstWallIndex = WallInstancesMap[MazeChunkToRemove].StartingIndexMeshes;
	int32 OrigWallsLength = WallInstancesMap[MazeChunkToRemove].LengthOfIndexes; // repeat of the floors but with the walls
	for (int32 i = 0; i < OrigWallsLength; i++)
	{
		WallMesh->RemoveInstance(FirstWallIndex);
	}

	FloorInstancesMap.Remove(MazeChunkToRemove); // get rid of key-val pair in map
	WallInstancesMap.Remove(MazeChunkToRemove);

	// Need to update other int maps to accurately reflect position in their Instance arrays
	for (auto& elem : FloorInstancesMap)
	{
		if (elem.Value.StartingIndexMeshes > FirstFloorIndex)
		{
			elem.Value.StartingIndexMeshes -= OrigFloorsLength; // we only have to update the 0th value bc it's all that's used for removal
		}
	}

	for (auto& elem : WallInstancesMap) // repeat for walls
	{
		if (elem.Value.StartingIndexMeshes > FirstWallIndex)
		{
			elem.Value.StartingIndexMeshes -= OrigWallsLength;
		}
	}

	if (NeedToDespawnMonster)
	{
		DespawnMonster();
	}
}


void APGMaze::Village(FIntPoint Location)
{
}


void APGMaze::MonsterField(FIntPoint Location, TArray<FIntPoint> ExitLocs)
{
	UE_LOG(LogTemp, Warning, TEXT("Monster field location: %d, %d"), Location.X, Location.Y);

	FIndexAndLengthInMap Temp = FIndexAndLengthInMap();
	Temp.StartingIndexMeshes = -1; // to tell when we're setting the first index
	Temp.LengthOfIndexes = 0; // to keep track of how many instances have been added to viewport
	FloorInstancesMap.Add(Location, Temp);
	WallInstancesMap.Add(Location, Temp);

	MazeChunkOffset = Location * 2000; // 4000 / 2

	for (int32 i = 0; i < MazeSize.X; i++)
	{
		for (int32 j = 0; j < MazeSize.Y; j++)
		{
			float LocX = i * TileSize;
			LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
			float LocY = j * TileSize;
			LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
			FTransform MyTransform(FRotator(0.f, 0.f, 0.f), FVector(LocX, LocY, 0.f), FVector(1.f, 1.f, .1f));

			if (FloorInstancesMap[Location].StartingIndexMeshes == -1)
			{
				int32 StartIndexMesh = FloorMesh->AddInstance(MyTransform);
				FloorInstancesMap[Location].StartingIndexMeshes = StartIndexMesh;
				FloorInstancesMap[Location].LengthOfIndexes++;
			}
			else
			{
				if (i == MazeSize.X / 2 && j == MazeSize.Y / 2)
				{
					int32 InstanceOfMesh = FloorMesh->AddInstance(MyTransform);
					FloorInstancesMap[Location].LengthOfIndexes++;
					FTransform InstanceTransform;
					FloorMesh->GetInstanceTransform(InstanceOfMesh, InstanceTransform, true);
					FVector WhereToSpawnMonster = InstanceTransform.GetTranslation() - FVector(0.f, 0.f, 5.5);
					SpawnMonster(WhereToSpawnMonster);

					UpdateNavMesh(WhereToSpawnMonster, FVector(ScaleUnitNavMesh, ScaleUnitNavMesh, 2.f));
				}
				FloorMesh->AddInstance(MyTransform); // don't need to keep track of index
				FloorInstancesMap[Location].LengthOfIndexes++;
			}

			bool DoIfs = true;
			for (FIntPoint NoWalls : ExitLocs)
			{
				if (NoWalls == FIntPoint(i, j))
				{
					DoIfs = false;
				}
			}

			if (DoIfs && i == 0)
			{
				AddSouthWall(FIntPoint(i, j));
			}
			if (DoIfs && i == MazeSize.X - 1)
			{
				AddNorthWall(FIntPoint(i, j));
			}
			if (DoIfs && j == 0)
			{
				AddEastWall(FIntPoint(i, j));
			}
			if (DoIfs && j == MazeSize.Y - 1)
			{
				AddWestWall(FIntPoint(i, j));
			}
		}
	}

	MazeChunkOffset *= 2; // now want it to be 4000 not just 2000

	// spawn guards over two exits we don't want player to leave

}


FVector APGMaze::GetBlockLocationFromIntPoints(FIntPoint OffsetToChunk, FIntPoint TileInChunk)
{
	MazeChunkOffset = OffsetToChunk * 4000; // maze size * tile size * mesh size
	float LocX = TileInChunk.X * TileSize * 2; // 2 is for the mesh
	LocX += AddSoStartIsZeroZero.X + MazeChunkOffset.X;
	float LocY = TileInChunk.Y * TileSize * 2;
	LocY += AddSoStartIsZeroZero.Y + MazeChunkOffset.Y;
	FTransform FloorTransform(FRotator(0.f, 0.f, 0.f), FVector(LocX, LocY, 50.f), FVector(1.f, 1.f, .1f));

	FVector Temp = (FloorMesh->GetComponentTransform() * FloorTransform).GetTranslation();

	return Temp;
}