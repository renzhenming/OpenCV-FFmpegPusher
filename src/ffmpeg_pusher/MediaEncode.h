#pragma once

struct AVFrame;

class MediaEncode
{
public:
	//输入参数
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3;

	//输出参数
	int outWidth = 1280;
	int outHeight = 720;

	//工厂生产方法
	static MediaEncode *Get(unsigned char index = 0);

	virtual AVFrame* RgbToYuv(char *rgb) = 0;

	//初始化像素格式转换的上下文
	virtual bool InitScale() = 0;

	//关闭上下文
	virtual void Close() = 0;

	virtual ~MediaEncode();
protected:
	MediaEncode();
};