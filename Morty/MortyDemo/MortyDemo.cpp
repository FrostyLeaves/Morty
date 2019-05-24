// MortyDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MEngine.h"

int _tmain(int argc, _TCHAR* argv[])
{
	MEngine engine;

	engine.Initialize();

	engine.Run();

//	while (!view.IsClosed());

	return 0;
}

