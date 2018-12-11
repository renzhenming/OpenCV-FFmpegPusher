#pragma once

#include "DataThread.h"

class MediaCapture :public DataThread
{
public:
	int width = 0;
	int height = 0;
	int fps = 0;

	static MediaCapture *Get(unsigned char index = 0);

	//初始化相机
	virtual bool Init(int cameraIndex = 0) = 0;

	//初始化网络流
	virtual bool Init(const char *url) = 0;

	//停止
	virtual void Stop() = 0;

	virtual ~MediaCapture();
protected:
	MediaCapture();
};