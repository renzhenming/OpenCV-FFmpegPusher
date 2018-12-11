#include <iostream>
using namespace std;

#include "MediaEncoder.h"
#include "MediaPusher.h"
#include "MediaCapture.h"

int main(int argc, char *argv[])
{

	//rtsp url，这是一个可用的测试流地址
	char *inUrl = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
	//nginx-rtmp 直播服务器rtmp推流URL(192.168.1.106是你服务器的ip,确保服务器开启)
	char *outUrl = "rtmp://192.168.43.24/live";

	MediaCapture *capture = MediaCapture::Get();
	if (!capture->Init(inUrl))
	{
		cout << "open camera failed!" << endl;
		getchar();
		return -1;
	}
	cout << "open camera success!" << endl;
	capture->Start();

	MediaEncoder *encode = MediaEncoder::Get();
	MediaPusher *pusher = MediaPusher::Get();

	try
	{
		//声道数
		encode->inChannels = 2;
		//输入的样本格式
		encode->inSampleFmt = SampleFmt::S16;
		//输入的采样率
		encode->inSampleRate = 44100;
		//单通道每帧大小
		encode->nbSample = 1024;
		//输出的样本格式
		encode->outSmapleFmt = SampleFmt::FLATP;


		///视频属性
		encode->inWidth = capture->width;
		encode->inHeight = capture->height;
		encode->outWidth = capture->width;
		encode->outHeight = capture->height;

		///初始化像素格式转换上下文
		if (!encode->InitScale()){
			getchar();
			return -1;
		}
		cout << inUrl << "初始化像素格式转换 success" << endl;
		///初始化音频重采样上下文
		if (!encode->InitResample())
		{
			getchar();
			return -1;
		}
		cout << inUrl << "初始化音频重采样 success" << endl;
		///初始化视频编码器
		if (!encode->InitVideoCodec())
		{
			getchar();
			return -1;
		}
		cout << inUrl << "初始化视频编码器 success" << endl;
		///初始化音频编码器
		if (!encode->InitAudioCodec())
		{
			getchar();
			return -1;
		}
		cout << inUrl << "初始化音频编码器 success" << endl;
		///封装器设置
		if (!pusher->Init(outUrl)){
			getchar();
			return -1;
		}
		cout << inUrl << "封装器设置 success" << endl;
		//添加视频流
		pusher->AddStream(encode->vc);
		cout << inUrl << "添加视频流 success" << endl;
		//添加音频流
		pusher->AddStream(encode->ac);
		cout << inUrl << "添加音频流 success" << endl;
		///打开网络IO流通道
		pusher->OpenIO();
		cout << inUrl << "打开网络IO流通道 success" << endl;
		for (;;) {
			Data videoData = capture->Pop();
			if (videoData.size <= 0)
			{
				Sleep(1);
				continue;
			}

			if (videoData.size > 0)
			{
				///rgb to yuv
				AVFrame *yuv = encode->RgbToYuv((char *)videoData.data);
				if (!yuv) continue;
				cout << inUrl << "RgbToYuv success" << endl;
				///h264编码
				AVPacket *packet = encode->EncodeVideo(yuv);
				if (!packet)continue;
				cout << inUrl << "EncodeVideo success" << endl;
				if (pusher->SendPacket(packet))
				{
					cout << packet << flush;
				}
			}
		}
	}
	catch (const std::exception& ex)
	{
		if (encode)
		{
			encode->Close();
		}
		if (pusher)
		{
			pusher->Close();
		}
		cerr << ex.what() << endl;
	}

	return 0;
}

