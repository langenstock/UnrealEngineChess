// Copyright David Lauritsen 2025

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AkGameplayStatics.h"
#include "ImpactSFXMap.generated.h"

UENUM(BlueprintType)
enum class EPiece : uint8
{
	King, Queen, Rook, Knight, Bishop, Pawn
};


class AChessPiecePType;

UCLASS()
class CHESS_API UImpactSFXMap : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TMap <EPiece, TObjectPtr<UAkAudioEvent>> soundMap;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UAkAudioEvent> defaultSFX;
};
