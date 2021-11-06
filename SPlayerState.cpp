// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"

void ASPlayerState::AddScore(float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);
}

float ASPlayerState::GetLeadingScore()
{
	float LeadingScore = 0.f;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn)
		{
			ASPlayerState* PS = Cast<ASPlayerState>(TestPawn->GetPlayerState());
			if (PS)
			{
				if (PS->Score > LeadingScore)
				{
					LeadingScore = PS->Score;
				}
			}
		}
	}

	return LeadingScore;
}