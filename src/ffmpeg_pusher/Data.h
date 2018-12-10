#pragma once

class Data
{
public:
	char *data = 0;
	int size = 0;

	//创建空间并复制data内容
	Data(char *data, int size);
	void Drop();

	Data();
	virtual ~Data();
};