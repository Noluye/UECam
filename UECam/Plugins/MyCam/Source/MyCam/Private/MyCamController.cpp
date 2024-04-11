// Fill out your copyright notice in the Description page of Project Settings.
#include "MyCamController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/MediaBlueprintFunctionLibrary.h"
#include "MediaPlayer.h"


// Sets default values
AMyCamController::AMyCamController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	UpdateCameraDevices();
}

void AMyCamController::UpdateCameraDevices()
{
	UMediaBlueprintFunctionLibrary::EnumerateVideoCaptureDevices(CameraDevices, -1);
	for (FMediaCaptureDevice& CameraDevice : CameraDevices)
	{
		NameOptions.Add(CameraDevice.DisplayName.ToString());
	}
	if (NameOptions.Num() == 0) UE_LOG(LogTemp, Error, TEXT("No camera detected!"));
}

#if WITH_EDITOR
void AMyCamController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if property is valid
	if (PropertyChangedEvent.Property == nullptr) return;

	// Get the name of the changed property
	const FName PropertyName(PropertyChangedEvent.Property->GetFName());
	// If the changed property is CamName
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AMyCamController, CamName)) {
		UE_LOG(LogTemp, Warning, TEXT("Ready to open %s..."), *CamName);
		OpenCamera(false);  // Just to update the configurations 
	}
}
#endif

// Called when the game starts or when spawned
void AMyCamController::BeginPlay()
{
	Super::BeginPlay();
	OpenCamera(true);
}

void AMyCamController::OpenCamera(bool bRunCamera)
{
	if (MediaPlayer == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("You must set MediaPlayer first!"));
		UE_LOG(LogTemp, Error, TEXT("You must set MediaPlayer first!"));
		return;
	}
	for (FMediaCaptureDevice& CameraDevice : CameraDevices)
	{
		if (CamName != CameraDevice.DisplayName.ToString()) continue;  // Choose the camera based on the name

		if (!MediaPlayer->OpenUrl(CameraDevice.Url))
		{
			UE_LOG(LogTemp, Error, TEXT("Can't open %s's URL"), *CamName);
			return;
		}

		if (bRunCamera) GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyCamController::SelectCamFormat, 0.5, false); 
		else GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyCamController::UpdateCamDetails, 0.5, false);
		
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("No camera detected or you didn't set your camera in CameraController!"));
}

void AMyCamController::UpdateCamDetails()
{
	int32 NumTracks = MediaPlayer->GetNumTracks(EMediaPlayerTrack::Video);
	TrackID = NumTracks - 1;  // Currently we fix the trackID.

	int32 FormatNum = MediaPlayer->GetNumTrackFormats(EMediaPlayerTrack::Video, TrackID);
	if (FormatNum == 0) UE_LOG(LogTemp, Error, TEXT("FormatNum == 0"));

	for (int32 i = 0; i < FormatNum; ++i)
	{
		FIntPoint Resolution = MediaPlayer->GetVideoTrackDimensions(TrackID, i);
		ResolutionOptions.AddUnique(Resolution);
		float FrameRate = MediaPlayer->GetVideoTrackFrameRate(TrackID, i);
		FrameRateOptions.AddUnique(FrameRate);
	}
}

void AMyCamController::SelectCamFormat()
{
	MediaPlayer->SelectTrack(EMediaPlayerTrack::Video, TrackID);
	SelectedFormat = FindFormatIndex();
	if (SelectedFormat == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("SelectedFormat == -1"));
		return;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SelectedFormat = %d"), SelectedFormat);
	}
	bool Status = MediaPlayer->SetTrackFormat(EMediaPlayerTrack::Video, TrackID, SelectedFormat);

	if (!Status) UE_LOG(LogTemp, Error, TEXT("MediaPlayer->SetTrackFormat failed!"));

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMyCamController::Play, 0.5, false);
}

void AMyCamController::Play()
{
	bool Status = MediaPlayer->Play();
	if (!Status) UE_LOG(LogTemp, Error, TEXT("MediaPlayer->Play() failed!"));
}

int32 AMyCamController::FindFormatIndex()
{
	int32 FormatsNum = MediaPlayer->GetNumTrackFormats(EMediaPlayerTrack::Video, TrackID);
	if (FormatsNum == 0) return -1;
	for (int i = 0; i < FormatsNum; ++i)
	{
		FIntPoint CurrResolution = MediaPlayer->GetVideoTrackDimensions(TrackID, i);
		bool MatchResolution = CurrResolution == CamResolution;
		float CurrFrameRate = MediaPlayer->GetVideoTrackFrameRate(TrackID, i);
		bool MatchFrameRate = FMath::IsNearlyEqual(CamFrameRate, CurrFrameRate, 0.1f);
		if (MatchResolution && MatchFrameRate) return i;
	}
	UE_LOG(LogTemp, Error, TEXT("Cann't match for (%d, %d) and %f"), CamResolution.X, CamResolution.Y, CamFrameRate);
	return 0;  // Still trying to return a valid format.
}
