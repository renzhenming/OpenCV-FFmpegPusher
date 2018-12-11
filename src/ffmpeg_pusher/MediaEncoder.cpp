#include "MediaEncoder.h"
#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swresample.lib")

using namespace std;

class CMediaEncode : public MediaEncoder {
private:
	//像素格式转换上下文
	SwsContext *vsc = NULL;
	//音频重采样上下文
	SwrContext *asc = NULL;
	//视频frame
	AVFrame *yuv = NULL;
	//音频frame
	AVFrame *pcm = NULL;
	//存储视频压缩数据
	AVPacket vpacket = { 0 };
	//存储音频压缩数据
	AVPacket apacket = { 0 };
	int vpts = 0;
	int apts = 0;
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
		if (pcm)
		{
			av_frame_free(&pcm);
		}
		if (vc)
		{
			avcodec_free_context(&vc);
		}
		vpts = 0;
		av_packet_unref(&vpacket);
		apts = 0;
		av_packet_unref(&apacket);
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

	
	bool InitResample() {
		///初始化音频重采样上下文
		asc = swr_alloc_set_opts(
			asc,
			//输出
			av_get_default_channel_layout(inChannels),
			(AVSampleFormat)outSmapleFmt,
			inSampleRate,
			//输入
			av_get_default_channel_layout(inChannels),
			(AVSampleFormat)inSampleFmt,
			inSampleRate,
			0,0
		);

		if (!asc)
		{
			cout << "swr_alloc_set_opts failed!";
			return false;
		}
		int result = swr_init(asc);
		if (result != 0)
		{
			char err[1024] = { 0 };
			av_strerror(result, err, sizeof(err) - 1);
			cout <<err << endl;
			return false;
		}
		cout << "音频重采样 上下文初始化成功!" << endl;

		///重采样输出空间分配
		pcm = av_frame_alloc();
		pcm->format = outSmapleFmt;
		pcm->channels = inChannels;
		pcm->channel_layout = av_get_default_channel_layout(inChannels);
		//一帧音频单通道的采用数量
		pcm->nb_samples = nbSample;
		//真正分配存储空间的操作是这个
		result = av_frame_get_buffer(pcm, 0);
		if (result != 0)
		{
			char err[1024] = { 0 };
			av_strerror(result, err, sizeof(err) - 1);
			cout << err << endl;
			return false;
		}
		return true;
	}

	AVFrame *Resample(char *data) {
		const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
		indata[0] = (uint8_t*)data;
		int len = swr_convert(
			asc,
			pcm->data,
			pcm->nb_samples,
			indata,
			pcm->nb_samples
		);
		if (len <= 0 )
		{
			return NULL;
		}
		return pcm;
	}

	bool InitAudioCodec() {
		if (!CreateCodec(AV_CODEC_ID_AAC)) {
			return false;
		}
		ac->bit_rate = 40000;
		ac->sample_rate = inSampleRate;
		ac->sample_fmt = AV_SAMPLE_FMT_FLTP;
		ac->channels = inChannels;
		ac->channel_layout = av_get_default_channel_layout(inChannels);
		return OpenCodec(&ac);
	}

	AVPacket * EncodeAudio(AVFrame* frame) {
		pcm->pts = apts;
		// 1/inSampleRate表示一个样本表示多少秒 
		//nb_samples表示这个frame有多少个样本
		//av_rescale_q函数的意义表示第一个参数乘以第二个再除以第三个
		//前两个参数相乘得到的是nb_samples个样本占用多少秒
		//然后乘以时间基数，得到的就是真正的时间
		apts += av_rescale_q(pcm->nb_samples, { 1,inSampleRate }, ac->time_base);
		int result = avcodec_send_frame(ac, frame);
		if (result != 0)
		{
			return NULL;
		}

		//将packet重置，去除引用，清空数据
		av_packet_unref(&apacket);
		result = avcodec_receive_packet(ac, &apacket);
		if (result != 0)
		{
			return NULL;
		}
		return &apacket;
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
		frame->pts = vpts;
		vpts++;
		int result = avcodec_send_frame(vc,frame);
		if (result != 0) {
			return NULL;
		}
		//将packet重置，去除引用，清空数据
		av_packet_unref(&vpacket);
		result = avcodec_receive_packet(vc, &vpacket);
		if (result != 0 || vpacket.size <= 0)
		{
			return NULL;
		}
		return &vpacket;
	}

	bool CreateCodec(AVCodecID codecId) {
		//找到编码器
		AVCodec *codec = avcodec_find_encoder(codecId);
		if (!codec)
		{
			cout << "avcodec_find_encoder  failed!" << endl;
			return false;
		}
		//获取编码器上下文
		ac = avcodec_alloc_context3(codec);
		if (!ac)
		{
			cout << "avcodec_alloc_context3  failed!" << endl;
			return false;
		}
		cout << "avcodec_alloc_context3 success!" << endl;
		ac->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		ac->thread_count = 4;
		return true;
	}

	bool OpenCodec(AVCodecContext **context)
	{
		//打开编码器
		int result = avcodec_open2(*context, 0, 0);
		if (result != 0)
		{
			char err[1024] = { 0 };
			av_strerror(result, err, sizeof(err) - 1);
			cout << err << endl;
			avcodec_free_context(context);
			return false;
		}
		cout << "avcodec_open2 success!" << endl;
		return true;
	}
};

MediaEncoder* MediaEncoder :: Get(unsigned char index) {
	static bool isFirst = true;
	if (isFirst)
	{
		avcodec_register_all();
	}
	static CMediaEncode encode[255];
	return &encode[index];
}

MediaEncoder::MediaEncoder() {

}

MediaEncoder :: ~MediaEncoder() {

}