#pragma once
#include "PMDPlayer.h"

#define KEYNAME_NO_XAUDIO2	"NoXAudio2"

class PMDPlayer_XAudio2 :public PMDPlayer
{
public:
	int Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)override;
	int Release()override;
	int SetPlaybackSpeed(float)override;
	void SetVolume(int)override;
	int GetVolume()override;
protected:
	void OnPlay()override;
};

