// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessGameManager.h"
using KM = UKismetMathLibrary;

AChessGameManager::AChessGameManager()
{
	PrimaryActorTick.bCanEverTick = true;
	movesHistory.Reserve(50);
}

void AChessGameManager::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
	if (ensure(menuScreenClass)) {
		menuScreen = CreateWidget<UUserWidget>(pc, menuScreenClass);
		menuScreen->AddToViewport();
		menuScreen->SetVisibility(ESlateVisibility::Visible);
		ReceiveMenuWidget(menuScreen);
	}

	SetGameState(EGameState::InbetweenGames);
}

void AChessGameManager::UndoMove(bool switchSides)
{
	if (movesHistory.Num() == 0) { return; };

	MoveRecord r = movesHistory.Last();
	int moveBackToI = r.fromI;
	int moveBackToJ = r.fromJ;

	AChessPiecePType* pieceThatMoved = r.pieceThatMoved;
	grid[moveBackToI][moveBackToJ]->ReceivePiece(pieceThatMoved);
	pieceThatMoved->ReceiveCoordinates(moveBackToI, moveBackToJ);

	FVector moveBackTo = grid[moveBackToI][moveBackToJ]->GetPieceLocation();
	pieceThatMoved->SetActorLocation(moveBackTo);
	pieceThatMoved->DecrementMovesMade();

	int moveBackFromI = r.toI;
	int moveBackFromJ = r.toJ;

	if (r.pieceTaken) {
		grid[moveBackFromI][moveBackFromJ]->ReceivePiece(r.pieceTaken);
		FVector backToLoc = grid[moveBackFromI][moveBackFromJ]->GetPieceLocation();
		r.pieceTaken->SetActorLocation(backToLoc);
		r.pieceTaken->SetPieceStateToIdle();
		allPieces.Push(r.pieceTaken);
	}
	else {
		grid[moveBackFromI][moveBackFromJ]->SetIsVacant();
		TryJiggle(moveBackFromI, moveBackFromJ);
	}

	for (ASquare* s : allSquares) {
		s->SetSelectableIndicator(false);
	}

	if (gameState == EGameState::WhiteTurnNoSelection) {
		if (switchSides) {
			SetGameState(EGameState::BlackTurnNoSelection);
		}
		else {
			SetGameState(EGameState::WhiteTurnNoSelection);
		}
	}
	else if (gameState == EGameState::WhiteTurnPieceInFocus) {
		if (switchSides) {
			SetGameState(EGameState::BlackTurnNoSelection);
		}
		else {
			SetGameState(EGameState::WhiteTurnNoSelection);
		}
	}
	else if (gameState == EGameState::BlackTurnNoSelection) {
		if (switchSides) {
			SetGameState(EGameState::WhiteTurnNoSelection);
		}
		else {
			SetGameState(EGameState::BlackTurnNoSelection);
		}
	}
	else if (gameState == EGameState::BlackTurnPieceInFocus) {
		if (switchSides) {
			SetGameState(EGameState::WhiteTurnNoSelection);
		}
		else {
			SetGameState(EGameState::BlackTurnNoSelection);
		}
	}

	movesHistory.Pop();
	if (r.isMoveTwoOfACastling) {
		UndoMove(false); // a castling is stored as two moves
	}
	OnMoveUndoSuccessful();
}

void AChessGameManager::NewGame()
{
	if (!debugBoard) {
		SetUpBoard();
	}
	else {
		SetUpDebugBoard();
	}
	FindAndStoreKingPositions();
	SetGameState(EGameState::WhiteTurnNoSelection);
}

