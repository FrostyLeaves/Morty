// MortyDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "MEngine.h"

#include "MObject.h"
#include "MMaterial.h"
#include "MSpatial.h"
#include "MResourceManager.h"

#include "MShader.h"
#include "MVertex.h"
#include "MIRenderer.h"
#include "MMeshInstance.h"

int main(int argc, char* argv[])
{
    
    
	MEngine engine;

	engine.Initialize();
	engine.CreateView();



	MResource* pResource = engine.GetResourceManager()->Load("D:/marie naked/xxxf.FBX");
	MSpatial* pSpatial = engine.GetObjectManager()->CreateObject<MSpatial>();
	pSpatial->Load(pResource);

	MResource* pVSResource = engine.GetResourceManager()->Load("D:/marie naked/test_shader.mvs");
	MResource* pPSResource = engine.GetResourceManager()->Load("D:/marie naked/test_shader.mps");

	MShaderBuffer* pTestBuffer = nullptr;

	MMaterial* pPass = new MMaterial();
	pPass->LoadVertexShader(pVSResource);
	pPass->LoadPixelShader(pPSResource);

	for (MNode* pChild : pSpatial->GetChildren())
	{
		MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
		pMeshIns->SetMaterial(pPass);
	}
	
	engine.SetRootNode(pSpatial);

	while (engine.MainLoop());

	engine.Release();

	return 0;
}
