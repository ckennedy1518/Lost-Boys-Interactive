// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"
#include "PGMaze.h"
#include "Kismet/GameplayStatics.h"
#include "ProcGenMaze/ProcGenMazeCharacter.h"
#include "PGLoadGameManager.h"

APGPlayerController::APGPlayerController()
{
	PlayerChunk.X = 0;
	PlayerChunk.Y = 0;
	NextMonsterField = 10;
	ChunksUntilNextMonsterFieldMin = 25;
	ChunksUntilNextMonsterFieldMax = 100;
}


void APGPlayerController::BeginPlay()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APGMaze::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
		MazeInGame = Cast<APGMaze>(FoundActors[0]);

	CurrentSeed = MazeInGame->GetSeed();
	MazeSize = MazeInGame->GetMazeSize();

	InitialLoad();

	// see if we need to load a new maze chunk every second 
	GetWorldTimerManager().SetTimer(TimerHandle_NeedToLoadMazeCaller, this, &APGPlayerController::ControlMazeLoading, 1.f);
}


void APGPlayerController::InitialLoad()
{
	FIntPoint FirstChunk(0, 0); // the starting spot
	FChunkContainer NewMazeChunk;
	NewMazeChunk.PointsInChunk = TArray<FIntPoint>();
	NewMazeChunk.PointsInChunk.Add(FirstChunk); // want the player to be able to start here
	NewMazeChunk.SeedForChunk = CurrentSeed;
	FRandomStream RandStream(CurrentSeed);
	CurrentSeed++;

	NewMazeChunk.PointsInChunk.Append(AddPoints(RandStream, false, true, false, true));

	MazeInGame->NewMaze(CurrentSeed, FirstChunk, NewMazeChunk.PointsInChunk);

	LocInfoPairs.Add(FirstChunk, NewMazeChunk);
	LocInfoPairs[FirstChunk].HasBeenLoaded = true;
	UpdateChunksAround(FirstChunk);

	for (int32 i = 0; i < MAZE_SIZE_LOADED; i++)
	{
		for (int32 j = 0; j < MAZE_SIZE_LOADED; j++)
		{ // create 3 x 3 square to start
			if (i != 1 || j != 1) // this will be 0, 0 and we already did that
			{
				FIntPoint Offset(i - 1, j - 1); // -1 to 1

				AddNewChunk(Offset);
			}
		}
	}
}


TArray<FIntPoint> APGPlayerController::AddPoints(FRandomStream& RandStream, bool XZero, bool XMax, bool YZero, bool YMax)
{
	TArray<FIntPoint> ToRet = TArray<FIntPoint>();

	if (XZero)
	{
		int32 RandPos = RandStream.RandRange(1, MazeSize.X - 2);
		ToRet.Add(FIntPoint(0, RandPos));
	}
	if (XMax)
	{
		int32 RandPos = RandStream.RandRange(1, MazeSize.X - 2);
		ToRet.Add(FIntPoint(MazeSize.X - 1, RandPos));
	}
	if (YZero)
	{
		int32 RandPos = RandStream.RandRange(1, MazeSize.X - 2);
		ToRet.Add(FIntPoint(RandPos, 0));
	}
	if (YMax)
	{
		int32 RandPos = RandStream.RandRange(1, MazeSize.X - 2);
		ToRet.Add(FIntPoint(RandPos, MazeSize.Y - 1));
	}

	return ToRet;
}


