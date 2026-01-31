// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AudioManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CHESS_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta = (worldContext = "worldContext"))
	static UAudioManagerSubsystem* GetAudioManager(const UObject* worldContext);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION(BlueprintCallable)
	int GetAnInt();

protected:
private:

};
