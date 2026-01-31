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
	anchorLocation = GetActorLocation();
}

void AChessPiecePType::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (pieceState == EPieceState::Moving) {
		FRotator rot = GetActorRotation();
		FRotator newRot = KM::RInterpTo(rot, targetRotation, DeltaTime, rotationSpeed);		
		SetActorRotation(newRot);

		if (KM::EqualEqual_RotatorRotator(rot, newRot, 30.f)) {
			SetActorRotation(targetRotation);
			
			// the transition to PostAttack happens from anim notify
		}
		
		float interpSpeed = 1.f;
		FVector step = GetActorLocation() + moveSpeed * KM::GetDirectionUnitVector(GetActorLocation(), moveTarget);
		SetActorLocation(step);
		
		//FVector nextStep = KM::VLerp(GetActorLocation(), moveTarget, DeltaTime);
		//FVector nextStep = KM::VInterpTo(GetActorLocation(), moveTarget, DeltaTime, interpSpeed);
		//SetActorLocation(nextStep);
		float gracePoint = (IsValid(attackTarget)) ? 200.f : 20.f;
		if ((KM::Vector_Distance(GetActorLocation(), moveTarget)) < gracePoint) {
			if (attackTarget) {
				pieceState = EPieceState::Attack;
			}
			else {
				//pieceState = EPieceState::PostAttack;
				SetActorLocation(moveTarget);
				// TODO DL we need to somehow call the OnAttackFinished
				// // which was previous bound in blueprints
				OnReadyToTeleport();
			}
		}
	}
	else if (pieceState == EPieceState::PostAttack) {
		postAttackTime += DeltaTime;
		if (postAttackTime > postAttackWaitTime) {
			postAttackTime = 0.f;
			pieceState = EPieceState::Idle;
			anchorLocation = GetActorLocation();
		}
	}
	else if (pieceState == EPieceState::Death) {
		deathTimer += DeltaTime;
		if (deathTimer > deathCleanUpTime) {
			deathTimer = 0.f;
			pieceState = EPieceState::DeathCleanUp;
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
	movesMade++;
}

void AChessPiecePType::DecrementMovesMade()
{
	ensure(--movesMade >= 0);
}

void AChessPiecePType::SetTeam(ETeam t)
{
	team = t;
	BP_ReceiveTeamAllocation(t); // to blueprints
}

void AChessPiecePType::ReceiveCoordinates(int i, int j)
{
	squareI = i;
	squareJ = j;
}


void AChessPiecePType::SetIsInFocus(bool isInFocus) {
	inFocus = isInFocus;
	BP_SetIsInFocus(isInFocus);
}


void AChessPiecePType::SetMoveTarget(FVector target, int sqI, int sqJ) {
	previousI = squareI;
	previousJ = squareJ;
	pieceState = EPieceState::Moving;
	moveTarget = target;
	attackTarget = nullptr;
	squareI = sqI; // this is updated in gameManager after the move
	squareJ = sqJ;

	StartTargetMove(moveTarget);
}

void AChessPiecePType::StartTargetMove(FVector loc)
{
	pieceState = EPieceState::Moving;
	moveTarget = loc;
	targetRotation = KM::FindLookAtRotation(this->GetActorLocation(), moveTarget);
}

void AChessPiecePType::SetTargetAttack(AChessPiecePType* target, int sqI, int sqJ)
{
	previousI = squareI;
	previousJ = squareJ;
	pieceState = EPieceState::Moving;
	moveTarget = target->GetActorLocation();
	attackTarget = target;
	squareI = sqI; // this is updated in gameManager after the move
	squareJ = sqJ;

	StartTargetMove(moveTarget);
}

void AChessPiecePType::SetPieceStateToIdle()
{
	pieceState = EPieceState::Idle;
}

void AChessPiecePType::OnReadyToTeleport()
{
	// This is called from AnimNotify_FinishAttack
	pieceState = EPieceState::PostAttack;
	bool isAttack = IsValid(attackTarget);
	BP_OnReadyToTeleport(isAttack); // spawn VFX and SFX at location

	SetActorLocation(moveTarget);
	if (attackTarget) {
		EvaluateImpactSFX(attackTarget);
		attackTarget->Kill();
	}
}

void AChessPiecePType::Kill()
{
	pieceState = EPieceState::Death;
	BP_OnDeath(); // to blueprints
}