void AChessGameManager::SetUpDebugBoard()
{
	if (allSquares.Num() > 0) {
		for (int i = allSquares.Num() - 1; i >= 0; i--) {
			allSquares[i]->Destroy();
			allSquares.RemoveAt(i);
		}
	}

	if (allPieces.Num() > 0) {
		for (int i = allPieces.Num() - 1; i >= 0; i--) {
			allPieces[i]->Destroy();
			allPieces.RemoveAt(i);
		}
	}

	SpawnSquares();

	if (ensure(pawnClass && rookClass && knightClass && bishopClass && kingClass && queenClass)) {
		UWorld* w = GetWorld();
		ensure(w);

		ASquare* sq = grid[0][1];
		FRotator r = FRotator{ 0, 0, 0 };
		r.Yaw = faceWhite;
		FActorSpawnParameters p{};
		p.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FVector v = sq->GetPieceLocation();
		AChessPiecePType* piece = w->SpawnActor<AChessPiecePType>(queenClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 1);
		piece->ReceiveEPieceTypeAllocation(EPiece::Queen);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[4][5];
		v = sq->GetPieceLocation();
		r.Yaw = faceWhite;
		piece = w->SpawnActor<AChessPiecePType>(kingClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(4, 5);
		piece->ReceiveEPieceTypeAllocation(EPiece::King);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[2][5];
		v = sq->GetPieceLocation();
		r.Yaw = faceWhite;
		piece = w->SpawnActor<AChessPiecePType>(rookClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(2, 5);
		piece->ReceiveEPieceTypeAllocation(EPiece::Rook);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][0];
		v = sq->GetPieceLocation();
		r.Yaw = faceBlack;
		piece = w->SpawnActor<AChessPiecePType>(queenClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(0, 0);
		piece->ReceiveEPieceTypeAllocation(EPiece::Queen);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[6][6];
		v = sq->GetPieceLocation();
		r.Yaw = faceBlack;
		piece = w->SpawnActor<AChessPiecePType>(kingClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(6, 6);
		piece->ReceiveEPieceTypeAllocation(EPiece::King);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);
	}

	PostBoardSetup(); // to blueprints
}

void AChessGameManager::FindAndStoreKingPositions()
{
	for (ASquare* s : allSquares) {
		if (s->GetSquareHasKing(ETeam::White)) {
			int I = s->GetI();
			int J = s->GetJ();
			whiteKingPos = { I, J };
		}
		else if (s->GetSquareHasKing(ETeam::Black)) {
			int I = s->GetI();
			int J = s->GetJ();
			blackKingPos = { I, J };
		}
	}
}

void AChessGameManager::SetUpBoard()
{
	if (allSquares.Num() > 0) {
		for (int i = allSquares.Num() - 1; i >= 0; i--) {
			allSquares[i]->Destroy();
			allSquares.RemoveAt(i);
		}
	}

	if (allPieces.Num() > 0) {
		for (int i = allPieces.Num() - 1; i >= 0; i--) {
			allPieces[i]->Destroy();
			allPieces.RemoveAt(i);
		}
	}

	SpawnSquares();

	if (ensure(pawnClass && rookClass && knightClass && bishopClass && kingClass && queenClass)) {
		UWorld* w = GetWorld();
		ensure(w);

		ASquare* sq = grid[0][0];
		FVector v = sq->GetPieceLocation();
		FRotator r = FRotator{ 0, 0, 0 };
		r.Yaw = faceWhite;
		FActorSpawnParameters p{};
		p.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AChessPiecePType* piece = w->SpawnActor<AChessPiecePType>(rookClass, v, r, p);

		if (ensure(piece)) {
			piece->SetTeam(ETeam::Black);
			piece->ReceiveCoordinates(0, 0);
			piece->ReceiveEPieceTypeAllocation(EPiece::Rook);
			allPieces.Push(piece);
			sq->ReceivePiece(piece);
		}

		sq = grid[0][1];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(knightClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 1);
		piece->ReceiveEPieceTypeAllocation(EPiece::Knight);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][2];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(bishopClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 2);
		piece->ReceiveEPieceTypeAllocation(EPiece::Bishop);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][3];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(kingClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 3);
		piece->ReceiveEPieceTypeAllocation(EPiece::King);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][4];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(queenClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 4);
		piece->ReceiveEPieceTypeAllocation(EPiece::Queen);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][5];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(bishopClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 5);
		piece->ReceiveEPieceTypeAllocation(EPiece::Bishop);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][6];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(knightClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 6);
		piece->ReceiveEPieceTypeAllocation(EPiece::Knight);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[0][7];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(rookClass, v, r, p);
		piece->SetTeam(ETeam::Black);
		piece->ReceiveCoordinates(0, 7);
		piece->ReceiveEPieceTypeAllocation(EPiece::Rook);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		for (int i = 0; i < 8; i++) {
			sq = grid[1][i];
			v = sq->GetPieceLocation();
			piece = w->SpawnActor<AChessPiecePType>(pawnClass, v, r, p);
			piece->SetTeam(ETeam::Black);
			piece->ReceiveCoordinates(1, i);
			piece->ReceiveEPieceTypeAllocation(EPiece::Pawn);
			allPieces.Push(piece);
			sq->ReceivePiece(piece);
		}

		sq = grid[7][0];
		v = sq->GetPieceLocation();
		r.Yaw = faceBlack;
		piece = w->SpawnActor<AChessPiecePType>(rookClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 0);
		piece->ReceiveEPieceTypeAllocation(EPiece::Rook);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][1];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(knightClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 1);
		piece->ReceiveEPieceTypeAllocation(EPiece::Knight);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][2];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(bishopClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 2);
		piece->ReceiveEPieceTypeAllocation(EPiece::Bishop);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][3];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(kingClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 3);
		piece->ReceiveEPieceTypeAllocation(EPiece::King);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][4];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(queenClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 4);
		piece->ReceiveEPieceTypeAllocation(EPiece::Queen);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][5];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(bishopClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 5);
		piece->ReceiveEPieceTypeAllocation(EPiece::Bishop);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][6];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(knightClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 6);
		piece->ReceiveEPieceTypeAllocation(EPiece::Knight);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		sq = grid[7][7];
		v = sq->GetPieceLocation();
		piece = w->SpawnActor<AChessPiecePType>(rookClass, v, r, p);
		piece->SetTeam(ETeam::White);
		piece->ReceiveCoordinates(7, 7);
		piece->ReceiveEPieceTypeAllocation(EPiece::Rook);
		allPieces.Push(piece);
		sq->ReceivePiece(piece);

		for (int i = 0; i < 8; i++) {
			sq = grid[6][i];
			v = sq->GetPieceLocation();
			piece = w->SpawnActor<AChessPiecePType>(pawnClass, v, r, p);
			piece->SetTeam(ETeam::White);
			piece->ReceiveCoordinates(6, i);
			piece->ReceiveEPieceTypeAllocation(EPiece::Pawn);
			allPieces.Push(piece);
			sq->ReceivePiece(piece);
		}
	}

	PostBoardSetup();
}



void AChessGameManager::SpawnSquares()
{
	FVector squareLoc{ 0, 0, 0 };
	FRotator rot{};
	float width = 200.f;

	if (!squareClassBlack)
		return;
	if (!squareClassWhite)
		return;

	UWorld* w = GetWorld();
	FActorSpawnParameters params{};

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			TSubclassOf<ASquare> thisSquareType = squareClassBlack;
			if (j % 2 == 1 && i % 2 == 0) {
				thisSquareType = squareClassWhite;
			}
			else if (j % 2 == 0 && i % 2 == 1) {
				thisSquareType = squareClassWhite;
			}

			squareLoc.X = width * i;
			squareLoc.Y = width * j;

			ASquare* thisSq = w->SpawnActor<ASquare>(thisSquareType, squareLoc, rot, params);
			thisSq->ReceiveCoordinates(i, j);
			grid[i][j] = thisSq;
			allSquares.Push(thisSq);
		}
	}
	FVector purgPos = { -9999, -9999, -9999 };
	purgatory = w->SpawnActor<ASquare>(squareClassWhite, purgPos, rot, params);
}

void AChessGameManager::OnClickPiece(AActor* p, int button)
{
	ProcessSquareVacancies();

	switch (gameState) {
	case(EGameState::WhiteTurnNoSelection):
		OnClickPiecePlayerTurnNoFocus(p, button, ETeam::White);
		break;
	case(EGameState::WhiteTurnPieceInFocus):
		OnClickPiecePlayerTurnPieceInFocus(p, button, ETeam::White);
		break;
	case(EGameState::WhiteTurnAnim):
		return;
		break;
	case(EGameState::BlackTurnNoSelection):
		OnClickPiecePlayerTurnNoFocus(p, button, ETeam::Black);
		break;
	case(EGameState::BlackTurnPieceInFocus):
		OnClickPiecePlayerTurnPieceInFocus(p, button, ETeam::Black);
		break;
	case(EGameState::BlackTurnAnim):
		return;
		break;
	}

	if (false) {
		if (pieceInFocus) {
			pieceInFocus->SetIsInFocus(false);
		}
		pieceInFocus = Cast<AChessPiecePType>(p);
		pieceInFocus->SetIsInFocus(true);

		SetGameState(EGameState::WhiteTurnPieceInFocus);
		validSquaresToMove = GetValidMoves(pieceInFocus, true);

		for (ASquare* sq : allSquares) {
			sq->RemoveAsValidMove();
		}

		for (ASquare* sq : validSquaresToMove) {
			sq->SetAsValidMove();
		}
	}
}