void APGPlayerController::ControlMazeLoading()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NeedToLoadMazeCaller, this, &APGPlayerController::ControlMazeLoading, 1.f);
	// function is "ticking" every second

	FIntPoint CurrentChunk; // the current chunk the player is in
	AActor* PawnAsActor = Cast<AActor>(GetPawn());

	if (PawnAsActor)
	{
		FVector PlayerLoc = PawnAsActor->GetActorLocation();

		float XClose = PlayerLoc.X / (MazeSize.X * MazeInGame->TileSize * 2); // 20 tiles per row, 100 units wide, * 2 for mesh
		float YClose = PlayerLoc.Y / (MazeSize.Y * MazeInGame->TileSize * 2);

		CurrentChunk.X = FMath::FloorToInt(XClose);
		CurrentChunk.Y = FMath::FloorToInt(YClose);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get PawnAsActor"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}

	if (CurrentChunk != PlayerChunk)
	{
		if (LocInfoPairs[CurrentChunk].PointsInChunk.Num() == 5) // in a monster field
		{
			AProcGenMazeCharacter* PGMC = Cast<AProcGenMazeCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
			FVector PlayerLoc(0.f, 0.f, 0.f);
			TMap<FIntPoint, FVector> ExitsAndLocs = TMap<FIntPoint, FVector>();

			if (PGMC)
			{
				PlayerLoc = PGMC->GetActorLocation();

				float DistanceFromPoint = 100000000; // some large number to start
				FVector WinningVector(0.f, 0.f, 0.f);
				for (FIntPoint LocCheck : LocInfoPairs[CurrentChunk].PointsInChunk) // loop finds the world location of the exits and which IntPoint they belong to
				{
					FVector DistanceCheck = MazeInGame->GetBlockLocationFromIntPoints(CurrentChunk, LocCheck);
					ExitsAndLocs.Add(LocCheck, DistanceCheck);

					float DistanceBetweenPoints = FVector::Dist(PlayerLoc, DistanceCheck);
					if (DistanceBetweenPoints < DistanceFromPoint)
					{
						DistanceFromPoint = DistanceBetweenPoints;
						WinningVector = DistanceCheck;
					}
				}

				FIntPoint MazeStartIntPoint = *ExitsAndLocs.FindKey(WinningVector);
				if (MazeStartIntPoint.X == 0) // based on the for loop above we can now determine where it starts, meaning we know what's across
				{
					for (FIntPoint TempCheck : LocInfoPairs[CurrentChunk].PointsInChunk)
					{
						if (TempCheck.X == MazeSize.X - 1)
						{
							PlayerExitMonsterField = ExitsAndLocs[TempCheck];
						}
					}
				}
				else if (MazeStartIntPoint.X == MazeSize.X - 1)
				{
					for (FIntPoint TempCheck : LocInfoPairs[CurrentChunk].PointsInChunk)
					{
						if (TempCheck.X == 0)
						{
							PlayerExitMonsterField = ExitsAndLocs[TempCheck];
						}
					}
				}
				else if (MazeStartIntPoint.Y == 0)
				{
					for (FIntPoint TempCheck : LocInfoPairs[CurrentChunk].PointsInChunk)
					{
						if (TempCheck.Y == MazeSize.Y - 1)
						{
							PlayerExitMonsterField = ExitsAndLocs[TempCheck];
						}
					}
				}
				else if (MazeStartIntPoint.Y == MazeSize.Y - 1)
				{
					for (FIntPoint TempCheck : LocInfoPairs[CurrentChunk].PointsInChunk)
					{
						if (TempCheck.Y == 0)
						{
							PlayerExitMonsterField = ExitsAndLocs[TempCheck];
						}
					}
				}
			}
		}

		for (int32 i = 0; i < MAZE_SIZE_LOADED; i++)
		{
			FIntPoint ChunkToRemove(0, 0); // remove the three chunks that are now far away
			FIntPoint ChunkToAdd(0, 0); // add the three chunks that are now closer

			if (CurrentChunk.X > PlayerChunk.X)
			{
				ChunkToRemove.X = PlayerChunk.X - 1;
				ChunkToRemove.Y = PlayerChunk.Y - i + 1;
				ChunkToAdd.X = PlayerChunk.X + 2;
				ChunkToAdd.Y = PlayerChunk.Y - i + 1;
			}
			else if (CurrentChunk.X < PlayerChunk.X)
			{
				ChunkToRemove.X = PlayerChunk.X + 1;
				ChunkToRemove.Y = PlayerChunk.Y - i + 1;
				ChunkToAdd.X = PlayerChunk.X - 2;
				ChunkToAdd.Y = PlayerChunk.Y - i + 1;
			}
			else if (CurrentChunk.Y > PlayerChunk.Y)
			{
				ChunkToRemove.X = PlayerChunk.X - i + 1;
				ChunkToRemove.Y = PlayerChunk.Y - 1;
				ChunkToAdd.X = PlayerChunk.X - i + 1;
				ChunkToAdd.Y = PlayerChunk.Y + 2;
			}
			else // CurrentChunk.Y < PlayerChunk.Y
			{
				ChunkToRemove.X = PlayerChunk.X - i + 1;
				ChunkToRemove.Y = PlayerChunk.Y + 1;
				ChunkToAdd.X = PlayerChunk.X - i + 1;
				ChunkToAdd.Y = PlayerChunk.Y - 2;
			}

			MazeInGame->RemoveMazeFromViewport(ChunkToRemove, LocInfoPairs[ChunkToRemove].PointsInChunk.Num() == 5); // removal
			AddNewChunk(ChunkToAdd); // addition
		}
	}

	if (CurrentChunk.X != PlayerChunk.X)
	{
		PlayerChunk.X = CurrentChunk.X;
	}
	else if (CurrentChunk.Y != PlayerChunk.Y)
	{
		PlayerChunk.Y = CurrentChunk.Y;
	}
}


