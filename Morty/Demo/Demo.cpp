// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include "MEngine.h"

#include "MTaskGraph.h"
#include "MObjectSystem.h"

int main()
{
	MEngine engine;

	engine.Initialize();

	engine.Start();



	engine.Stop();

	engine.Release();
}