void AChessGameManager::OnClickPiecePlayerTurnNoFocus(AActor* p, int button, ETeam thisTeam)
{
	ETeam otherTeam = (thisTeam == ETeam::White) ? ETeam::Black : ETeam::White;

	AChessPiecePType* piece = Cast<AChessPiecePType>(p);
	if (!piece) { return; };

	if (piece->GetTeam() == otherTeam) { return; };

	if (button == 0) { // left click
		if (pieceInFocus) {
			pieceInFocus->SetIsInFocus(false);
		}

		piece->SetIsInFocus(true);
		pieceInFocus = piece;

		validSquaresToMove = GetValidMoves(pieceInFocus, true);

		if (thisTeam == ETeam::White) {
			SetGameState(EGameState::WhiteTurnPieceInFocus);
		}
		else {
			SetGameState(EGameState::BlackTurnPieceInFocus);
		}
	}
}

void AChessGameManager::OnClickPiecePlayerTurnPieceInFocus(AActor* p, int button, ETeam thisTeam)
{	
	ETeam otherTeam = (thisTeam == ETeam::White) ? ETeam::Black : ETeam::White;
	AChessPiecePType* pieceClickedOn = Cast<AChessPiecePType>(p);
	if (!pieceClickedOn) { return; };

	if (!pieceInFocus) { 
		//DeselectAll();
		return; 
	};

	int oldI = pieceInFocus->GetSquareI();
	int oldJ = pieceInFocus->GetSquareJ();

	if (button == 0) {
		if (pieceClickedOn->GetTeam() == thisTeam) {
			if (pieceInFocus) {
				pieceInFocus->SetIsInFocus(false);
			}

			pieceClickedOn->SetIsInFocus(true);
			pieceInFocus = pieceClickedOn;
			validSquaresToMove = GetValidMoves(pieceInFocus, true);
		}
	}
	else if (button == 1) {
		if (pieceClickedOn->GetTeam() == otherTeam) {
			int i = pieceClickedOn->GetSquareI();
			int j = pieceClickedOn->GetSquareJ();
			ASquare* sq = grid[i][j];
			ensure(sq);
			if (validSquaresToMove.Contains(sq)) {
				pieceInFocus->SetTargetAttack(pieceClickedOn, i, j);
				if (gameState == EGameState::WhiteTurnPieceInFocus) {
					SetGameState(EGameState::WhiteTurnAnim);
				}
				else if (gameState == EGameState::BlackTurnPieceInFocus) {
					SetGameState(EGameState::BlackTurnAnim);
				}

				// update vacancy on previous square
				ASquare* squareToVacate = grid[oldI][oldJ];
				if (IsValid(squareToVacate)) {
					squareToVacate->SetIsVacant();
				}

				DeselectAll();
			}
		}
	}
}

void AChessGameManager::TransitionToIdleState()
{
	switch (gameState) {
	case(EGameState::BlackTurnAnim):
		SetGameState(EGameState::WhiteTurnNoSelection);
		break;
	case(EGameState::WhiteTurnAnim):
		SetGameState(EGameState::BlackTurnNoSelection);
		break;
	case(EGameState::BlackTurnNoSelection):
		//gameState = EGameState::WhiteTurnNoSelection;
		break;
	case(EGameState::WhiteTurnNoSelection):
		//gameState = EGameState::BlackTurnNoSelection;
		break;
	case(EGameState::BlackTurnPieceInFocus):
		//gameState = EGameState::WhiteTurnNoSelection;
		break;
	case(EGameState::WhiteTurnPieceInFocus):
		//gameState = EGameState::BlackTurnNoSelection;
		break;
	default:
		break;
	}

	DeselectAll();
}

void AChessGameManager::DeselectAll() {
	for (ASquare* sq : allSquares) {
		sq->RemoveAsValidMove();
	}
	validSquaresToMove.Empty();

	if (pieceInFocus) {
		pieceInFocus->SetIsInFocus(false);
		pieceInFocus = nullptr;
	}
}



ECheckStatus AChessGameManager::SimulateBoardCheckForCheck(int iToMove, int jToMove, int newI, int newJ, ETeam thisTeam)
{
	ETeam otherTeam = thisTeam == ETeam::White ? ETeam::Black : ETeam::White;
	ASquare* sqToMoveFrom = grid[iToMove][jToMove];
	ASquare* sqToMoveTo = grid[newI][newJ];
	AChessPiecePType* pieceToMove = sqToMoveFrom->GetOccupyingPiece();

	ensure(IsValid(pieceToMove));

	sqToMoveFrom->SetIsVacant();
	sqToMoveTo->ReceivePiece(pieceToMove);

	auto undo = [=]() {
		sqToMoveTo->SetIsVacant();
		sqToMoveFrom->ReceivePiece(pieceToMove);
		};

	for (AChessPiecePType* p : allPieces) {
		TArray<ASquare*> simValidMoves = GetValidMoves(p, false);
		for (ASquare* s : simValidMoves) {
			if (s->GetSquareHasKing()) {
				if (s->GetOccupyingPiece()->GetTeam() == ETeam::White) {
					undo();
					return ECheckStatus::WhiteInCheck;
				}
				undo();
				return ECheckStatus::BlackInCheck;
			}
		}
	}
	undo();
	return ECheckStatus::None;
}

ECheckStatus AChessGameManager::CheckCurrentBoardForCheck()
// TODO DL this funtion is unused ?
{
	for (AChessPiecePType* p : allPieces) {
		TArray<ASquare*> possibleMoves = GetValidMoves(p, false);
		for (ASquare* s : possibleMoves) {
			if (s->GetSquareHasKing()) {
				if (s->GetOccupyingPiece()->GetTeam() == ETeam::White) {
					return ECheckStatus::WhiteInCheck;
				}
				return ECheckStatus::BlackInCheck;
			}
		}
	}
	return ECheckStatus::None;
}

