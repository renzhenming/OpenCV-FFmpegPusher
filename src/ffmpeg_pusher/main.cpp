#include <iostream>
using namespace std;
using namespace cv;

extern "C" {
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <opencv2/highgui.hpp>
#pragma comment(lib,"opencv_world320.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib,"avcodec.lib")



int main(int argc, char *argv[])
{
	//海康相机的rtsp url
	char *inUrl = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
	VideoCapture cam;
	namedWindow("video");
	//if (cam.open(0))

	//像素格式转换上下文对象
	SwsContext *vsc = NULL;

	//输出的数据结构
	AVFrame *yuv = NULL;

	//编码器上下文
	AVCodecContext *vc = NULL;
	try
	{
		///opencv打开流
		cam.open(inUrl);
		if (!cam.isOpened())
		{
			throw exception("camera open failed");
		}
		cout << inUrl << " cam open success" << endl;
		//获取frame宽高信息
		int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
		int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
		int fps = cam.get(CAP_PROP_FPS);

		///初始化像素格式转换上下文
		vsc = sws_getCachedContext(vsc,
			//我们使用opencv打开的流，它的格式是AV_PIX_FMT_BGR24，所以这里要注意
			inWidth,inHeight,AV_PIX_FMT_BGR24,
			//目标的宽高和像素格式
			inWidth,inHeight, AV_PIX_FMT_YUV420P,
			//尺寸变化算法
			SWS_BICUBIC,
			0,0,0
			);

		if(!vsc)
		{
			throw exception("sws_getCachedContext failed!");
		}

		///初始化输出的数据结构
		yuv = av_frame_alloc();
		yuv->format = AV_PIX_FMT_YUV420P;
		yuv->width = inWidth;
		yuv->height = inHeight;
		yuv->pts = 0;
		int result = av_frame_get_buffer(yuv, 32);
		if (result!= 0)
		{
			char arr[1024] = { 0 };
			av_strerror(result, arr, sizeof(arr));
			throw exception(arr);
		}

		///初始化编码相关
		AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if(!codec)
		{
			throw exception("find h264 encoder failed");
		}

		vc = avcodec_alloc_context3(codec);
		if(!vc)
		{
			throw exception("avcodec_alloc_context3 failed!");
		}

		vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		vc->codec_id = codec->id;
		vc->thread_count = 8;
		//压缩后每秒视频的bit位大小 50kB
		vc->bit_rate = 50 * 1024 * 8;
		vc->width = inWidth;
		vc->height = inHeight;
		vc->time_base = { 1,fps };
		vc->framerate = { fps,1 };
		//画面组的大小，多少帧一个关键帧
		vc->gop_size = 50;
		//不要b帧
		vc->max_b_frames = 0;
		vc->pix_fmt = AV_PIX_FMT_YUV420P;

		result = avcodec_open2(vc, 0, 0);
		if (result != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			throw exception(buf);
		}

		cout << "avcodec_open2 success!" << endl;

		Mat frame;
		for (;;) {
			///读取rtsp视频帧，并解码
			//cam.read(frame); read内
			//部就是做了grab和retrieve两步
			if (!cam.grab()) {
				continue;
			}
			///yuv转rgb
			if (!cam.retrieve(frame)) {
				continue;
			}
			imshow("video", frame);

			///rgb to yuv
			uint8_t * srcSlice[AV_NUM_DATA_POINTERS] = {0};
			//indata[0] bgrbgrbgr
			//plane indata[0] bbbbb indata[1]ggggg indata[2]rrrrr 
			srcSlice[0] = frame.data;
			int srcStride[AV_NUM_DATA_POINTERS] = {0};
			//一行（宽）数据的字节数
			srcStride[0] = frame.cols*frame.elemSize();
			int h = sws_scale(vsc,
					srcSlice,
					srcStride,
					0,
					frame.rows,
					yuv->data,
					yuv->linesize
					);
			if (h < 0)
			{
				continue;
			}
			waitKey(1);
		}
	}
	catch (const std::exception& ex)
	{
		if (cam.isOpened())
		{
			cam.release();
		}
		if (vsc)
		{
			sws_freeContext(vsc);
			//指针置NULL是一个良好的习惯
			vsc = NULL;
		}
		cerr << ex.what()<< endl;
	}
	
	return 0;
}

//引入头文件
//extern "C" {
//#include "libavformat/avformat.h"
//#include "libavutil/time.h"
//}
//
//#pragma comment(lib,"avformat.lib")
//#pragma comment(lib,"avutil.lib")
//#pragma comment(lib,"avcodec.lib")

//int XError(int errorNum) {
//	char buf[2048] = { 0 };
//	av_strerror(errorNum, buf, sizeof(buf));
//	cout << buf << endl;
//	getchar();
//	return -1;
//}

//static double r2d(AVRational r) {
//	return r.den == 0 || r.num == 0 ? 0. : (double)r.num / (double)r.den;
//}


//int main2(int argc, char *argv[]) {
//	char *input = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
//
//
//	//注意，这个ip地址一定要和你的nginx服务器ip一致，否则avio_open是会阻塞的
//	char *output = "rtmp://192.168.1.108/live";
//	//注册所有的封装解封装器
//	av_register_all();
//
//	//初始化网络
//	avformat_network_init();
//	//**********************************输入流
//	//打开文件
//	AVFormatContext *ictx = NULL;
//
//	//配置参数
//	AVDictionary *opts = NULL;
//	char key[] = "max_delay";
//	char val[] = "500";
//	av_dict_set(&opts, key, val, 0);
//
//	char key2[] = "rtsp_transport";
//	char val2[] = "tcp";
//	av_dict_set(&opts, key2, val2, 0);
//
//	int result = avformat_open_input(&ictx, input, 0, &opts);
//	if (result != 0) {
//		return XError(result);
//	}
//
//	cout << "open file "<<input <<" success"<< endl;
//	result = avformat_find_stream_info(ictx, 0);
//	if (result != 0) {
//		return XError(result);
//	}
//
//	//打印视频文件的格式信息
//	av_dump_format(ictx, 0, input, 0);
//
//	//**********************************输出流
//
//	AVFormatContext *octx = NULL;
//	//创建输出流上下文
//	avformat_alloc_output_context2(&octx, 0, "flv", output);
//	if (!octx)
//	{
//		return XError(result);
//	}
//	//配置输出流
//	//遍历输入的AVStream
//	for (int i = 0; i < ictx->nb_streams; i++){
//		//创建输出流
//		AVStream *out = avformat_new_stream(octx, ictx->streams[i]->codec->codec);
//		if (!out) {
//			return XError(0);
//		}
//		//复制配置信息 mp4
//		//result = avcodec_copy_context(out->codec, ictx->streams[i]->codec);
//		result = avcodec_parameters_copy(out->codecpar, ictx->streams[i]->codecpar);
//		out->codec->codec_tag = 0;
//	}
//
//	av_dump_format(octx, 0, output, 1);
//	
//	//rtmp推流
//	//打开io
//	result = avio_open(&octx->pb, output, AVIO_FLAG_WRITE);
//	cout << "avio_open " << result << endl;
//	if (!octx->pb) {
//		return XError(result);
//	}
//	cout << "avio_open" << endl;
//	//写入头信息
//	result = avformat_write_header(octx, 0);
//	if (result < 0) {
//		return XError(result);
//	}
//
//	cout << "avformat_write_header推流每一帧数据" << endl;
//	//推流每一帧数据
//	AVPacket pkt;
//	long long startTime = av_gettime();
//	for (;;) {
//		result = av_read_frame(ictx, &pkt);
//		if (result != 0 || pkt.size <=0) {
//			continue;
//		}
//		//计算转换时间戳pts dts
//		AVRational itime = ictx->streams[pkt.stream_index]->time_base;
//		AVRational otime = octx->streams[pkt.stream_index]->time_base;
//		pkt.pts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_NEAR_INF));
//		pkt.dts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_NEAR_INF));
//		pkt.duration = av_rescale_q_rnd(pkt.duration, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_NEAR_INF));
//		pkt.pos = -1;
//		
//		//视频帧推送速度
//		//if (ictx->streams[pkt.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//		//	//获取视频的时间戳
//		//	AVRational rational = ictx->streams[pkt.stream_index]->time_base;
//		//	//从开始解码到现在过去的时间,可以作为推流进行的时间，用当前帧的pts做对比，进行同步
//		//	long long now = av_gettime() - startTime;
//		//	long long dts = 0;
//		//	//单位微秒
//		//	dts = pkt.dts*r2d(rational)*1000*1000;
//
//		//	//说明推送太快,等一等
//		//	if (dts > now) {
//		//		av_usleep(dts - now);
//		//		cout << "等待"<< dts - now<< endl;
//		//	}
//		//	else {
//		//		cout << "无需等待" << dts - now << endl;
//		//	}
//		//}
//
//		result = av_interleaved_write_frame(octx, &pkt);
//	
//		if (result < 0) {
//			//return XError(result);
//		}
//
//		//av_packet_unref(&pkt);
//	}
//	cout << "test "<< endl;
//	getchar();
//	return 0;
//}