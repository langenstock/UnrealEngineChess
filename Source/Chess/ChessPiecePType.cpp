// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessPiecePType.h"

using KM = UKismetMathLibrary;

AChessPiecePType::AChessPiecePType()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AChessPiecePType::BeginPlay()
{
	Super::BeginPlay();
	m_AnchorLocation = GetActorLocation();
}

void AChessPiecePType::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_PieceState == EPieceState::Moving) {
		FRotator rot = GetActorRotation();
		FRotator newRot = KM::RInterpTo(rot, m_TargetRotation, DeltaTime, rotationSpeed);		
		SetActorRotation(newRot);

		if (KM::EqualEqual_RotatorRotator(rot, newRot, 30.f)) {
			SetActorRotation(m_TargetRotation);
			// the transition to PostAttack happens from anim notify
		}
		
		float interpSpeed = 1.f;
		FVector step = GetActorLocation() + moveSpeed * KM::GetDirectionUnitVector(GetActorLocation(), m_MoveTarget);
		SetActorLocation(step);
		
		float gracePoint = (IsValid(m_AttackTarget)) ? 200.f : 20.f;
		if ((KM::Vector_Distance(GetActorLocation(), m_MoveTarget)) < gracePoint) {
			if (m_AttackTarget) {
				m_PieceState = EPieceState::Attack;
			}
			else {
				SetActorLocation(m_MoveTarget);
				OnReadyToTeleport();
			}
		}
	}
	else if (m_PieceState == EPieceState::PostAttack) {
		m_PostAttackTime += DeltaTime;
		if (m_PostAttackTime > postAttackWaitTime) {
			m_PostAttackTime = 0.f;
			m_PieceState = EPieceState::Idle;
			m_AnchorLocation = GetActorLocation();
		}
	}
	else if (m_PieceState == EPieceState::Death) {
		m_DeathTimer += DeltaTime;
		if (m_DeathTimer > deathCleanUpTime) {
			m_DeathTimer = 0.f;
			m_PieceState = EPieceState::DeathCleanUp;
		}
	}
}

// Called to bind functionality to input
void AChessPiecePType::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AChessPiecePType::ReceiveEPieceTypeAllocation(EPiece ep)
{
	ePiece = ep;
}

void AChessPiecePType::IncrementMovesMade()
{
	m_MovesMade++;
}

void AChessPiecePType::DecrementMovesMade()
{
	ensure(--m_MovesMade >= 0);
}

void AChessPiecePType::SetTeam(ETeam t)
{
	m_Team = t;
	BP_ReceiveTeamAllocation(t); // to blueprints
}

void AChessPiecePType::ReceiveCoordinates(int i, int j)
{
	m_SquareI = i;
	m_SquareJ = j;
}

void AChessPiecePType::SetIsInFocus(bool isInFocus) {
	m_InFocus = isInFocus;
	BP_SetIsInFocus(isInFocus);
}

void AChessPiecePType::SetMoveTarget(FVector target, int sqI, int sqJ) {
	m_PreviousI = m_SquareI;
	m_PreviousJ = m_SquareJ;
	m_PieceState = EPieceState::Moving;
	m_MoveTarget = target;
	m_AttackTarget = nullptr;
	m_SquareI = sqI; // this is updated in gameManager after the move
	m_SquareJ = sqJ;

	StartTargetMove(m_MoveTarget);
}

void AChessPiecePType::StartTargetMove(FVector loc)
{
	m_PieceState = EPieceState::Moving;
	m_MoveTarget = loc;
	m_TargetRotation = KM::FindLookAtRotation(this->GetActorLocation(), m_MoveTarget);
}

void AChessPiecePType::SetTargetAttack(AChessPiecePType* target, int sqI, int sqJ)
{
	m_PreviousI = m_SquareI;
	m_PreviousJ = m_SquareJ;
	m_PieceState = EPieceState::Moving;
	m_MoveTarget = target->GetActorLocation();
	m_AttackTarget = target;
	m_SquareI = sqI; // this is updated in gameManager after the move
	m_SquareJ = sqJ;

	StartTargetMove(m_MoveTarget);
}

void AChessPiecePType::SetPieceStateToIdle()
{
	m_PieceState = EPieceState::Idle;
}

void AChessPiecePType::OnReadyToTeleport()
{
	// This is called from AnimNotify_FinishAttack
	m_PieceState = EPieceState::PostAttack;
	bool isAttack = IsValid(m_AttackTarget);
	BP_OnReadyToTeleport(isAttack);

	SetActorLocation(m_MoveTarget);
	if (m_AttackTarget) {
		EvaluateImpactSFX(m_AttackTarget);
		m_AttackTarget->Kill();
	}
}

void AChessPiecePType::Kill()
{
	m_PieceState = EPieceState::Death;
	BP_OnDeath(); // to blueprints
}