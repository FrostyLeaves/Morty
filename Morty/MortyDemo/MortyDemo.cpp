// MortyDemo.cpp : 定义控制台应用程序的入口点
//

#ifdef _DEBUG
//#include "vld.h"
#endif

//#define MORTY_EDITOR_ENABLE

#include "stdafx.h"
#include "MEngine.h"
#include "Type/MColor.h"
#include "MTypedClass.h"

#include "MVertex.h"
#include "MMesh.h"

#include "MObject.h"
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
#include "MInputNode.h"
#include "MIRenderer.h"

#include "MShader.h"
#include "MVertex.h"
#include "MInputManager.h"
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

#include "MBasicRenderSystem.h"
#include "MRenderSystem.h"

#include "MWindowsRenderView.h"


#define MORTY_EDITOR_ENABLE

#ifdef MORTY_EDITOR_ENABLE
#include "MainEditor.h"
#include "NotifyManager.h"
#endif

class MyCamera : public MCamera
{
public:
	MyCamera()
		: MCamera()
	{}

	virtual void OnTick(const float& fDelta)
	{
		MPointLight* pLight = static_cast<MPointLight*>(GetRootNode()->FindFirstChildByName("Light"));

		const float speed = 16;
		
		if (true == m_tKeyBoardDown['W'])
		{
			m_v3MoveSpeed += GetForward() * speed * fDelta;
		}
		if (true == m_tKeyBoardDown['S'])
		{
			m_v3MoveSpeed += GetForward() * -speed * fDelta;
		}
		if (true == m_tKeyBoardDown['A'])
		{
			m_v3MoveSpeed += GetRight() * -speed * fDelta;
		}
		if (true == m_tKeyBoardDown['D'])
		{
			m_v3MoveSpeed += GetRight() * speed * fDelta;
		}
		if (true == m_tKeyBoardDown['Q'])
		{
			m_v3MoveSpeed += Vector3(0, 1, 0) * -speed * fDelta;
		}
		if (true == m_tKeyBoardDown['E'])
		{
			m_v3MoveSpeed += Vector3(0, 1, 0) * speed * fDelta;
		}
		else if (m_tKeyBoardDown[MMouseInputEvent::MEMouseDownButton::RightButton] && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
		{

			Vector3 up = Vector3(0, 1, 0);

			Vector3 right = GetRight();
			SetRotation(GetRotation() * Quaternion(up, m_v2MouseAddi.x * 0.25f) * Quaternion(right, m_v2MouseAddi.y * 0.25f));

			m_v2MouseAddi = Vector2(0, 0);
		}

		float fLength = m_v3MoveSpeed.Length();
		if (fLength > speed)
		{
			m_v3MoveSpeed.Normalize();
			m_v3MoveSpeed = m_v3MoveSpeed * speed;
		}

		SetPosition(GetPosition() + m_v3MoveSpeed);
		m_v3MoveSpeed = m_v3MoveSpeed * 0.8f;
	}

	std::map<unsigned int, bool> m_tKeyBoardDown;
	Vector2 m_v2MouseAddi;

	Vector3 m_v3MoveSpeed;
};

int main(int argc, char* argv[])
{
	MEngine engine;
	engine.Initialize();


// 	MShaderBuffer* buffer = nullptr;
// 	MShaderMacro macro;
// 	engine.GetDevice()->CompileShader(&buffer, "D:/Custom/x/Morty/Morty/Morty/Resource/Shader/test.mvs", MShader::Vertex, macro);
// 

// 	{
//  		{
// 			MModelConverter conver(&engine);
// 			conver.Convert("./Model/Pikachu.fbx", "./Model", "Pikachu");
//  		}
// // 		engine.Release();
// // 		return 0;
// 	}

	 
#ifdef MORTY_EDITOR_ENABLE

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
	pRootNode->SetName("RootNode");

	auto time = MTimer::GetCurTime();

	MModelResource* pGroundResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/Sphere/Sphere.model"));

	time = MTimer::GetCurTime() - time;

	MLogManager::GetInstance()->Log("Load Model Time: %lld", time);

	for (int i = 0; i < 1; ++i)
	{
		MModelInstance* pSpatial = engine.GetObjectManager()->CreateObject<MModelInstance>();
		pSpatial->Load(pGroundResource);
		pSpatial->SetPosition(Vector3(0, 0, 0));
		pSpatial->SetScale(Vector3(1, 1, 1));
		pSpatial->SetName("Cat");

		pRootNode->AddNode(pSpatial);

		//	pSpatial->SetRotation(Quaternion(Vector3(0, 1, 0), 90.0f));
	}


	MModelResource* pResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/nfsq/model.dae"));

	for (int i = 0; i < 1; ++i)
	{
		MModelInstance* pSpatial = engine.GetObjectManager()->CreateObject<MModelInstance>();
		pSpatial->Load(pResource);
		pSpatial->SetPosition(Vector3((i / 6) * 12, 0, -(i % 6) * 12));
		pSpatial->SetScale(Vector3(10, 10, 10));
		pSpatial->SetName("Ground");

		pRootNode->AddNode(pSpatial);
	}


// 	MString textureID[] = {"005","003","007","004","014","008","002","015","019"};
// 
// 	MModelResource* pPikachuResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/gun/model.dae"));
// 	for (int i = 0; i < 9; ++i)
// 	{
// 		MMaterial* pMaterial = dynamic_cast<MMaterial*>((*pPikachuResource->GetMeshes())[i]->GetDefaultMaterial());
// 		MResource* pDiffuseRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_albedo.jpg");
// 		MResource* pNormalMapRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_normal.png");
// 
// 		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_DIFFUSE, pDiffuseRes);
// 		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_NORMAL, pNormalMapRes);
// 
// 		for (MShaderParam& param : *pMaterial->GetShaderParams())
// 		{
// 			if (param.unCode == SHADER_PARAM_CODE_MATERIAL)
// 			{
// 				MStruct* pStruct = param.var.GetStruct()->FindMember("U_mat")->GetStruct();
// 				pStruct->SetMember("bUseNormalTex", true);
// 
// 				param.SetDirty();
// 				continue;
// 			}
// 		}
// 	}
// 
// 	for (unsigned int i = 0; i < 1; ++i)
// 	{
// 		MModelInstance* pPikachu = engine.GetObjectManager()->CreateObject<MModelInstance>();
// 		pPikachu->Load(pPikachuResource);
// 		pPikachu->SetGenerateDirLightShadow(true);
// 		pPikachu->SetPosition(Vector3(0, 0, 10));
// 		pPikachu->SetScale(Vector3(10, 10, 10));
// 		pPikachu->SetName("Pikachu");
// 
// 		pRootNode->AddNode(pPikachu);
// 	}

	for (unsigned int i = 0; i < 1; ++i)
	{
		MSpotLight* pLight = engine.GetObjectManager()->CreateObject<MSpotLight>();
		pLight->SetName("Spot Light");
		pLight->SetPosition(Vector3(0, 0, 10000));
		pRootNode->AddNode(pLight);
	}

	for (unsigned int i = 0; i < 1; ++i)
	{
		MPointLight* pLight = engine.GetObjectManager()->CreateObject<MPointLight>();
		pLight->SetName("Light");
		pLight->SetPosition(Vector3(0, 0, 10000));
		pRootNode->AddNode(pLight);
	}

	MDirectionalLight* pDirLight = engine.GetObjectManager()->CreateObject<MDirectionalLight>();
	pDirLight->SetName("DirLight");
	pRootNode->AddNode(pDirLight);

	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetPosition(Vector3(0, 10, -30));
	pCamera->SetName("Camera");
	pCamera->SetZNearFar(Vector2(10, 500));
	pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
	pRootNode->AddNode(pCamera);

	MInputNode* pInputNode = engine.GetObjectManager()->CreateObject<MInputNode>();
	pInputNode->SetName("InputNode");
	pCamera->AddNode(pInputNode);

	pInputNode->SetInputCallback([&pCamera](MInputEvent* pEvent, MViewport* pViewport) {

		if (MKeyBoardInputEvent* pKeyInput = dynamic_cast<MKeyBoardInputEvent*>(pEvent))
		{
			pCamera->m_tKeyBoardDown[pKeyInput->GetKey()] = pKeyInput->GetType() == MEKeyState::DOWN;

		}
		else if (MMouseInputEvent* pMouseInput = dynamic_cast<MMouseInputEvent*>(pEvent))
		{
			if (pMouseInput->GetType() == MMouseInputEvent::MouseMove)
			{
				pCamera->m_v2MouseAddi = pMouseInput->GetMouseAddition();
			}
			else
			{
				pCamera->m_tKeyBoardDown[(unsigned int)pMouseInput->GetButton()] = pMouseInput->GetType() == MMouseInputEvent::ButtonDown;
			}
		}

		return false;
	});

	MainEditor* pEditorView = new MainEditor();
	pEditorView->Initialize(&engine, "Morty");
	engine.AddView(pEditorView);
	pEditorView->SetEditorNode(pRootNode);
#endif

// 	MString code;
// 	pRootNode->Encode(code);
// 
// 	MFileHelper::WriteString("D:/Test.node", code);







// 	if (MResource* pResource = engine.GetResourceManager()->LoadResource("D:/Test.node"))
// 	{
// 		if (MNodeResource* pNodeResource = pResource->DynamicCast<MNodeResource>())
// 		{
// 			MNode* pRootNode = pNodeResource->CreateNode();
// 			pEditorView->SetEditorNode(pRootNode);
// 		}
// 	}

// 	MScene* pScene = engine.GetObjectManager()->CreateObject<MScene>();
// 	//pScene->RegisterSystem<MBasicRenderSystem>();
// 	pScene->RegisterSystem<MRenderSystem>();
// 	pScene->SetRootNode(nullptr);
// 	MWindowsRenderView* pView = new MWindowsRenderView();
// 	pView->Initialize(&engine, "Morty");
// 	MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
// 	pView->AppendViewport(pViewport);
// 	pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewWidth()));
// 	pViewport->SetScene(pScene);
// 	engine.AddView(pView);
// 
// 	pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewHeight()));

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
