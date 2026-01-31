// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "ImpactSFXMap.h"
#include "ChessPiecePType.generated.h"



UENUM(BlueprintType)
enum class ETeam : uint8
{
	Black, White
};

UENUM(BlueprintType)
enum class EPieceState : uint8
{
	Idle, Moving, Attack, PostAttack, Death, DeathCleanUp
};

UCLASS()
class CHESS_API AChessPiecePType : public APawn
{
	GENERATED_BODY()

public:

	// Sets default values for this pawn's properties
	AChessPiecePType();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ReceiveEPieceTypeAllocation(EPiece ep);
	EPiece GetEPieceType() const { return ePiece; };
	bool GetHasMoved() const { return (movesMade != 0); };
	UFUNCTION(BlueprintCallable)
	int GetMovesMade() const { return movesMade; };
	void IncrementMovesMade();
	void DecrementMovesMade();
	void SetTeam(ETeam t);
	ETeam GetTeam() const {	return team; };
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ReceiveTeamAllocation(ETeam t);
	UFUNCTION(BlueprintCallable)
	int GetSquareI() const { return squareI; };
	UFUNCTION(BlueprintCallable)
	int GetSquareJ() const { return squareJ; };
	int GetPreviousI() const { return previousI; };
	int GetPreviousJ() const { return previousJ; };
	AChessPiecePType* GetAttackTarget() const { return attackTarget; };
	void ReceiveCoordinates(int i, int j);
	void SetIsInFocus(bool isInFocus);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetIsInFocus(bool isInFocus);

	void StartTargetMove(FVector loc);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_MoveToTarget(FVector location);

	void SetMoveTarget(FVector target, int sqI, int sqJ);
	void SetTargetAttack(AChessPiecePType* target, int sqI, int sqJ);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_AttackTarget(AChessPiecePType* target);

	UFUNCTION(BlueprintCallable)
	EPieceState GetPieceState() const {	return pieceState; };
	void SetPieceStateToIdle();
	UFUNCTION(BlueprintCallable)
	void OnReadyToTeleport();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnReadyToTeleport(bool isAttack);
	void Kill();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnDeath();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnCheckMate();

public:


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	float GetPostAttackTime() const { return postAttackTime; };

	UFUNCTION(BlueprintImplementableEvent)
	void EvaluateImpactSFX(AChessPiecePType* target);

protected:
	UPROPERTY(BlueprintReadOnly)
	EPiece ePiece;
	//bool hasMoved = false; // TODO DL replace with counter
	int movesMade = 0;
	// and reduce the counter when undo move etc.
	ETeam team;
	bool inFocus = false;
	int squareI;
	int squareJ;

	int previousI = -1; // -1 means they haven't moved yet
	int previousJ = -1;

	//bool lastMoveWasAttack = false;
	UPROPERTY(EditAnywhere)
	float moveSpeed = 10.f;
	FVector moveTarget{};
	FVector anchorLocation{};
	AChessPiecePType* attackTarget;
	UPROPERTY(EditAnywhere)
	EPieceState pieceState = EPieceState::Idle;
	UPROPERTY(EditAnywhere)
	float rotationSpeed = 5.f;
	FRotator targetRotation;
	UPROPERTY(EditAnywhere)
	float postAttackWaitTime = 1.f;
	float postAttackTime = 0.f;

	UPROPERTY(EditAnywhere)
	float deathCleanUpTime = 1.f;
	float deathTimer = 0.f;
};
