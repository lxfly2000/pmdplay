#pragma once
#include "PMDPlayer.h"

class PMDPlayer_DSound :public PMDPlayer
{
public:
	int Init(int nChannel, int sampleRate, int bytesPerVar, int buffer_time_ms)override;
	int Release()override;
	int Play()override;
	int Pause()override;
	int Stop()override;
	int SetPlaybackSpeed(float)override;
	void SetVolume(int)override;
	int GetVolume()override;
protected:
	void OnPlay()override;
};