bool AChessGameManager::WouldKingBeInCheck(int moveFromI, int moveFromJ, int moveToI, int moveToJ, ETeam thisTeamKing)
{
	ETeam otherTeam = (thisTeamKing == ETeam::White) ? ETeam::Black : ETeam::White;

	ASquare* squareFrom = grid[moveFromI][moveFromJ];
	AChessPiecePType* pieceToMove = squareFrom->GetOccupyingPiece();
	ensure(pieceToMove);
	if (!pieceToMove) { return false; };

	ASquare* squareTo = grid[moveToI][moveToJ];
	AChessPiecePType* pieceInDestination = squareTo->GetOccupyingPiece();

	// Make the temporary move
	squareFrom->SetIsVacant();
	squareTo->ReceivePiece(pieceToMove);
	purgatory->ReceivePiece(pieceInDestination);

	bool kingHasMoved = (pieceToMove->GetEPieceType() == EPiece::King);
	ETeam kingThatMovedTeam;
	if (kingHasMoved) {
		kingThatMovedTeam = pieceToMove->GetTeam();
		if (kingThatMovedTeam == ETeam::White) {
			whiteKingPos = { moveToI, moveToJ };
		}
		else if (kingThatMovedTeam == ETeam::Black) {
			blackKingPos = { moveToI, moveToJ };
		}
	}

	auto undo = [=, this]() {
			squareFrom->ReceivePiece(pieceToMove);
			if (!pieceInDestination) {
				squareTo->SetIsVacant();
			}
			else {
				squareTo->ReceivePiece(pieceInDestination);
			}
			purgatory->SetIsVacant();
			if (kingHasMoved) {
				if (kingThatMovedTeam == ETeam::White) {
					whiteKingPos = { moveFromI, moveFromJ };
				}
				else if (kingThatMovedTeam == ETeam::Black) {
					blackKingPos = { moveFromI, moveFromJ };
				}
			}
		};

	TPair<int, int> kingLoc;
	if (thisTeamKing == ETeam::White) {
		kingLoc = whiteKingPos;
	}
	else if (thisTeamKing == ETeam::Black) {
		kingLoc = blackKingPos;
	}
	ASquare* kingSq = grid[kingLoc.Key][kingLoc.Value];

	for (AChessPiecePType* p : allPieces) {
		if (p->GetTeam() == otherTeam) {
			if (p == pieceInDestination) { continue; };
			TArray<ASquare*> possibleMoves = GetValidMoves(p, false);

			if (possibleMoves.Contains(kingSq)) {
				undo();
				return true;
			}
		}
	}
	undo();
	return false;
}



void AChessGameManager::OnClickSquare(AActor* square, int button)
{
	if (button == 0) {
		DeselectAll();
		if (gameState == EGameState::BlackTurnPieceInFocus) {
			SetGameState(EGameState::BlackTurnNoSelection);
		}
		else if (gameState == EGameState::WhiteTurnPieceInFocus) {
			SetGameState(EGameState::WhiteTurnNoSelection);
		}
		return;
	}
	switch (gameState) {
	case(EGameState::WhiteTurnPieceInFocus):
		if (validSquaresToMove.Contains(square)) {
			ExecuteMove(Cast<ASquare>(square), ETeam::White, button);
		}
		else {
			SetGameState(EGameState::WhiteTurnNoSelection);
			DeselectAll();
		}
		break;
	case(EGameState::BlackTurnPieceInFocus):
		if (validSquaresToMove.Contains(square)) {
			ExecuteMove(Cast<ASquare>(square), ETeam::Black, button);
		}
		else {
			SetGameState(EGameState::BlackTurnNoSelection);
			DeselectAll();
		}
		break;
	case(EGameState::BlackTurnNoSelection):
		// TODO DL see if there is a piece on this square 
		// and if so set that to be selected
		break;
	case(EGameState::WhiteTurnNoSelection):
		// TODO DL see if there is a piece on this square 
		// and if so set that to be selected
		break;
	default:
		break;
	}
}

void AChessGameManager::ExecuteMove(ASquare* square, ETeam team, int button)
{
	if (!square) { return; };

	// set animation path to new square
	int i = square->GetI();
	int j = square->GetJ();

	// cache old co-ords for later
	int oldI = pieceInFocus->GetSquareI();
	int oldJ = pieceInFocus->GetSquareJ();

	AChessPiecePType* occupyingPiece = square->GetOccupyingPiece();
	if (IsValid(occupyingPiece)) {
		OnClickPiecePlayerTurnPieceInFocus(occupyingPiece, button, team);
	}
	else { // i.e. unoccupied square
		if (team == ETeam::Black) {
			SetGameState(EGameState::BlackTurnAnim);
			pieceInFocus->SetMoveTarget(square->GetPieceLocation(), i, j);
		}
		else if (team == ETeam::White) {
			SetGameState(EGameState::WhiteTurnAnim);
			pieceInFocus->SetMoveTarget(square->GetPieceLocation(), i, j);
		}
		// release occupancy from previous square
		ASquare* squareToVacate = grid[oldI][oldJ];
		if (IsValid(squareToVacate)) {
			squareToVacate->SetIsVacant();
		}

		DeselectAll();
	}
}

void AChessGameManager::CastleRook(int fromI, int fromJ, int toI, int toJ)
{
	AChessPiecePType* rook = grid[fromI][fromJ]->GetOccupyingPiece();
	ensure(rook);
	grid[fromI][fromJ]->SetIsVacant();
	grid[fromI][fromJ]->SpawnSmokeFX();
	grid[toI][toJ]->ReceivePiece(rook);
	rook->ReceiveCoordinates(toI, toJ);
	FVector l = grid[toI][toJ]->GetPieceLocation();
	rook->SetActorLocation(l);
	rook->IncrementMovesMade();

	MoveRecord rec;
	rec.fromI = fromI;
	rec.fromJ = fromJ;
	rec.pieceThatMoved = rook;
	rec.pieceTaken = nullptr;
	rec.toI = toI;
	rec.toJ = toJ;
	rec.isMoveTwoOfACastling = false;
	// 'move one' of the castling is the rook's move, which is recorded elsewhere
	RecordMoveInHistory(rec);
}

void AChessGameManager::FaceNearestEnemy(AChessPiecePType* thisPiece)
{
	ETeam thisTeam = thisPiece->GetTeam();
	ETeam otherTeam = (thisTeam == ETeam::White) ? ETeam::Black : ETeam::White;
	
	FVector v1 = thisPiece->GetActorLocation();
	float smallest = BIG_NUMBER;
	FVector closestTarget{};

	for (AChessPiecePType* p : allPieces) {
		if (p->GetTeam() != otherTeam) { continue; };

		FVector v2 = p->GetActorLocation();
		float dist = KM::Vector_Distance(v1, v2);
		if (dist < smallest) {
			smallest = dist;
			closestTarget = v2;
		}
	}

	FRotator r = KM::FindLookAtRotation(v1, closestTarget);
	thisPiece->SetActorRotation(r);
}

