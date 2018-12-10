#pragma once

#include "Data.h"
#include <list>
#include <mutex>

void Sleep(int mis);

class DataThread
{
protected:
	//缓冲队列，先进先出
	std::list<Data> lists;

	//缓冲队列大小
	int listCount = 0;

	//锁
	std::mutex mutex;

	//线程退出标记
	bool isExit = false;

	//线程是否退出完成标记
	bool isRunning = false;

public:
	DataThread();

	//(缓冲列表大小）最大值，超出删除最旧的数据
	int maxListNum = 100;

	//插入列表结尾
	virtual void Push(Data d);

	//读取列表中最早插入的数据，返回的数据需要调用Data.Drop清理
	virtual Data Pop();

	//启动线程
	virtual bool Start();

	//退出线程，并等待线程退出（阻塞）
	virtual void Stop();

	//线程函数
	virtual void Main() {}

	virtual ~DataThread();

private:
	void ThreadMain();
};