// MortyDemo.cpp : 定义控制台应用程序的入口点
//

#ifdef _DEBUG
//#include "vld.h"
#endif
#include "MObject.h"

#define MORTY_EDITOR_ENABLE

#include "MEngine.h"
#include "Type/MColor.h"
#include "MTypedClass.h"

#include "MVertex.h"
#include "MMesh.h"

#include "MNode.h"
#include "MMaterial.h"
#include "Model/MModelConverter.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"
#include "MResourceManager.h"
#include "MVariant.h"
#include "Material/MMaterialResource.h"
#include "Texture/MTextureResource.h"
#include "Node/MNodeResource.h"
#include "MTexture.h"
#include "MIRenderer.h"

#include "Shader/MShader.h"
#include "MVertex.h"
#include "MLogManager.h"
#include "MIRenderView.h"
#include "MScene.h"
#include "MViewport.h"
#include "MSkeletalAnimation.h"
#include "MTypedClass.h"

#include "MBounds.h"
#include "Timer/MTimer.h"

#include "Quaternion.h"


#include "Shader/MShaderResource.h"

#include "Json/MJson.h"
#include "MFileHelper.h"

#include "MFunction.h"


#include "DirectX11/MWindowsRenderView.h"

#include "MIRenderTarget.h"
#include "MForwardRenderProgram.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MRigidBodyComponent.h"
#include "MDirectionalLightComponent.h"

#include "MoveInputNode.h"

#ifdef MORTY_EDITOR_ENABLE
#include "MainEditor.h"
#include "NotifyManager.h"
#endif

#include "SDL.h"
#include <fstream>

#ifdef MORTY_WIN
#undef main
#endif

int main(int argc, char* argv[])
{
    std::string strBasePath = SDL_GetBasePath();

	MEngine engine;
	engine.Initialize("../../Resource");

	std::ifstream ifs("./Model/output/BerP/BerP.mnode", std::ios::binary);
	if (!ifs.good())
	{
		{
			MModelConvertInfo convertInfo;
			convertInfo.strResourcePath = "Model/pbr/1793_Bermuda_Penny/1793BerP.obj";
			convertInfo.strOutputDir = "./Model/output";
			convertInfo.strOutputName = "BerP";
			convertInfo.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;

			MModelConverter conver(&engine);
			conver.Convert(convertInfo);
		}
	}
	ifs.close();

	MNode* pRootNode = engine.GetObjectManager()->CreateObject<MNode>();
	pRootNode->SetName("RootNode");

	MResource* pNodeResourceBase = engine.GetResourceManager()->LoadResource("./Model/output/BerP/BerP.mnode");
//	MResource* pNodeResourceBase = engine.GetResourceManager()->LoadResource("./Model/output/bullet/bullet.mnode");
//    MResource* pNodeResourceBase = engine.GetResourceManager()->LoadResource("./Model/pbr/rustediron2/Sphere.mnode");
    if (MNodeResource* pNodeResource = pNodeResourceBase ? pNodeResourceBase->DynamicCast<MNodeResource>() : nullptr)
    {
        MNode* pEditorNode = pNodeResource->CreateNode();

        pRootNode->AddNode(pEditorNode);

		pEditorNode->RegisterComponent<MRigidBodyComponent>();
    }

	MNode* pDirectionalLight = engine.GetObjectManager()->CreateObject<MNode>();
	pDirectionalLight->SetName("DirectionalLight");
	pDirectionalLight->RegisterComponent<MSceneComponent>();
	if (MDirectionalLightComponent* pLightComponent = pDirectionalLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pLightComponent->SetLightIntensity(100.0f);
	}
	pRootNode->AddNode(pDirectionalLight);

	MNode* pCamera = engine.GetObjectManager()->CreateObject<MNode>();
	pCamera->SetName("Camera");
	pCamera->RegisterComponent<MSceneComponent>();
	pCamera->RegisterComponent<MCameraComponent>();
	pCamera->RegisterComponent<MoveInputComponent>();
	pRootNode->AddNode(pCamera);


// 	for (int x = 0; x < 10; ++x)
// 	{
// 		for (int y = 0; y < 10; ++y)
// 		{
// 			MPointLight* pLight = engine.GetObjectManager()->CreateObject<MPointLight>();
// 			pLight->SetLightIntensity(100.0f); 
// 			pLight->SetName(MString("PointLight_") + MStringHelper::ToString(x * 10 + y));
// 			pRootNode->AddNode(pLight);
// 
// 			pLight->SetPosition(Vector3(x * 2, 10, y * 2));
// 		}
// 	}


// 	if (!pRootNode->FindFirstChildByType<MCamera>())
// 	{
// 		MCamera* pCamera = engine.GetObjectManager()->CreateObject<MCamera>();
// 		pCamera->SetPosition(Vector3(0, 3, -8));
// 		pCamera->SetName("Camera");
// 		pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
// 		pRootNode->AddNode(pCamera);
// 	}
// 
// 	MoveInputNode* pMoveNode = engine.GetObjectManager()->CreateObject<MoveInputNode>();
// 	pMoveNode->SetName("InputNode");
// 	pRootNode->AddNode(pMoveNode);
// 	pMoveNode->SetMoveNode(pRootNode->FindFirstChildByName("Camera")->DynamicCast<M3DNode>());

#ifdef MORTY_EDITOR_ENABLE
	MainEditor* pEditorView = new MainEditor();
	pEditorView->Initialize(&engine, "Morty");
	engine.AddView(pEditorView);
	pEditorView->SetEditorNode(pRootNode);

	// 	pEditorView->SetCloseCallback([pEditorNode, pNodeResource]() {
	// 		pNodeResource->SaveByNode(pEditorNode);
	// 		return true;
	// 	});

#else
	MScene* pScene = engine.GetObjectManager()->CreateObject<MScene>();
	pScene->SetRootNode(pRootNode);
	MWindowsRenderView* pView = new MWindowsRenderView();
	pView->Initialize(&engine, "Morty");
	pView->SetBackColor(MColor(0.25f, 0.25f, 0.25f, 1.0f));
	pView->GetRenderTarget()->RegisterRenderProgram<MForwardRenderProgram>();

	MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
	pView->AppendViewport(pViewport);
	pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewHeight()));
	pViewport->SetScene(pScene);
	engine.AddView(pView);
	engine.SetScene(pScene);

#endif


	while (engine.MainLoop());

	engine.Release();

#ifdef MORTY_EDITOR_ENABLE
	NotifyManager::GetInstance()->UnRegisterAll();
#endif

	//system("PAUSE");
	return 0;
}
