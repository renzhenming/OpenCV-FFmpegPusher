#pragma once
#include "DataThread.h"

enum AUDIOTYPE
{
	AUDIO_QT
};

class AudioRecord :public DataThread
{
public:
	//声道数
	int channels = 2;	
	//样本率
	int sampleRate = 44100;	
	//样本字节大小
	int sampleByte = 2;		
	//一帧音频单个通道的样本数量
	int nbSamples = 1024;	
	//开始录制
	virtual bool Init() = 0;
	//停止录制
	virtual void Stop() = 0;

	static AudioRecord *Get(AUDIOTYPE type = AUDIO_QT, unsigned char index = 0);
	virtual ~AudioRecord();
protected:
	AudioRecord();
};
