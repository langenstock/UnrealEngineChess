// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AudioListener.generated.h"

UCLASS()
class CHESS_API AAudioListener : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAudioListener();
	virtual void Tick(float DeltaTime) override;
	//static AAudioListener* Get(UWorld* world);
	//void InitInstance();
	//static AAudioListener* instance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//UPROPERTY(EditAnywhere)
	//TSubclassOf<AAudioListener> audioListenerClass;


};
