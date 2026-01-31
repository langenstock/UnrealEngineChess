// Fill out your copyright notice in the Description page of Project Settings.


#include "Square.h"


ASquare::ASquare()
{
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