void AChessGameManager::OnMoveCompleted(AChessPiecePType* pieceThatMoved)
{	// Comes from blueprints
	pieceThatMoved->IncrementMovesMade();
	int i = pieceThatMoved->GetSquareI();
	int j = pieceThatMoved->GetSquareJ();
	int previousI = pieceThatMoved->GetPreviousI();
	int previousJ = pieceThatMoved->GetPreviousJ();
	EPiece pieceType = pieceThatMoved->GetEPieceType();
	ETeam thisTeam = pieceThatMoved->GetTeam();

	// evaluate if it was a castling move
	bool wasCastling = false;
	if (pieceType == EPiece::King) {
		if (thisTeam == ETeam::White) {
			if (previousI == 7 && previousJ == 3) {
				if (i == 7 && j == 1) {
					CastleRook(7, 0, 7, 2);
					wasCastling = true;
				}
				else if (i == 7 && j == 5) {
					CastleRook(7, 7, 7, 4);
					wasCastling = true;
				}
			}
		}
		else {
			if (previousI == 0 && previousJ == 3) {
				if (i == 0 && j == 1) {
					CastleRook(0, 0, 0, 2);
					wasCastling = true;
				}
				else if (i == 0 && j == 5) {
					CastleRook(0, 7, 0, 4);
					wasCastling = true;
				}
			}
		}
	}

	ASquare* sq = grid[i][j];
	sq->ReceivePiece(pieceThatMoved);

	CalculateMoveJiggle(previousI, previousJ, i, j, pieceType);

	// update tracked king positions
	if (pieceType == EPiece::King) {
		if (pieceThatMoved->GetTeam() == ETeam::White) {
			whiteKingPos = { i, j };
		}
		else if (pieceThatMoved->GetTeam() == ETeam::Black) {
			blackKingPos = {i, j};
		}

		if (IsSquareValid(previousI, previousJ)) {
			grid[previousI][previousJ]->SetIsInCheck(false);
		}
	}

	// eval if it is a pawn reaching the end row
	if (pieceType == EPiece::Pawn) {
		if (i == 0 || i == 7) {
			allPieces.Remove(pieceThatMoved);
			pieceThatMoved->SetActorLocation(purgatory->GetPieceLocation());
			FActorSpawnParameters params{};
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AChessPiecePType* newPiece = GetWorld()->SpawnActor<AChessPiecePType>(
				queenClass, grid[i][j]->GetPieceLocation(), FRotator{},
				params);
			if (ensure(newPiece)) {
				grid[i][j]->ReceivePiece(newPiece);
				allPieces.Push(newPiece);
				ETeam t = (i == 0) ? ETeam::White : ETeam::Black;
				newPiece->SetTeam(t);
				newPiece->ReceiveCoordinates(i, j);
				newPiece->ReceiveEPieceTypeAllocation(EPiece::Queen);
				RegisterNewUnit(newPiece);
			}
		}
	}

	TransitionToIdleState();
	ProcessSquareVacancies();
	whiteKingInCheck = false;
	blackKingInCheck = false;
	ASquare* whiteKingSquare = grid[whiteKingPos.Key][whiteKingPos.Value];
	ASquare* blackKingSquare = grid[blackKingPos.Key][blackKingPos.Value];
	whiteKingSquare->SetIsInCheck(false);
	blackKingSquare->SetIsInCheck(false);

	for (AChessPiecePType* p : allPieces) {
		if (p->GetPieceState() == EPieceState::Death) {
			continue;
		}
		if (p->GetPieceState() == EPieceState::DeathCleanUp) {
			continue;
		}
		TArray<ASquare*> possibleMoves = GetValidMoves(p, false);

		if (possibleMoves.Contains(whiteKingSquare)) {
			if (p->GetTeam() == ETeam::Black) {
				whiteKingInCheck = true;
				whiteKingSquare->SetIsInCheck(true);
				if (GetIsCheckMate(ETeam::White)) {
					OnEventCheckMate(ETeam::White);
				}
			}
		}
		else if (possibleMoves.Contains(blackKingSquare)) {
			if (p->GetTeam() == ETeam::White) {
				blackKingInCheck = true;
				blackKingSquare->SetIsInCheck(true);
				if (GetIsCheckMate(ETeam::Black)) {
					OnEventCheckMate(ETeam::Black);
				}
			}
		}
	}

	MoveRecord rec;
	rec.fromI = pieceThatMoved->GetPreviousI();
	rec.fromJ = pieceThatMoved->GetPreviousJ();
	rec.pieceThatMoved = pieceThatMoved;
	rec.pieceTaken = pieceThatMoved->GetAttackTarget(); // could be nullptr
	rec.toI = i;
	rec.toJ = j;
	rec.isMoveTwoOfACastling = wasCastling;
	// 'move one' of the castling is the rook's move, which is recorded elsewhere
	RecordMoveInHistory(rec);
}

void AChessGameManager::RecordMoveInHistory(MoveRecord rec)
{
	movesHistory.Push(rec);
}

bool AChessGameManager::GetIsCheckMate(ETeam teamInCheck)
{
	for (AChessPiecePType* p : allPieces) {
		if (p->GetTeam() != teamInCheck) { continue; };

		TArray<ASquare*> possibleMoves = GetValidMoves(p, true);
		if (possibleMoves.Num() > 0) {
			return false;
		}
	}

	return true;
}

void AChessGameManager::OnEventCheckMate(ETeam teamLost)
{
	SetGameState(EGameState::Checkmate);

	int squaresFallCount = 0;
	for (ASquare* s : allSquares) {
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!IsValid(p)) {
			s->CheckmateAction();
			squaresFallCount++;
		}
		else if (p->GetTeam() == teamLost) {
			s->CheckmateAction();
			squaresFallCount++;
		}
	}

	TilesFallEvent(squaresFallCount); // to blueprints
}

void AChessGameManager::SetGameState(EGameState state)
{
	gameState = state;
	bool shouldResetSelectableIndicators = true;

	switch (state) {
	case(EGameState::InbetweenGames):
		menuScreen->SetVisibility(ESlateVisibility::Visible);
		break;
	case(EGameState::WhiteTurnNoSelection):
		for (ASquare* s : allSquares) {
			AChessPiecePType* p = s->GetOccupyingPiece();
			if (p) {
				if (p->GetTeam() == ETeam::White) {
					s->SetSelectableIndicator(true);
				}
			}
		}
		shouldResetSelectableIndicators = false;
		break;
	case(EGameState::WhiteTurnPieceInFocus):
		break;
	case(EGameState::WhiteTurnAnim):
		break;
	case(EGameState::BlackTurnNoSelection):
		for (ASquare* s : allSquares) {
			AChessPiecePType* p = s->GetOccupyingPiece();
			if (p) {
				if (p->GetTeam() == ETeam::Black) {
					s->SetSelectableIndicator(true);
				}
			}
		}
		shouldResetSelectableIndicators = false;
		break;
	case(EGameState::BlackTurnPieceInFocus):
		break;
	case(EGameState::BlackTurnAnim):
		break;
	case(EGameState::Checkmate):
		break;
	case(EGameState::Stalemate):
		break;
	}

	if (shouldResetSelectableIndicators) {
		for (ASquare* s : allSquares) {
			s->SetSelectableIndicator(false);
		}
	}
}

