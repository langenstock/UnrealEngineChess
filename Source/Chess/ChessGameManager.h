// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Square.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

#include "ChessGameManager.generated.h"

UENUM(BlueprintType) 
enum class EGameState : uint8 {
	WhiteTurnNoSelection, WhiteTurnPieceInFocus, WhiteTurnAnim,
	BlackTurnNoSelection, BlackTurnPieceInFocus, BlackTurnAnim,
	Checkmate, Stalemate, InbetweenGames
};

UENUM(BlueprintType)
enum class ECheckStatus : uint8 {
	None, WhiteInCheck, BlackInCheck, Stalemate
};

struct MoveRecord
{
	AChessPiecePType* pieceThatMoved;
	AChessPiecePType* pieceTaken; // could be none
	int fromI;
	int fromJ;
	int toI;
	int toJ;
	bool isMoveTwoOfACastling;
};


UCLASS()
class CHESS_API AChessGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChessGameManager();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	EGameState GetGameState() const {return gameState; };

	UFUNCTION(BlueprintCallable)
	void UndoMove(bool switchSides);

protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintImplementableEvent)
	void PostBoardSetup();
	UFUNCTION(BlueprintCallable)
	void OnClickPiece(AActor* p, int button);

	UFUNCTION(BlueprintCallable)
	void OnClickSquare(AActor* square, int button);
	void ExecuteMove(ASquare* square, ETeam team, int button);

	UFUNCTION(BlueprintCallable)
	void OnMoveCompleted(AChessPiecePType* pieceThatMoved);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMoveUndoSuccessful();

	void ProcessSquareVacancies();

	TArray<ASquare*> GetValidMoves(AChessPiecePType* pieceToGetMovesFor, bool checkForCheck);
	TArray<ASquare*> GetValidMovesPawn(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team, bool hasMoved);
	TArray<ASquare*> GetValidMovesRook(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team);
	TArray<ASquare*> GetValidMovesKnight(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam team);
	TArray<ASquare*> GetValidMovesBishop(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam);
	TArray<ASquare*> GetValidMovesQueen(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam);
	TArray<ASquare*> GetValidMovesKing(AChessPiecePType* pieceToGetMovesFor, int focusI, int focusJ, ETeam focusTeam);

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> menuScreenClass;
	UUserWidget* menuScreen;

	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveMenuWidget(UUserWidget* menu);

	UFUNCTION(BlueprintImplementableEvent)
	void TilesFallEvent(int tilesToFall);

	UFUNCTION(BlueprintImplementableEvent)
	void RegisterNewUnit(AChessPiecePType* newPiece);

	//UPROPERTY(EditAnywhere)
	//AActor* UndoButton;

private:
	void NewGame();
	void SetUpBoard();
	void SetUpDebugBoard();
	void SpawnPiece(int i, int j, TSubclassOf<AChessPiecePType> cls, ETeam team);
	void FindAndStoreKingPositions();
	void SpawnSquares();
	bool IsSquareValid(int i, int j);
	TArray<ASquare*> validSquaresToMove;
	void OnClickPiecePlayerTurnNoFocus(AActor* p, int button, ETeam thisTeam);
	void OnClickPiecePlayerTurnPieceInFocus(AActor* p, int button, ETeam thisTeam);
	void OnClickPieceBlackTurnNoFocus(AActor* p, int button);
	void OnClickPieceBlackTurnPieceInFocus(AActor* p, int button);

	void TransitionToIdleState();
	void DeselectAll();
	void RecordMoveInHistory(MoveRecord rec);

	ECheckStatus SimulateBoardCheckForCheck(int iToMove, int jToMove, int newI, int newJ, ETeam thisTeam);
	bool WouldKingBeInCheck(int moveFromI, int moveFromJ, int moveToI, int moveToJ, ETeam thisTeamKing);
	bool GetIsCheckMate(ETeam teamInCheck);
	void OnEventCheckMate(ETeam teamLost);

	void SetGameState(EGameState state);

	void CalculateMoveJiggle(int oldI, int oldJ, int newI, int newJ, EPiece pieceType);
	void TryJiggle(int i, int j);
	void CastleRook(int fromI, int fromJ, int toI, int toJ);

	void FaceNearestEnemy(AChessPiecePType* thisPiece);

	EPiece GetPieceTypeFromSubclass(TSubclassOf<AChessPiecePType> cls);
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ASquare> squareClassWhite;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ASquare> squareClassBlack;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> pawnClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> rookClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> knightClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> bishopClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> kingClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AChessPiecePType> queenClass;

	ASquare* grid[8][8];
	ASquare* purgatory;
	UPROPERTY(BlueprintReadWrite)
	TArray<ASquare*> allSquares;
	AChessPiecePType* pieceInFocus;
	UPROPERTY(BlueprintReadWrite)
	TArray<AChessPiecePType*> allPieces;

	UPROPERTY(BlueprintReadOnly)
	EGameState gameState = EGameState::InbetweenGames;

	UFUNCTION(BlueprintCallable)
	void OnClickPlayButton();

	UPROPERTY(EditAnywhere)
	bool debugBoard = true;

private:
	float m_FaceBlack = -180.f;
	float m_FaceWhite = 0.f;
	TPair<int, int> m_WhiteKingPos;
	TPair<int, int> m_BlackKingPos;
	bool m_WhiteKingInCheck;
	bool m_BlackKingInCheck;
	TArray<MoveRecord> m_MovesHistory;
	FVector m_BenchLocation = { -888, 888, -888 };

	float m_CheckMateWaitTime = 3.f;
	float m_CheckMateCounter = 0.f;

};
