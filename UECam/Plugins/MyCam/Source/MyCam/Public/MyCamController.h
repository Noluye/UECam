// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyCamController.generated.h"


UCLASS()
class MYCAM_API AMyCamController : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera");
	class UMediaPlayer* MediaPlayer;  // Created in the editor
	UPROPERTY(EditAnywhere, Category = "Camera Configuration", meta = (GetOptions = "GetNameOptions"))
	FString CamName;
	UPROPERTY(EditAnywhere, Category = "Camera Configuration")
	FIntPoint CamResolution = FIntPoint(0, 0);
	UPROPERTY(EditAnywhere, Category = "Camera Configuration")
	float CamFrameRate;
	UPROPERTY(VisibleAnywhere, Category = "Camera Reference")
	TArray<FIntPoint> ResolutionOptions;
	UPROPERTY(VisibleAnywhere, Category = "Camera Reference")
	TArray<float> FrameRateOptions;
	UPROPERTY(VisibleAnywhere, Category = "Camera Reference")
	int32 TrackID;
	UPROPERTY(VisibleAnywhere, Category = "Camera Reference")
	int32 SelectedFormat = -1;

	
public:	
	// Sets default values for this actor's properties
	AMyCamController();
	UFUNCTION(BlueprintCallable)
	void UpdateCameraDevices();
	UFUNCTION(BlueprintCallable)
	void Play();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
private:
	UFUNCTION()
	FORCEINLINE TArray<FString> GetNameOptions() const { return NameOptions; };
	void OpenCamera(bool bRunCamera);
	UFUNCTION()
	void UpdateCamDetails();
	int32 FindFormatIndex();
	void SelectCamFormat();
private:
	FTimerHandle TimerHandle;
	TArray<struct FMediaCaptureDevice> CameraDevices;
	TArray<FString> NameOptions;
};
