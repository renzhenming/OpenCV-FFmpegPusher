#pragma once

struct AVCodecContext;
struct AVPacket;

class MediaPusher {
public:

	//工厂方法生产
	static MediaPusher *Get(unsigned char index = 0);

	//初始化封装器上下文
	virtual bool Init(const char *url) = 0;

	//添加流
	virtual bool AddStream(const AVCodecContext *actx) = 0;

	//打开rtmp网络IO，发送封装头
	virtual bool OpenIO() = 0;

	//推流
	virtual bool SendPacket(AVPacket *pkt) = 0;

	//关闭资源
	virtual void Close() = 0;

	virtual ~MediaPusher();
protected:
	MediaPusher();
};