void APGPlayerController::AddNewChunk(FIntPoint ChunkToAdd)
{
	UpdateChunksAround(ChunkToAdd);

	if (LocInfoPairs.Contains(ChunkToAdd) && LocInfoPairs[ChunkToAdd].HasBeenLoaded)
	{
		if (LocInfoPairs[ChunkToAdd].PointsInChunk.Num() <= 4) // normal chunk
		{
			MazeInGame->NewMaze(LocInfoPairs[ChunkToAdd].SeedForChunk, ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk); // "Reload" Maze chunk that's already been visited
		}
		else if (LocInfoPairs[ChunkToAdd].PointsInChunk.Num() == 5) // monster field
		{
			MazeInGame->MonsterField(ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
		}
	}
	else if (LocInfoPairs.Contains(ChunkToAdd) && LocInfoPairs[ChunkToAdd].PointsInChunk.Num() == 5)
	{
		MazeInGame->MonsterField(ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
		LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
		UpdateChunksAround(ChunkToAdd);

		FRandomStream RandStream(CurrentSeed);
		NextMonsterField = RandStream.RandRange(ChunksUntilNextMonsterFieldMin, ChunksUntilNextMonsterFieldMax);
	}
	else if (LocInfoPairs.Contains(ChunkToAdd) && LocInfoPairs[ChunkToAdd].PointsInChunk.Num() == 4)
	{
		if (NextMonsterField == 0)
		{
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(MazeSize.X / 2, MazeSize.Y / 2)); // some point that is not on the edge, now has five points

			MazeInGame->MonsterField(ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);

			FRandomStream RandStream(CurrentSeed);
			NextMonsterField = RandStream.RandRange(ChunksUntilNextMonsterFieldMin, ChunksUntilNextMonsterFieldMax);
		}
		else
		{
			NextMonsterField--; // getting closer to spawning instead of maze chunk
			MazeInGame->NewMaze(LocInfoPairs[ChunkToAdd].SeedForChunk, ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);
		}
	}
	else if (LocInfoPairs.Contains(ChunkToAdd))
	{
		// start by getting 4 sides, then do monster field check
		UpdateChunksAround(FIntPoint(ChunkToAdd.X + 1, ChunkToAdd.Y)); // call these four to ensure walls can join
		UpdateChunksAround(FIntPoint(ChunkToAdd.X - 1, ChunkToAdd.Y));
		UpdateChunksAround(FIntPoint(ChunkToAdd.X, ChunkToAdd.Y + 1));
		UpdateChunksAround(FIntPoint(ChunkToAdd.X, ChunkToAdd.Y - 1));

		FRandomStream RandStream(LocInfoPairs[ChunkToAdd].SeedForChunk);

		bool NorthSide = false;
		bool EastSide = false;
		bool WestSide = false;
		bool SouthSide = false;
		for (FIntPoint SeeWhichSide : LocInfoPairs[ChunkToAdd].PointsInChunk)
		{
			if (SeeWhichSide.X == 0)
			{
				NorthSide = true;
			}
			else if (SeeWhichSide.X == MazeSize.X - 1)
			{
				SouthSide = true;
			}
			else if (SeeWhichSide.Y == 0)
			{
				EastSide = true;
			}
			else if (SeeWhichSide.Y == MazeSize.Y - 1)
			{
				WestSide = true;
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("SeeWhichSide doesn't have a side (from else if 2)"));
		}

		LocInfoPairs[ChunkToAdd].PointsInChunk.Append(AddPoints(RandStream, !NorthSide, !SouthSide, !EastSide, !WestSide));

		if (NextMonsterField == 0)
		{
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(MazeSize.X / 2, MazeSize.Y / 2)); // some point that is not on the edge

			MazeInGame->MonsterField(ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);

			NextMonsterField = RandStream.RandRange(ChunksUntilNextMonsterFieldMin, ChunksUntilNextMonsterFieldMax);
		}
		else
		{
			NextMonsterField--; // getting closer to spawning instead of maze chunk
			
			MazeInGame->NewMaze(LocInfoPairs[ChunkToAdd].SeedForChunk, ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);
		}
	}
	else
	{
		UpdateChunksAround(FIntPoint(ChunkToAdd.X + 1, ChunkToAdd.Y)); // still need to ensure that theres no walls around
		UpdateChunksAround(FIntPoint(ChunkToAdd.X - 1, ChunkToAdd.Y));
		UpdateChunksAround(FIntPoint(ChunkToAdd.X, ChunkToAdd.Y + 1));
		UpdateChunksAround(FIntPoint(ChunkToAdd.X, ChunkToAdd.Y - 1));

		if (LocInfoPairs.Contains(ChunkToAdd)) // it was added by one of the four previous calls to UCA
		{ // this is the same as the 2nd else if but without the UCA calls (they were just done)
			FRandomStream RandStream(LocInfoPairs[ChunkToAdd].SeedForChunk);

			bool NorthSide = false;
			bool EastSide = false;
			bool WestSide = false;
			bool SouthSide = false;
			for (FIntPoint SeeWhichSide : LocInfoPairs[ChunkToAdd].PointsInChunk)
			{
				if (SeeWhichSide.X == 0)
					NorthSide = true;
				else if (SeeWhichSide.X == MazeSize.X - 1)
					SouthSide = true;
				else if (SeeWhichSide.Y == 0)
					EastSide = true;
				else if (SeeWhichSide.Y == MazeSize.Y - 1)
					WestSide = true;
				else
					UE_LOG(LogTemp, Warning, TEXT("SeeWhichSide doesn't have a side (from else)"));
			}

			LocInfoPairs[ChunkToAdd].PointsInChunk.Append(AddPoints(RandStream, !NorthSide, !SouthSide, !EastSide, !WestSide));
		}
		else
		{
			FChunkContainer MazeChunkAdding;
			MazeChunkAdding.PointsInChunk = TArray<FIntPoint>();
			MazeChunkAdding.HasBeenLoaded = false;
			MazeChunkAdding.SeedForChunk = CurrentSeed;
			CurrentSeed++;

			LocInfoPairs.Add(ChunkToAdd, MazeChunkAdding);

			FRandomStream RandStream(LocInfoPairs[ChunkToAdd].SeedForChunk);

			int32 RandPos = RandStream.RandRange(1, MazeSize.X - 2); // X and Y should be the same, just need a number that won't be on the corner
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(0, RandPos));
			
			RandPos = RandStream.RandRange(1, MazeSize.X - 2); // get new one for each side
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(RandPos, 0));
			
			RandPos = RandStream.RandRange(1, MazeSize.X - 2);
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(RandPos, MazeSize.Y - 1));

			RandPos = RandStream.RandRange(1, MazeSize.X - 2);
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(MazeSize.X - 1, RandPos));
		}


		if (NextMonsterField == 0)
		{
			LocInfoPairs[ChunkToAdd].PointsInChunk.Add(FIntPoint(MazeSize.X / 2, MazeSize.Y / 2)); // some point that is not on the edge

			MazeInGame->MonsterField(ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);

			FRandomStream RandStream(CurrentSeed);
			NextMonsterField = RandStream.RandRange(ChunksUntilNextMonsterFieldMin, ChunksUntilNextMonsterFieldMax);
		}
		else
		{
			NextMonsterField--;

			MazeInGame->NewMaze(LocInfoPairs[ChunkToAdd].SeedForChunk, ChunkToAdd, LocInfoPairs[ChunkToAdd].PointsInChunk);
			LocInfoPairs[ChunkToAdd].HasBeenLoaded = true;
			UpdateChunksAround(ChunkToAdd);
		}
	}
}