void AChessGameManager::CalculateMoveJiggle(int oldI, int oldJ, int newI, int newJ, EPiece pieceType)
{
	if (oldI == newI) {
		// same row
		if (oldJ > newJ) {
			for (int j = oldJ - 1; j > newJ; j--) {
				TryJiggle(oldI, j);
			}
		}
		else if (oldJ < newJ) {
			for (int j = oldJ + 1; j < newJ; j++) {
				TryJiggle(oldI, j);
			}
		}
	}
	else if (oldJ == newJ) {
		// same col
		if (oldI < newI) {
			for (int i = oldI + 1; i < newI; i++) {
				TryJiggle(i, oldJ);
			}
		}
		else if (oldI > newI) {
			for (int i = oldI - 1; i > newI; i--) {
				TryJiggle(i, oldJ);
			}
		}
	}
	else if (FMath::Abs((oldI - newI)) == FMath::Abs((oldJ - newJ))) {
		// is diagonal
		if (oldI < newI) {
			for (int i = 1; (i + oldI) < newI; i++) {
				if (oldJ < newJ) {
					TryJiggle(i + oldI, i + oldJ);
				}
				else if (oldJ > newJ) {
					TryJiggle(i + oldI, oldJ - i);
				}
			}
		}
		else if (newI < oldI) {
			for (int i = 1; newI+i < oldI; i++) {
				if (oldJ < newJ) {
					TryJiggle(i + newI, i + oldJ);
				}
				else if (newJ < oldJ) {
					TryJiggle(i - newI, newJ + i);
				}
			}
		}
	}
	else if (pieceType == EPiece::Knight) {
		if (oldI - newI == 2) {
			TryJiggle(oldI - 1, oldJ);
			if (oldJ < newJ) {
				TryJiggle(oldI - 1, oldJ + 1);
			}
			else {
				TryJiggle(oldI - 1, oldJ - 1);
			}
		}
		else if (newI - oldI == 2) {
			TryJiggle(newI - 1, oldJ);
			if (oldJ < newJ) {
				TryJiggle(newI - 1, oldJ + 1);
			}
			else {
				TryJiggle(newI - 1, oldJ - 1);
			}
		}
		else if (oldJ - newJ == 2) {
			TryJiggle(oldI, oldJ - 1);
			if (oldI < newI) {
				TryJiggle(oldI + 1, oldJ - 1);
			}
			else {
				TryJiggle(oldI - 1, oldJ - 1);
			}
		}
		else if (newJ - oldJ == 2) {
			TryJiggle(oldI, newJ - 1);
			if (oldI < newI) {
				TryJiggle(oldI + 1, newJ - 1);
			}
			else {
				TryJiggle(oldI - 1, newJ - 1);
			}
		}
	}

}

void AChessGameManager::TryJiggle(int i, int j)
{
	if (IsSquareValid(i, j)) {
		grid[i][j]->MovementJiggle();
	}
}

void AChessGameManager::OnClickPlayButton()
{
	NewGame();
	menuScreen->SetVisibility(ESlateVisibility::Hidden);
}

void AChessGameManager::ProcessSquareVacancies() {
	for (ASquare* s : allSquares) {
		if (!IsValid(s->GetOccupyingPiece())) {
			s->SetIsVacant();
		}
	}
}

TArray<ASquare*> AChessGameManager::GetValidMoves(AChessPiecePType* pieceToGetMovesFor, bool checkForCheck)
{
	TArray<ASquare*> validSquares;
	if (!IsValid(pieceToGetMovesFor)) { return validSquares; };

	if (pieceToGetMovesFor->GetPieceState() == EPieceState::DeathCleanUp) {
		return validSquares;
	}

	int focusI = pieceToGetMovesFor->GetSquareI();
	int focusJ = pieceToGetMovesFor->GetSquareJ();

	EPiece type = pieceToGetMovesFor->GetEPieceType();
	ETeam focusTeam = pieceToGetMovesFor->GetTeam();
	bool hasMoved = pieceToGetMovesFor->GetHasMoved();

	switch (type) {
	case(EPiece::Pawn):
		validSquares = GetValidMovesPawn(pieceToGetMovesFor, focusI, focusJ, focusTeam, hasMoved);
		break;
	case(EPiece::Rook):
		validSquares = GetValidMovesRook(pieceToGetMovesFor, focusI, focusJ, focusTeam);
		break;
	case(EPiece::Knight):
		validSquares = GetValidMovesKnight(pieceToGetMovesFor, focusI, focusJ, focusTeam);
		break;
	case(EPiece::Bishop):
		validSquares = GetValidMovesBishop(pieceToGetMovesFor, focusI, focusJ, focusTeam);
		break;
	case(EPiece::Queen):
		validSquares = GetValidMovesQueen(pieceToGetMovesFor, focusI, focusJ, focusTeam);
		break;
	case(EPiece::King):
		validSquares = GetValidMovesKing(pieceToGetMovesFor, focusI, focusJ, focusTeam);
		break;
	default:
		break;
	}
	
	if (pieceToGetMovesFor == pieceInFocus || checkForCheck) {
		for (int i = validSquares.Num() - 1; i >= 0; i--) {
			ASquare* s = validSquares[i];
			int iToCheck = s->GetI();
			int jToCheck = s->GetJ();
			if (WouldKingBeInCheck(focusI, focusJ, iToCheck, jToCheck, focusTeam)) {
				validSquares.Remove(s);
			}
		}

		for (ASquare* sq : allSquares) {
			sq->RemoveAsValidMove();
		}

		for (ASquare* sq : validSquares) {
			sq->SetAsValidMove();
		}
	}
	// else we are doing a simulated move
	
	return validSquares;
}

