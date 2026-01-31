// Fill out your copyright notice in the Description page of Project Settings.


#include "Square.h"

// Sets default values
ASquare::ASquare()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASquare::BeginPlay()
{
	Super::BeginPlay();
	
}

float ASquare::RandomDirectionF()
{
	return (FMath::RandBool()) ? -1.f : 1.f;
}

void ASquare::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASquare::ReceiveCoordinates(int I, int J)
{
	i = I;
	j = J;
}

void ASquare::ReceivePiece(AChessPiecePType* piece)
{
	occupyingPiece = piece;
}

AChessPiecePType* ASquare::GetOccupyingPiece() const
{
	return occupyingPiece;
}

void ASquare::SetIsVacant()
{
	occupyingPiece = nullptr;
}

bool ASquare::GetIsVacant() const
{
	return (!IsValid(occupyingPiece));
}

bool ASquare::GetSquareHasKing(ETeam kingsTeam) const
{
	if (IsValid(occupyingPiece)) {
		if (occupyingPiece->GetEPieceType() == EPiece::King) {
			return (occupyingPiece->GetTeam() == kingsTeam);
		}
	}
	return false;
}

bool ASquare::GetSquareHasKing() const
{
	if (IsValid(occupyingPiece)) {
		if (occupyingPiece->GetEPieceType() == EPiece::King) {
			return true;
		}
	}
	return false;
}


int ASquare::GetI() const
{
	return i;
}

int ASquare::GetJ() const
{
	return j;
}

FVector ASquare::GetPieceLocation()
{
	return GetActorLocation();
}

void ASquare::CheckmateAction()
{
	BP_CheckmateAction(); // to blueprints
	if (occupyingPiece) {
		occupyingPiece->BP_OnCheckMate();
	}
}

