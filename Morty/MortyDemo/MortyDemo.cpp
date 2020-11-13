// MortyDemo.cpp : 定义控制台应用程序的入口点
//

#ifdef _DEBUG
//#include "vld.h"
#endif
#include "MObject.h"

#define MORTY_EDITOR_ENABLE

#include "stdafx.h"
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


#include "MWindowsRenderView.h"

#include "MIRenderTarget.h"
#include "MBasicRenderProgram.h"
#include "MForwardRenderProgram.h"

#include "EditorCamera.h"

#ifdef MORTY_EDITOR_ENABLE
#include "MainEditor.h"
#include "NotifyManager.h"
#endif

int main(int argc, char* argv[])
{
	MEngine engine;
	engine.Initialize("./");


// 	{
// 		{
// 			MModelConverter conver(&engine);
// 			conver.Convert("./Model/just-a-girl/source/final_v01.obj", "./Model/output", "girl");
// 		}
// 	}

 	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
 	pRootNode->SetName("RootNode");

	EditorCamera* pCamera = engine.GetObjectManager()->CreateObject<EditorCamera>();
	pCamera->SetPosition(Vector3(0, 0, -20));
	pCamera->SetName("Camera");
	pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
	//pCamera->SetCameraType(MCamera::MECameraType::EOrthographic);
	pRootNode->AddNode(pCamera);

	MNodeResource* pNodeResource = engine.GetResourceManager()->LoadResource("./Model/output/girl/girl.mnode")->DynamicCast<MNodeResource>();

	MNode* pEditorNode = pNodeResource->CreateNode();

	pRootNode->AddNode(pEditorNode);

// 	if (auto pMeshIns = pJeepModel->GetFixedChildren()[0]->DynamicCast<MIModelMeshInstance>())
// 	{
// 		pMeshIns->GetMaterial()->SetMaterialType(MEMaterialType::ETransparent);
// 	}


// // 	MString textureID[] = {"005","003","007","004","014","008","002","015","019"};
// // 
// // 	MModelResource* pPikachuResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/gun/model.dae"));
// // 	for (int i = 0; i < 9; ++i)
// // 	{
// // 		MMaterial* pMaterial = dynamic_cast<MMaterial*>((*pPikachuResource->GetMeshes())[i]->GetDefaultMaterial());
// // 		MResource* pDiffuseRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_albedo.jpg");
// // 		MResource* pNormalMapRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_normal.png");
// // 
// // 		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_DIFFUSE, pDiffuseRes);
// // 		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_NORMAL, pNormalMapRes);
// // 
// // 		for (MShaderParam& param : *pMaterial->GetShaderParams())
// // 		{
// // 			if (param.unCode == SHADER_PARAM_CODE_MATERIAL)
// // 			{
// // 				MStruct* pStruct = param.var.GetStruct()->FindMember("U_mat")->GetStruct();
// // 				pStruct->SetMember("bUseNormalTex", true);
// // 
// // 				param.SetDirty();
// // 				continue;
// // 			}
// // 		}
// // 	}
// // 
// // 	for (unsigned int i = 0; i < 1; ++i)
// // 	{
// // 		MModelInstance* pPikachu = engine.GetObjectManager()->CreateObject<MModelInstance>();
// // 		pPikachu->Load(pPikachuResource);
// // 		pPikachu->SetGenerateDirLightShadow(true);
// // 		pPikachu->SetPosition(Vector3(0, 0, 10));
// // 		pPikachu->SetScale(Vector3(10, 10, 10));
// // 		pPikachu->SetName("Pikachu");
// // 
// // 		pRootNode->AddNode(pPikachu);
// // 	}


	MDirectionalLight* pDirLight = engine.GetObjectManager()->CreateObject<MDirectionalLight>();
	pDirLight->SetName("DirLight");
	pRootNode->AddNode(pDirLight);


	for (int i = 0; i < 1; ++i)
	{
		MPointLight* pPointLight = engine.GetObjectManager()->CreateObject<MPointLight>();
		pPointLight->SetName(MString("PointLight_") + MStringHelper::ToString(i));
		pRootNode->AddNode(pPointLight);
	}


	
#ifdef MORTY_EDITOR_ENABLE
	MainEditor* pEditorView = new MainEditor();
	pEditorView->Initialize(&engine, "Morty");
	engine.AddView(pEditorView);
	pEditorView->SetEditorNode(pRootNode);

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
