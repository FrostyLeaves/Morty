// MortyDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MEngine.h"

#include "MObject.h"
#include "MMeshInstance.h"
#include "MResourceManager.h"

int main(int argc, char* argv[])
{
    
    
	MEngine engine;

	engine.Initialize();
	engine.CreateView();



	MResource* pResource = engine.GetResourceManager()->Load("D:/marie naked/head.fbx");
	MMeshInstance* pMeshIns = engine.GetObjectManager()->CreateObject<MMeshInstance>();
	pMeshIns->Load(pResource);

	engine.SetRootNode(pMeshIns);

	while (engine.MainLoop());

	engine.Release();

	return 0;
}
