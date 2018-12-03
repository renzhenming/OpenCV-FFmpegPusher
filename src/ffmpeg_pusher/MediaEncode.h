#pragma once

struct AVFrame;
struct AVCodecContext;
struct AVPacket;

class MediaEncode
{
public:
	//编码器上下文
	AVCodecContext *vc = 0;	

	//输入参数
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3;

	//输出参数
	int outWidth = 1280;
	int outHeight = 720;
	int fps = 25;
	//压缩后每秒视频的bit位大小 50kB
	int bitrate = 4000000;

	//工厂生产方法
	static MediaEncode *Get(unsigned char index = 0);

	virtual AVFrame* RgbToYuv(char *rgb) = 0;

	//关闭资源
	virtual void Close() = 0;

	//初始化像素格式转换的上下文
	virtual bool InitScale() = 0;

	//初始化视频编码器
	virtual bool InitVideoCodec() = 0;

	//视频编码
	virtual AVPacket *EncodeVideo(AVFrame *frame) = 0;

	virtual ~MediaEncode();
protected:
	MediaEncode();
};