void APGPlayerController::UpdateChunksAround(FIntPoint ChunkToUpdate)
{
	if (!LocInfoPairs.Contains(ChunkToUpdate))
		return; // avoid running through more computations than necessary

	bool North = false; // directions may not be accurate, just labels
	bool East = false;
	bool South = false;
	bool West = false;
	TArray<FIntPoint> NorthArr = TArray<FIntPoint>();
	TArray<FIntPoint> EastArr = TArray<FIntPoint>();
	TArray<FIntPoint> SouthArr = TArray<FIntPoint>();
	TArray<FIntPoint> WestArr = TArray<FIntPoint>();

	for (FIntPoint Temp : LocInfoPairs[ChunkToUpdate].PointsInChunk)
	{
		if (Temp.X == 0)
		{
			North = true;
			NorthArr.Add(FIntPoint(MazeSize.X - 1, Temp.Y)); // what the point would be in the other chunk
		}
		if (Temp.X == MazeSize.X - 1)
		{
			East = true;
			EastArr.Add(FIntPoint(0, Temp.Y));
		}
		if (Temp.Y == 0)
		{
			South = true;
			SouthArr.Add(FIntPoint(Temp.X, MazeSize.Y - 1));
		}
		if (Temp.Y == MazeSize.Y - 1)
		{
			West = true;
			WestArr.Add(FIntPoint(Temp.X, 0));
		}
	}

	if (North)
	{
		if (LocInfoPairs.Contains(FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y)))
		{
			for (FIntPoint Temp : NorthArr)
			{
				if (!LocInfoPairs[FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y)].PointsInChunk.Contains(Temp))
				{
					LocInfoPairs[FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y)].PointsInChunk.Add(Temp);
				} // else already in array which is good
			}
		}
		else
		{ // need to add to LIP, including these points
			FChunkContainer NewMazeChunk;
			NewMazeChunk.PointsInChunk = TArray<FIntPoint>();
			NewMazeChunk.SeedForChunk = CurrentSeed;
			CurrentSeed++;

			LocInfoPairs.Add(FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y), NewMazeChunk);

			for (FIntPoint Temp : NorthArr)
			{
				LocInfoPairs[FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y)].PointsInChunk.Add(Temp);
			}
			LocInfoPairs[FIntPoint(ChunkToUpdate.X - 1, ChunkToUpdate.Y)].HasBeenLoaded = false;
		}
	} // else nothing to update

	if (East)
	{ // same as north but for diff direction
		if (LocInfoPairs.Contains(FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y)))
		{
			for (FIntPoint Temp : EastArr)
			{
				if (!LocInfoPairs[FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y)].PointsInChunk.Contains(Temp))
				{
					LocInfoPairs[FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y)].PointsInChunk.Add(Temp);
				}
			}
		}
		else
		{
			FChunkContainer NewMazeChunk;
			NewMazeChunk.PointsInChunk = TArray<FIntPoint>();
			NewMazeChunk.SeedForChunk = CurrentSeed;
			CurrentSeed++;

			LocInfoPairs.Add(FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y), NewMazeChunk);

			for (FIntPoint Temp : EastArr)
			{
				LocInfoPairs[FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y)].PointsInChunk.Add(Temp);
			}
			LocInfoPairs[FIntPoint(ChunkToUpdate.X + 1, ChunkToUpdate.Y)].HasBeenLoaded = false;
		}
	}

	if (South)
	{
		if (LocInfoPairs.Contains(FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1)))
		{
			for (FIntPoint Temp : SouthArr)
			{
				if (!LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1)].PointsInChunk.Contains(Temp))
				{
					LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1)].PointsInChunk.Add(Temp);
				}
			}
		}
		else
		{
			FChunkContainer NewMazeChunk;
			NewMazeChunk.PointsInChunk = TArray<FIntPoint>();
			NewMazeChunk.SeedForChunk = CurrentSeed;
			CurrentSeed++;

			LocInfoPairs.Add(FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1), NewMazeChunk);

			for (FIntPoint Temp : SouthArr)
			{
				LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1)].PointsInChunk.Add(Temp);
			}
		}
		LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y - 1)].HasBeenLoaded = false;
	}
	
	if (West)
	{
		if (LocInfoPairs.Contains(FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1)))
		{
			for (FIntPoint Temp : WestArr)
			{
				if (!LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1)].PointsInChunk.Contains(Temp))
				{
					LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1)].PointsInChunk.Add(Temp);
				}
			}
		}
		else
		{
			FChunkContainer NewMazeChunk;
			NewMazeChunk.PointsInChunk = TArray<FIntPoint>();
			NewMazeChunk.SeedForChunk = CurrentSeed;
			CurrentSeed++;

			LocInfoPairs.Add(FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1), NewMazeChunk);

			for (FIntPoint Temp : WestArr)
			{
				LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1)].PointsInChunk.Add(Temp);
			}
			LocInfoPairs[FIntPoint(ChunkToUpdate.X, ChunkToUpdate.Y + 1)].HasBeenLoaded = false;
		}
	}
}


bool APGPlayerController::IsInMonsterField()
{
	return LocInfoPairs[PlayerChunk].PointsInChunk.Num() == 5;
}