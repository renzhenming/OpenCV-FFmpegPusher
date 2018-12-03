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