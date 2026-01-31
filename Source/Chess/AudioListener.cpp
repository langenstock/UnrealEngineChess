// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioListener.h"

// Sets default values
AAudioListener::AAudioListener()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAudioListener::BeginPlay()
{
	Super::BeginPlay();
	
}
/*

void AAudioListener::InitInstance() {
	UWorld* w = GetWorld();
	if (!w) { return; };

	FTransform t = FTransform{};
	FActorSpawnParameters p = FActorSpawnParameters{};
	if (ensure(audioListenerClass)) {
		instance = w->SpawnActor<AAudioListener>(audioListenerClass, t, p);
	}
}

AAudioListener* AAudioListener::Get(UWorld* world) {
	if (!world) { return nullptr; };

	return (instance) ? instance : nullptr;
}
*/
// Called every frame
void AAudioListener::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