TArray<ASquare*> AChessGameManager::GetValidMovesPawn(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team, bool hasMoved)
{
	TArray<ASquare*> squares;
	
	if (team == ETeam::Black) {
		ASquare* sq = grid[focusI+1][focusJ];
		bool firstSquareFree = false;
		if (sq->GetOccupyingPiece() == nullptr) {
			squares.Push(sq);
			firstSquareFree = true;
		}
		if (firstSquareFree && !hasMoved) {
			sq = grid[focusI+2][focusJ];
			if (sq->GetOccupyingPiece() == nullptr) {
				squares.Push(sq);
			}
		}
		if (IsSquareValid(focusI+1, focusJ-1)) {
			sq = grid[focusI + 1][focusJ - 1];
			if (sq->GetOccupyingPiece()) {
				if (sq->GetOccupyingPiece()->GetTeam() == ETeam::White) {
					squares.Push(sq);
				}
			}
		}
		if (IsSquareValid(focusI+1, focusJ+1)) {
			sq = grid[focusI + 1][focusJ + 1];
			if (sq->GetOccupyingPiece()) {
				if (sq->GetOccupyingPiece()->GetTeam() == ETeam::White) {
					squares.Push(sq);
				}
			}
		}
	}
	else if (team == ETeam::White) {
		ASquare* sq = grid[focusI-1][focusJ];
		bool firstSquareFree = false;
		if (sq->GetOccupyingPiece() == nullptr) {
			squares.Push(sq);
			firstSquareFree = true;
		}
		if (firstSquareFree && !hasMoved) {
			sq = grid[focusI-2][focusJ];
			if (sq->GetOccupyingPiece() == nullptr) {
				squares.Push(sq);
			}
		}
		if (IsSquareValid(focusI-1, focusJ-1)) {
			sq = grid[focusI - 1][focusJ - 1];
			if (IsValid(sq->GetOccupyingPiece())) {
				if (sq->GetOccupyingPiece()->GetTeam() == ETeam::Black) {
					squares.Push(sq);
				}
			}
		}
		if (IsSquareValid(focusI - 1, focusJ + 1)) {
			sq = grid[focusI - 1][focusJ + 1];
			if (sq->GetOccupyingPiece()) {
				if (sq->GetOccupyingPiece()->GetTeam() == ETeam::Black) {
					squares.Push(sq);
				}
			}
		}
	}
	
	return squares;
}

TArray<ASquare*> AChessGameManager::GetValidMovesRook(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team)
{
	TArray<ASquare*> squares;

	ETeam otherTeam = (team == ETeam::White) ? ETeam::Black : ETeam::White;
	// Go Left
	for (int shift = 1; shift <= 7; shift++) {
		if (!IsSquareValid(focusI - shift, focusJ)) {
			break;
		}
		ASquare* sq = grid[focusI - shift][focusJ];
		AChessPiecePType* occPiece = grid[focusI - shift][focusJ]->GetOccupyingPiece();
		if (!IsValid(occPiece)) {
			squares.Push(sq);
		}
		else if (occPiece && occPiece->GetTeam() == otherTeam) {
			squares.Push(sq);
			break;
		}
		else if (occPiece && occPiece->GetTeam() == team) {
			break;
		}
	}
	// Go Right
	for (int shift = 1; shift <= 7; shift++) {
		if (!IsSquareValid(focusI + shift, focusJ)) {
			break;
		}
		ASquare* sq = grid[focusI + shift][focusJ];
		AChessPiecePType* occPiece = grid[focusI + shift][focusJ]->GetOccupyingPiece();
		if (!IsValid(occPiece)) {
			squares.Push(sq);
		}
		else if (occPiece && occPiece->GetTeam() == otherTeam) {
			squares.Push(sq);
			break;
		}
		else if (occPiece && occPiece->GetTeam() == team) {
			break;
		}
	}
	// Go Up
	for (int shift = 1; shift <= 7; shift++) {
		if (!IsSquareValid(focusI, focusJ+shift)) {
			break;
		}
		ASquare* sq = grid[focusI][focusJ+shift];
		AChessPiecePType* occPiece = grid[focusI][focusJ+shift]->GetOccupyingPiece();
		if (occPiece == nullptr) {
			squares.Push(sq);
		}
		else if (occPiece->GetTeam() == otherTeam) {
			squares.Push(sq);
			break;
		}
		else if (occPiece->GetTeam() == team) {
			break;
		}
	}
	// Go Down
	for (int shift = 1; shift <= 7; shift++) {
		if (!IsSquareValid(focusI, focusJ - shift)) {
			break;
		}
		ASquare* sq = grid[focusI][focusJ - shift];
		AChessPiecePType* occPiece = grid[focusI][focusJ - shift]->GetOccupyingPiece();
		if (occPiece == nullptr) {
			squares.Push(sq);
		}
		else if (occPiece->GetTeam() == otherTeam) {
			squares.Push(sq);
			break;
		}
		else if (occPiece->GetTeam() == team) {
			break;
		}
	}
	return squares;
}

TArray<ASquare*> AChessGameManager::GetValidMovesKnight(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team)
{
	TArray<ASquare*> squares;
	ETeam otherTeam = (team == ETeam::White) ? ETeam::Black : ETeam::White;

	if (IsSquareValid(focusI - 2, focusJ - 1)) {
		AChessPiecePType* occPiece = grid[focusI-2][focusJ - 1]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI - 2][focusJ - 1]);
		}
	}
	if (IsSquareValid(focusI - 2, focusJ + 1)) {
		AChessPiecePType* occPiece = grid[focusI - 2][focusJ + 1]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI - 2][focusJ + 1]);
		}
	}
	if (IsSquareValid(focusI + 2, focusJ - 1)) {
		AChessPiecePType* occPiece = grid[focusI + 2][focusJ - 1]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI + 2][focusJ - 1]);
		}
	}
	if (IsSquareValid(focusI + 2, focusJ + 1)) {
		AChessPiecePType* occPiece = grid[focusI + 2][focusJ + 1]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI + 2][focusJ + 1]);
		}
	}
	if (IsSquareValid(focusI - 1, focusJ - 2)) {
		AChessPiecePType* occPiece = grid[focusI - 1][focusJ - 2]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI - 1][focusJ - 2]);
		}
	}
	if (IsSquareValid(focusI - 1, focusJ + 2)) {
		AChessPiecePType* occPiece = grid[focusI - 1][focusJ + 2]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI - 1][focusJ + 2]);
		}
	}
	if (IsSquareValid(focusI + 1, focusJ - 2)) {
		AChessPiecePType* occPiece = grid[focusI + 1][focusJ - 2]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI + 1][focusJ - 2]);
		}
	}
	if (IsSquareValid(focusI + 1, focusJ + 2)) {
		AChessPiecePType* occPiece = grid[focusI + 1][focusJ + 2]->GetOccupyingPiece();
		if (!occPiece || occPiece->GetTeam() == otherTeam) {
			squares.Push(grid[focusI + 1][focusJ + 2]);
		}
	}

	return squares;
}

