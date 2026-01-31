// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiecePType.h"
#include "ChessPiece.generated.h"



UCLASS()
class CHESS_API AChessPiece : public AChessPiecePType
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChessPiece();
	virtual void Tick(float DeltaTime) override;
	/*
	EPiece GetEPieceType() const;
	bool GetHasMoved() const;
	ETeam GetTeam() const;
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveTeamAllocation(ETeam t);
	UFUNCTION(BlueprintCallable)
	int GetSquareI() const;
	UFUNCTION(BlueprintCallable)
	int GetSquareJ() const;
	void ReceiveCoordinates(int i, int j);
	void SetIsInFocus(bool isInFocus);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetIsInFocus(bool isInFocus);

	void SetTargetMove(FVector loc);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_MoveToTarget(FVector location);
	UFUNCTION(BlueprintCallable)
	EPieceState GetPieceState() const;
	*/
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//UFUNCTION(BlueprintCallable)
	//void OnClick(int button); // TODO DL delete this
	/*
protected:
	EPiece ePiece;
	bool hasMoved = false;
	ETeam team;
	bool inFocus = false;
	//bool focusStatusDirty = false;
	int squareI;
	int squareJ;
	float moveSpeed = 10.f;
	FVector moveTarget{};
	UPROPERTY(EditAnywhere)
	EPieceState pieceState = EPieceState::Idle;
	*/
};
