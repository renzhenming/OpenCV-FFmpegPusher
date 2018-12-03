#include "MediaEncode.h"
#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avcodec.lib")
using namespace std;

class CMediaEncode : public MediaEncode {
private:
	SwsContext *vsc = NULL;
	AVFrame *yuv = NULL;
	AVPacket packet = {0};
	int vpts = 0;
public:
	void Close() {
		if (vsc)
		{
			sws_freeContext(vsc);
			vsc = NULL;
		}
		if (yuv)
		{
			av_frame_free(&yuv);
		}
		if (vc)
		{
			avcodec_free_context(&vc);
		}
		vpts = 0;
		av_packet_unref(&packet);
	}

	bool InitScale() {
		//初始化像素格式转换上下文
		vsc = sws_getCachedContext(vsc,
			inWidth,inHeight,AV_PIX_FMT_BGR24,
			outWidth,outHeight,AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,0,0,0
			);
		if(!vsc)
		{
			cout << "sws_getCachedContext failed" ;
			return false;
		}

		//初始化输出的AVFrame
		yuv = av_frame_alloc();
		yuv->format = AV_PIX_FMT_YUV420P;
		yuv->width = inWidth;
		yuv->height = inHeight;
		yuv->pts = 0;
		//分配yuv空间
		int ret = av_frame_get_buffer(yuv, 32);
		if (ret != 0)
		{
			cout << "av_frame_get_buffer failed";
			return false;
		}
		return true;
	}

	AVFrame* RgbToYuv(char *rgb) {
		uint8_t * srcSlice[AV_NUM_DATA_POINTERS] = { 0 };
		srcSlice[0] = (uint8_t*)rgb;
		int srcStride[AV_NUM_DATA_POINTERS] = { 0 };
		//一行（宽）数据的字节数
		srcStride[0] = inWidth*inPixSize;
		int h = sws_scale(vsc,
			srcSlice,
			srcStride,
			0,
			inHeight,
			yuv->data,
			yuv->linesize
		);
		if (h <= 0)
		{
			return NULL;
		}
		return yuv;
	}

	bool InitVideoCodec() {
		AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec)
		{
			cout << "Can`t find h264 encoder!" << endl;;
			return false;
		}

		vc = avcodec_alloc_context3(codec);
		if (!vc)
		{
			cout << "avcodec_alloc_context3 failed!" << endl;
			return false;
		}

		vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		vc->codec_id = codec->id;
		vc->thread_count = 8;
		//压缩后每秒视频的bit位大小 50kB
		vc->bit_rate = 50 * 1024 * 8;
		vc->width = outWidth;
		vc->height = outHeight;
		vc->time_base = { 1,fps };
		vc->framerate = { fps,1 };
		//画面组的大小，多少帧一个关键帧
		vc->gop_size = 50;
		//不要b帧
		vc->max_b_frames = 0;
		vc->pix_fmt = AV_PIX_FMT_YUV420P;

		int result = avcodec_open2(vc, 0, 0);
		if (result != 0)
		{
			cout << "avcodec_open2 failed!" << endl;
			return false;
		}

		cout << "avcodec_open2 success!" << endl;
		return true;
	}

	AVPacket* EncodeVideo(AVFrame *frame) {
		av_packet_unref(&packet);
		frame->pts = vpts;
		vpts++;
		int result = avcodec_send_frame(vc,frame);
		if (result != 0) {
			return NULL;
		}
		result = avcodec_receive_packet(vc, &packet);
		if (result != 0 || packet.size <= 0)
		{
			return NULL;
		}
		return &packet;
	}
};

MediaEncode* MediaEncode :: Get(unsigned char index) {
	static bool isFirst = true;
	if (isFirst)
	{
		avcodec_register_all();
	}
	static CMediaEncode encode[255];
	return &encode[index];
}

MediaEncode::MediaEncode() {

}

MediaEncode :: ~MediaEncode() {

}