// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioManagerSubsystem.h"

UAudioManagerSubsystem* UAudioManagerSubsystem::GetAudioManager(const UObject* worldContext) {
	if (!worldContext) { return nullptr; };
	UGameInstance* gi = UGameplayStatics::GetGameInstance(worldContext);

	if (!gi) { return nullptr; };

	return gi->GetSubsystem<UAudioManagerSubsystem>();
}

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAudioManagerSubsystem::Deinitialize()
{
}

int UAudioManagerSubsystem::GetAnInt()
{
	return 7;
}
