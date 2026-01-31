// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_FinishAttack.h"

void UAnimNotify_FinishAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	int a = 1;
}

void UAnimNotify_FinishAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* a = MeshComp->GetOwner();
	AChessPiecePType* piece = Cast<AChessPiecePType>(a);
	if (!piece) { return; };

	piece->OnReadyToTeleport();
}
