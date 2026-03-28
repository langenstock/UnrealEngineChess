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
	AChessPiecePType();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ReceiveEPieceTypeAllocation(EPiece ep);
	EPiece GetEPieceType() const { return ePiece; };
	bool GetHasMoved() const { return (m_MovesMade != 0); };
	UFUNCTION(BlueprintCallable)
	int GetMovesMade() const { return m_MovesMade; };
	void IncrementMovesMade();
	void DecrementMovesMade();
	void SetTeam(ETeam t);
	ETeam GetTeam() const {	return m_Team; };
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ReceiveTeamAllocation(ETeam t);
	UFUNCTION(BlueprintCallable)
	int GetSquareI() const { return m_SquareI; };
	UFUNCTION(BlueprintCallable)
	int GetSquareJ() const { return m_SquareJ; };
	int GetPreviousI() const { return m_PreviousI; };
	int GetPreviousJ() const { return m_PreviousJ; };
	AChessPiecePType* GetAttackTarget() const { return m_AttackTarget; };
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
	EPieceState GetPieceState() const {	return m_PieceState; };
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
	float GetPostAttackTime() const { return m_PostAttackTime; };

	UFUNCTION(BlueprintImplementableEvent)
	void EvaluateImpactSFX(AChessPiecePType* target);

protected:
	UPROPERTY(BlueprintReadOnly)
	EPiece ePiece;


	UPROPERTY(EditAnywhere)
	float moveSpeed = 10.f;

	UPROPERTY(EditAnywhere)
	float rotationSpeed = 5.f;
	
	UPROPERTY(EditAnywhere)
	float postAttackWaitTime = 1.f;


	UPROPERTY(EditAnywhere)
	float deathCleanUpTime = 1.f;


private:
	int m_MovesMade = 0;
	ETeam m_Team;
	bool m_InFocus = false;
	int m_SquareI;
	int m_SquareJ;

	int m_PreviousI = -1; // -1 means they haven't moved yet
	int m_PreviousJ = -1;

	FVector m_MoveTarget{};
	FVector m_AnchorLocation{};
	AChessPiecePType* m_AttackTarget;

	EPieceState m_PieceState = EPieceState::Idle;

	FRotator m_TargetRotation;

	float m_PostAttackTime = 0.f;

	float m_DeathTimer = 0.f;
};
