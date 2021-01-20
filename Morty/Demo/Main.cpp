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

#include "MMaterial.h"
#include "Model/MModelInstance.h"
#include "Model/MIMeshInstance.h"
#include "Model/MModelConverter.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"
#include "Model/MIModelMeshInstance.h"
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
#include "MCamera.h"
#include "MScene.h"
#include "MViewport.h"
#include "Light/MPointLight.h"
#include "Light/MSpotLight.h"
#include "Light/MDirectionalLight.h"
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
#include "MBasicRenderProgram.h"
#include "MForwardRenderProgram.h"

#include "MoveInputNode.h"

#ifdef MORTY_EDITOR_ENABLE
#include "MainEditor.h"
#include "NotifyManager.h"
#endif

int main(int argc, char* argv[])
{
	MEngine engine;
	engine.Initialize("../../../../Resource");


	// 	{
	// 		{
	// 			MModelConverter conver(&engine);
	// 			conver.Convert("./Model/NEW YEAR 2021_SF.fbx", "./Model/output", "snowman");
	// 		}
	// 	}

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
	pRootNode->SetName("RootNode");

    MResource* pNodeResourceBase = engine.GetResourceManager()->LoadResource("./Model/output/girl/girl.mnode");
    if (MNodeResource* pNodeResource = pNodeResourceBase ? pNodeResourceBase->DynamicCast<MNodeResource>() : nullptr)
    {
        MNode* pEditorNode = pNodeResource->CreateNode();

        pRootNode->AddNode(pEditorNode);
    }


	if (!pRootNode->FindFirstChildByType<MDirectionalLight>())
	{
		MDirectionalLight* pDirLight = engine.GetObjectManager()->CreateObject<MDirectionalLight>();
		pDirLight->SetName("DirLight");
		pRootNode->AddNode(pDirLight);
	}

	if (!pRootNode->FindFirstChildByType<MCamera>())
	{
		MCamera* pCamera = engine.GetObjectManager()->CreateObject<MCamera>();
		pCamera->SetPosition(Vector3(0, 0, -20));
		pCamera->SetName("Camera");
		pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
		pRootNode->AddNode(pCamera);
	}

	MoveInputNode* pMoveNode = engine.GetObjectManager()->CreateObject<MoveInputNode>();
	pMoveNode->SetName("InputNode");
	pRootNode->AddNode(pMoveNode);
	pMoveNode->SetMoveNode(pRootNode->FindFirstChildByName("Camera")->DynamicCast<M3DNode>());

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



	// 	MWindowsRenderView* pTestView = new MWindowsRenderView();
	// 	pTestView->Initialize(&engine, "Test");
	// 	engine.AddView(pTestView);
	// 	MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
	// 	pViewport->SetScene(pEditorView->GetScene());
	// 	pViewport->SetSize(Vector2(pTestView->GetViewWidth(), pTestView->GetViewWidth()));
	// 	pTestView->AppendViewport(pViewport);


	while (engine.MainLoop());

	engine.Release();

#ifdef MORTY_EDITOR_ENABLE
	NotifyManager::GetInstance()->UnRegisterAll();
#endif

	system("PAUSE");
	return 0;
}
