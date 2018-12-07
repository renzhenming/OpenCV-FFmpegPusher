#pragma once

struct AVFrame;
struct AVCodecContext;
struct AVPacket;

enum SampleFmt
{
	S16 = 1,
	FLATP = 8
};

class MediaEncode
{
public:
	//视频编码器上下文
	AVCodecContext *vc = 0;
	//音频编码器上下文
	AVCodecContext *ac = 0;

	//输入参数
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3;
	int inChannels = 2;
	SampleFmt inSampleFmt = S16;
	int inSampleRate = 44100;


	//输出参数
	int outWidth = 1280;
	int outHeight = 720;
	int fps = 25;
	int bitrate = 4000000;
	int nbSample = 1024;
	SampleFmt outSmapleFmt = FLATP;

	//工厂生产方法
	static MediaEncode *Get(unsigned char index = 0);

	virtual AVFrame* RgbToYuv(char *rgb) = 0;

	//关闭资源
	virtual void Close() = 0;

	//初始化像素格式转换的上下文
	virtual bool InitScale() = 0;

	//音频重采样上下文初始化
	virtual bool InitResample() = 0;

	//重采样 
	virtual AVFrame *Resample(char *data) = 0;

	//初始化视频编码器
	virtual bool InitVideoCodec() = 0;

	//音频编码初始化
	virtual bool InitAudioCodec() = 0;

	//视频编码
	virtual AVPacket *EncodeVideo(AVFrame *frame) = 0;

	//音频编码 
	virtual AVPacket * EncodeAudio(AVFrame* frame) = 0;

	virtual ~MediaEncode();
protected:
	MediaEncode();
};