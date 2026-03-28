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
	m_I = I;
	m_J = J;
}

void ASquare::ReceivePiece(AChessPiecePType* piece)
{
	m_OccupyingPiece = piece;
}

void ASquare::SetIsVacant()
{
	m_OccupyingPiece = nullptr;
}

bool ASquare::GetIsVacant() const
{
	return (!IsValid(m_OccupyingPiece));
}

bool ASquare::GetSquareHasKing(ETeam kingsTeam) const
{
	if (IsValid(m_OccupyingPiece)) {
		if (m_OccupyingPiece->GetEPieceType() == EPiece::King) {
			return (m_OccupyingPiece->GetTeam() == kingsTeam);
		}
	}
	return false;
}

bool ASquare::GetSquareHasKing() const
{
	if (IsValid(m_OccupyingPiece)) {
		if (m_OccupyingPiece->GetEPieceType() == EPiece::King) {
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
	if (m_OccupyingPiece) {
		m_OccupyingPiece->BP_OnCheckMate();
	}
}