TArray<ASquare*> AChessGameManager::GetValidMovesBishop(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam)
{
	ETeam otherTeam = focusTeam == ETeam::White ? ETeam::Black : ETeam::White;
	TArray<ASquare*> squares;

	for (int i = 1; i < 8; i++) {
		if (IsSquareValid(focusI+i, focusJ+i)) {
			ASquare* sq = grid[focusI+i][focusJ+i];
			if (sq->GetIsVacant()) {
				squares.Add(sq);
			}
			else if (IsValid(sq->GetOccupyingPiece())) {
				AChessPiecePType* p = sq->GetOccupyingPiece();
				if (p->GetTeam() == otherTeam) {
					squares.Add(sq);
				}
				break;
			}
		}
		else {
			break;
		}
	}
	for (int i = 1; i < 8; i++) {
		if (IsSquareValid(focusI+i, focusJ-i)) {
			ASquare* sq = grid[focusI+i][focusJ-i];
			if (sq->GetIsVacant()) {
				squares.Add(sq);
			}
			else if (IsValid(sq->GetOccupyingPiece())) {
				AChessPiecePType* p = sq->GetOccupyingPiece();
				if (p->GetTeam() == otherTeam) {
					squares.Add(sq);
				}
				break;
			}
		}
	}
	for (int i = 1; i < 8; i++) {
		if (IsSquareValid(focusI-i, focusJ+i)) {
			ASquare* sq = grid[focusI-i][focusJ+i];
			if (sq->GetIsVacant()) {
				squares.Add(sq);
			}
			else if (IsValid(sq->GetOccupyingPiece())) {
				AChessPiecePType* p = sq->GetOccupyingPiece();
				if (p->GetTeam() == otherTeam) {
					squares.Add(sq);
				}
				break;
			}
		}
	}
	for (int i = 1; i < 8; i++) {
		if (IsSquareValid(focusI-i, focusJ-i)) {
			ASquare* sq = grid[focusI-i][focusJ-i];
			if (sq->GetIsVacant()) {
				squares.Add(sq);
			}
			else if (IsValid(sq->GetOccupyingPiece())) {
				AChessPiecePType* p = sq->GetOccupyingPiece();
				if (p->GetTeam() == otherTeam) {
					squares.Add(sq);
				}
				break;
			}
		}
	}
	return squares;
}

TArray<ASquare*> AChessGameManager::GetValidMovesQueen(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam)
{
	TArray<ASquare*> squaresTotal = GetValidMovesRook(pieceToGetMovesFor, focusI, focusJ, focusTeam);
	TArray<ASquare*> diags = GetValidMovesBishop(pieceToGetMovesFor, focusI, focusJ, focusTeam);
	for (ASquare* s : diags) {
		if (!squaresTotal.Contains(s)) {
			squaresTotal.Add(s);
		}
	}

	return squaresTotal;
}

TArray<ASquare*> AChessGameManager::GetValidMovesKing(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam)
{
	TArray<ASquare*> moves;
	const int& i = focusI;
	const int& j = focusJ;
	ETeam otherTeam = (focusTeam == ETeam::White) ? ETeam::Black : ETeam::White;
	bool kingHasMoved = pieceToGetMovesFor->GetHasMoved();
	if (!kingHasMoved) {
		// evaluate castling
		// white rooks known to be at (7, 0), (7, 7)
		// black rooks known to be at (0, 0), (0, 7)
		if (focusTeam == ETeam::White) {
			AChessPiecePType* r1 = grid[7][0]->GetOccupyingPiece();
			if (r1 && r1->GetEPieceType() == EPiece::Rook && !r1->GetHasMoved()) {
				if (grid[7][1]->GetIsVacant() && grid[7][2]->GetIsVacant()) {
					moves.Push(grid[7][1]);
				}
			}
			AChessPiecePType* r2 = grid[7][7]->GetOccupyingPiece();
			if (r2 && r2->GetEPieceType() == EPiece::Rook && !r2->GetHasMoved()) {
				if (grid[7][6]->GetIsVacant() && 
					grid[7][5]->GetIsVacant() && 
					grid[7][4]->GetIsVacant()) {
					moves.Push(grid[7][5]);
				}
			}
		}
		else if (focusTeam == ETeam::Black) {
			AChessPiecePType* r1 = grid[0][0]->GetOccupyingPiece();
			if (r1 && r1->GetEPieceType() == EPiece::Rook && !r1->GetHasMoved()) {
				if (grid[0][1]->GetIsVacant() && grid[0][2]->GetIsVacant()) {
					moves.Push(grid[0][1]);
				}
			}
			AChessPiecePType* r2 = grid[0][7]->GetOccupyingPiece();
			if (r2 && r2->GetEPieceType() == EPiece::Rook && !r2->GetHasMoved()) {
				if (grid[0][6]->GetIsVacant() &&
					grid[0][5]->GetIsVacant() &&
					grid[0][4]->GetIsVacant()) {
					moves.Push(grid[0][5]);
				}
			}
		}
	}

	if (IsSquareValid(i - 1, j - 1)) {
		ASquare* s = grid[i - 1][j - 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i, j - 1)) {
		ASquare* s = grid[i][j - 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i+1, j - 1)) {
		ASquare* s = grid[i + 1][j - 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i - 1, j)) {
		ASquare* s = grid[i - 1][j];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i - 1, j + 1)) {
		ASquare* s = grid[i - 1][j + 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i + 1, j)) {
		ASquare* s = grid[i + 1][j];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i + 1, j + 1)) {
		ASquare* s = grid[i + 1][j + 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}
	if (IsSquareValid(i, j + 1)) {
		ASquare* s = grid[i][j + 1];
		AChessPiecePType* p = s->GetOccupyingPiece();
		if (!p || (p->GetTeam() == otherTeam)) {
			moves.Add(s);
		}
	}

	return moves;
}


bool AChessGameManager::IsSquareValid(int i, int j)
{
	if (i < 0) { return false; };
	if (i > 7) { return false; };
	if (j < 0) { return false; };
	if (j > 7) { return false; };
	return true;
}


void AChessGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (AChessPiecePType* p : allPieces) {
		if (p->GetPieceState() == EPieceState::DeathCleanUp) {
			p->SetActorLocation(benchLocation);
			p->ReceiveCoordinates(-1, -1);
			allPieces.Remove(p);
			DeselectAll();
			TransitionToIdleState();
			break;
		}
	}

	if (gameState == EGameState::Checkmate) {
		checkMateCounter += DeltaTime;
		if (checkMateCounter > checkMateWaitTime) {
			SetGameState(EGameState::InbetweenGames);
			checkMateCounter = 0.f;
		}
	}
}

