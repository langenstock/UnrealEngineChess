// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessPiece.h"

// Sets default values
AChessPiece::AChessPiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChessPiece::BeginPlay()
{
	Super::BeginPlay();
	
}



// Called every frame
void AChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}










/*



EPiece AChessPiece::GetEPieceType() const
{
	return ePiece;
}

bool AChessPiece::GetHasMoved() const {
	return hasMoved;
}

ETeam AChessPiece::GetTeam() const
{
	return team;
}

int AChessPiece::GetSquareI() const
{
	return squareI;
}

int AChessPiece::GetSquareJ() const
{
	return squareJ;
}

void AChessPiece::ReceiveCoordinates(int i, int j)
{
	squareI = i;
	squareJ = j;
}

void AChessPiece::OnClick(int button)
{
	if (button == 0) {
		SetIsInFocus(true);
	}
	else if (button == 1) {

	}
}

void AChessPiece::SetIsInFocus(bool isInFocus) {
	inFocus = isInFocus;
	BP_SetIsInFocus(isInFocus);
}

void AChessPiece::SetTargetMove(FVector loc)
{
	pieceState = EPieceState::Moving;
	moveTarget = loc;
	BP_MoveToTarget(loc);
}

EPieceState AChessPiece::GetPieceState() const
{
	return pieceState;
}

*/