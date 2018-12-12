//#include "AudioRecord.h"
//#include <QAudioInput>
//#include <iostream>
//#include <list>
//using namespace std;
//class CXAudioRecord :public AudioRecord
//{
//public:
//	void Main()
//	{
//		cout << "进入音频录制线程" << endl;
//		//一次读取一帧音频的字节数
//		int readSize = nbSamples*channels*sampleByte;
//		char *buf = new char[readSize];
//		while (!isExit)
//		{
//			//读取已录制音频
//			//一次读取一帧音频
//			if (input->bytesReady() < readSize)
//			{
//				Sleep(1);
//				continue;
//			}
//
//			int size = 0;
//			while (size != readSize)
//			{
//				int len = io->read(buf + size, readSize - size);
//				if (len < 0)break;
//				size += len;
//			}
//			if (size != readSize)
//			{
//				continue;
//			}
//			//已经读取一帧音频
//			Push(Data(buf, readSize));
//		}
//		delete buf;
//		cout << "退出音频录制线程" << endl;
//	}
//
//	bool Init()
//	{
//		Stop();
//		///1 qt音频开始录制
//		QAudioFormat fmt;
//		fmt.setSampleRate(sampleRate);
//		fmt.setChannelCount(channels);
//		fmt.setSampleSize(sampleByte * 8);
//		fmt.setCodec("audio/pcm");
//		fmt.setByteOrder(QAudioFormat::LittleEndian);
//		fmt.setSampleType(QAudioFormat::UnSignedInt);
//		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
//		if (!info.isFormatSupported(fmt))
//		{
//			cout << "Audio format not support!" << endl;
//			fmt = info.nearestFormat(fmt);
//		}
//		input = new QAudioInput(fmt);
//
//		//开始录制音频
//		io = input->start();
//		if (!io)
//			return false;
//		return true;
//	}
//	void Stop()
//	{
//		DataThread::Stop();
//		if (input)
//			input->stop();
//		if (io)
//			io->close();
//		input = NULL;
//		io = NULL;
//	}
//	QAudioInput *input = NULL;
//	QIODevice *io = NULL;
//};
//
//AudioRecord *AudioRecord::Get(AUDIOTYPE type, unsigned char index)
//{
//	static CXAudioRecord record[255];
//	return &record[index];
//}
//
//AudioRecord::AudioRecord()
//{
//}
//
//
//AudioRecord::~AudioRecord()
//{
//}