//int main2(int argc, char *argv[])
//{
//	//rtsp url，这是一个可用的测试流地址
//	char *inUrl = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
//	//nginx-rtmp 直播服务器rtmp推流URL(192.168.1.106是你服务器的ip,确保服务器开启)
//	char *outUrl = "rtmp://192.168.42.134/live";
//	//注册所有的封装器
//	av_register_all();
//	//注册所有的编解码器
//	avcodec_register_all();
//	//注册所有网络协议
//	avformat_network_init();
//
//	VideoCapture cam;
//
//	namedWindow("video");
//	//if (cam.open(0))
//	Mat frame;
//
//	//像素格式转换上下文对象
//	SwsContext *vsc = NULL;
//
//	//输出的数据结构
//	AVFrame *yuv = NULL;
//
//	//编码器上下文
//	AVCodecContext *vc = NULL;
//
//	//rtmp flv 封装器
//	AVFormatContext *ic = NULL;
//	try
//	{
//		///opencv打开流
//		cam.open(inUrl);
//		if (!cam.isOpened())
//		{
//			throw exception("camera open failed");
//		}
//		cout << inUrl << " cam open success" << endl;
//		//获取流宽高信息
//		int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
//		int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
//		int fps = cam.get(CAP_PROP_FPS);
//
//		///初始化像素格式转换上下文
//		vsc = sws_getCachedContext(vsc,
//			//我们使用opencv打开的流，它的格式是AV_PIX_FMT_BGR24，所以这里要注意
//			inWidth,inHeight,AV_PIX_FMT_BGR24,
//			//目标的宽高和像素格式
//			inWidth,inHeight, AV_PIX_FMT_YUV420P,
//			//尺寸变化算法
//			SWS_BICUBIC,
//			0,0,0
//			);
//
//		if(!vsc)
//		{
//			throw exception("sws_getCachedContext failed!");
//		}
//
//		///初始化输出的数据结构
//		yuv = av_frame_alloc();
//		yuv->format = AV_PIX_FMT_YUV420P;
//		yuv->width = inWidth;
//		yuv->height = inHeight;
//		yuv->pts = 0;
//
//		//这个方法调用之前要确保这些条件已经设置
//		//  * -format (pixel format for video, sample format for audio)
//		//  * -width and height for video
//		//	* -nb_samples and channel_layout for audio
//		int result = av_frame_get_buffer(yuv, 32);
//		if (result!= 0)
//		{
//			char arr[1024] = { 0 };
//			av_strerror(result, arr, sizeof(arr));
//			throw exception(arr);
//		}
//
//		///初始化编码相关
//		AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
//		if(!codec)
//		{
//			throw exception("find h264 encoder failed");
//		}
//
//		vc = avcodec_alloc_context3(codec);
//		if(!vc)
//		{
//			throw exception("avcodec_alloc_context3 failed!");
//		}
//
//		vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//		vc->codec_id = codec->id;
//		vc->thread_count = 8;
//		//压缩后每秒视频的bit位大小 50kB
//		vc->bit_rate = 50 * 1024 * 8;
//		vc->width = inWidth;
//		vc->height = inHeight;
//		vc->time_base = { 1,fps };
//		vc->framerate = { fps,1 };
//		//画面组的大小，多少帧一个关键帧
//		vc->gop_size = 50;
//		//不要b帧
//		vc->max_b_frames = 0;
//		vc->pix_fmt = AV_PIX_FMT_YUV420P;
//
//		result = avcodec_open2(vc, 0, 0);
//		if (result != 0)
//		{
//			char buf[1024] = { 0 };
//			av_strerror(result, buf, sizeof(buf) - 1);
//			throw exception(buf);
//		}
//
//		cout << "avcodec_open2 success!" << endl;
//
//
//		///封装器设置
//		result = avformat_alloc_output_context2(&ic, 0, "flv", outUrl);
//		if (result < 0)
//		{
//			char buf[1024] = { 0 };
//			av_strerror(result, buf, sizeof(buf) - 1);
//			throw exception(buf);
//		}
//
//		//添加视频流
//		AVStream *vs = avformat_new_stream(ic, NULL);
//		if (!vs) 
//		{
//			throw exception("avformat_new_stream failed");
//		}
//		vs->codecpar->codec_tag = 0;
//
//		//从编码器复制参数
//		avcodec_parameters_from_context(vs->codecpar, vc);
//		av_dump_format(ic, 0, outUrl, 1);
//
//		///打开网络IO流通道
//		result = avio_open(&ic->pb, outUrl, AVIO_FLAG_WRITE);
//		if (result < 0)
//		{
//			char buf[1024] = { 0 };
//			av_strerror(result, buf, sizeof(buf) - 1);
//			throw exception(buf);
//		}
//		//写入封装头
//		result = avformat_write_header(ic, NULL);
//		if (result < 0) {
//			char buf[1024 ]= { 0 };
//			av_strerror(result, buf, sizeof(buf) - 1);
//			throw exception(buf);
//		}
//
//
//		int vpts = 0;
//
//		AVPacket packet;
//		memset(&packet, 0, sizeof(AVPacket));
//		for (;;) {
//			///读取rtsp视频帧，并解码
//			//cam.read(frame); read内
//			//部就是做了grab和retrieve两步
//			if (!cam.grab()) {
//				continue;
//			}
//			///yuv转rgb
//			if (!cam.retrieve(frame)) {
//				continue;
//			}
//			imshow("video", frame);
//
//			///rgb to yuv
//			uint8_t * srcSlice[AV_NUM_DATA_POINTERS] = { 0 };
//			//indata[0] bgrbgrbgr
//			//plane indata[0] bbbbb indata[1]ggggg indata[2]rrrrr 
//			srcSlice[0] = frame.data;
//			int srcStride[AV_NUM_DATA_POINTERS] = { 0 };
//			//一行（宽）数据的字节数
//			srcStride[0] = frame.cols*frame.elemSize();
//			int h = sws_scale(vsc,
//				srcSlice,
//				srcStride,
//				0,
//				frame.rows,
//				yuv->data,
//				yuv->linesize
//			);
//			if (h < 0)
//			{
//				continue;
//			}
//
//			///h264编码
//			yuv->pts = vpts;
//
//			//pts需要增长，不然提示
//			// non-strictly-monotonic PTS
//			vpts++;
//			result = avcodec_send_frame(vc, yuv);
//
//			if (result != 0)
//			{
//				continue;
//			}
//			result = avcodec_receive_packet(vc, &packet);
//			if (result != 0 ){
//				continue;
//			}
//			cout << "*" << packet.size << flush;
//			
//			packet.pts = av_rescale_q(packet.pts, vc->time_base, vs->time_base);
//			packet.dts = av_rescale_q(packet.dts, vc->time_base, vs->time_base);
//			packet.duration = av_rescale_q(packet.duration, vc->time_base, vs->time_base);
//
//			result = av_interleaved_write_frame(ic, &packet);
//			if (result == 0)
//			{
//				cout <<"#" << flush;
//			}
//			waitKey(1);
//		}
//	}
//	catch (const std::exception& ex)
//	{
//		if (cam.isOpened())
//		{
//			cam.release();
//		}
//		if (vsc)
//		{
//			sws_freeContext(vsc);
//			//指针置NULL是一个良好的习惯
//			vsc = NULL;
//		}
//		if (vc)
//		{
//			avio_closep(&ic->pb);
//			avcodec_free_context(&vc);
//		}
//		cerr << ex.what()<< endl;
//	}
//
//	return 0;
//}

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