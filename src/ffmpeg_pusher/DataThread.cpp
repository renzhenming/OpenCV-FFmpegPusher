#include "DataThread.h"

//thread需要同时引入下边两个
#include <thread>
using namespace std;


void Sleep(int mis)
{
	chrono::milliseconds du(mis);
	this_thread::sleep_for(du);
}

DataThread::DataThread() 
{

}

//插入列表结尾
void DataThread::Push(Data d)
{
	mutex.lock();
	//如果此时队列已满，则删除最早插入的那条数据
	if (lists.size() > maxListNum)
	{
		lists.front().Drop();
		lists.pop_front();
	}
	//新插入一条数据
	lists.push_back(d);
	mutex.unlock();
}

//读取列表中最早插入的数据，返回的数据需要调用Data.Drop清理
Data  DataThread::Pop()
{
	mutex.lock();
	if (lists.empty())
	{
		mutex.unlock();
		return Data();
	}
	//取出一条
	Data data = lists.front();
	//从队列中清除
	lists.pop_front();
	mutex.unlock();
	return data;
}

//启动线程
bool  DataThread::Start()
{
	isExit = false;
	thread th(&DataThread::ThreadMain, this);
	th.detach();
	return true;
}

void DataThread::ThreadMain()
{
	isRunning = true;
	Main();
	//main执行完成就是线程退出完成
	isRunning = false;
}

//退出线程，并等待线程退出（阻塞）
void  DataThread::Stop()
{
	isExit = true;
	for (int i = 0; i < 200; i++)
	{
		if (!isRunning)
		{
			return;
		}
		Sleep(1);
	}
}

DataThread::~DataThread()
{

}
