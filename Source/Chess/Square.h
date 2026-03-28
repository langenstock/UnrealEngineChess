// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiecePType.h"
#include "Square.generated.h"

UCLASS()
class CHESS_API ASquare : public AActor
{
	GENERATED_BODY()
	
public:	
	ASquare();
	virtual void Tick(float DeltaTime) override;

	void ReceiveCoordinates(int I, int J);
	void ReceivePiece(AChessPiecePType* piece);
	UFUNCTION(BlueprintImplementableEvent)
	void SetAsValidMove();
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveAsValidMove();
	
	AChessPiecePType* GetOccupyingPiece() const { return m_OccupyingPiece; };
	void SetIsVacant();
	bool GetIsVacant() const;
	bool GetSquareHasKing(ETeam kingsTeam) const;
	bool GetSquareHasKing() const;
	UFUNCTION(BlueprintImplementableEvent) 
	void SetIsInCheck(bool inCheck);

	UFUNCTION(BlueprintImplementableEvent)
	void SetSelectableIndicator(bool visible);
	
	UFUNCTION(BlueprintCallable)
	int GetI() const { return m_I; };
	UFUNCTION(BlueprintCallable)
	int GetJ() const { return m_J; };
	FVector GetPieceLocation();

	void CheckmateAction();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_CheckmateAction();
	
	UFUNCTION(BlueprintImplementableEvent)
	void MovementJiggle();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnSmokeFX();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	float RandomDirectionF();

protected:
	int m_I;
	int m_J;
	bool m_IsFocus = false;
	AChessPiecePType* m_OccupyingPiece = nullptr;
};
