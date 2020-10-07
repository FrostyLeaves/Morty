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
#include "MInputNode.h"
#include "MIRenderer.h"

#include "Shader/MShader.h"
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


#include "MWindowsRenderView.h"

#include "MBasicRenderProgram.h"
#include "MForwardRenderProgram.h"

//#define MORTY_EDITOR_ENABLE

#ifdef MORTY_EDITOR_ENABLE
#include "MainEditor.h"
#include "NotifyManager.h"
#endif

static float rrrr = 0.0f;

class MyCamera : public MCamera
{
public:
	MyCamera()
		: MCamera()
	{}

	virtual void OnTick(const float& fDelta)
	{
		//auto jeep = GetRootNode()->FindFirstChildByName("Jeep")->DynamicCast<M3DNode>();
		//auto rot = jeep->GetRotation();
		//rot.RotateY(rrrr);
		//rrrr += fDelta * 10.0f;

		//jeep->SetRotation(rot);
		
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


//		MLogManager::GetInstance()->Log("Camera position: %f, %f, %f", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	std::map<unsigned int, bool> m_tKeyBoardDown;
	Vector2 m_v2MouseAddi;

	Vector3 m_v3MoveSpeed;
};

int main(int argc, char* argv[])
{
	MEngine engine;
	engine.Initialize("./");


// 	{
// 		{
// 			MModelConverter conver(&engine);
// 			conver.Convert("./Model/teaport.fbx", "./Model", "teaport");
// 		}
// 	}

 	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
 	pRootNode->SetName("RootNode");

	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetPosition(Vector3(0, 0, -20));
	pCamera->SetName("Camera");
	pCamera->SetZNearFar(Vector2(10, 500));
	pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
	//pCamera->SetCameraType(MCamera::MECameraType::EOrthographic);
	pRootNode->AddNode(pCamera);

	MModelResource* pJeepResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/teaport/teaport.model"));
	MModelInstance* pJeepModel = engine.GetObjectManager()->CreateObject<MModelInstance>();
	pJeepModel->SetPosition(Vector3(0, 0, 10));
	pJeepModel->SetScale(Vector3(1, 1, 1));
	pJeepModel->SetGenerateDirLightShadow(true);
	pJeepModel->Load(pJeepResource);
	pJeepModel->SetName("Jeep");
	pRootNode->AddNode(pJeepModel);
	pJeepModel->SetRotation(Quaternion(Vector3(1, 0, 0), 0));
	pJeepModel->GetFixedChildren()[0]->DynamicCast<M3DNode>()->SetRotation(Quaternion(Vector3(1, 0, 0), 0));

	auto rot = pJeepModel->GetRotation();
	rot.RotateY(-90);
	
	pJeepModel->SetRotation(rot);

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

#ifdef MORTY_EDITOR_ENABLE
	MainEditor* pEditorView = new MainEditor();
	pEditorView->Initialize(&engine, "Morty");
	engine.AddView(pEditorView);
	pEditorView->SetEditorNode(pRootNode);

#else
	engine.RegisterRenderProgram<MForwardRenderProgram>();
	MScene* pScene = engine.GetObjectManager()->CreateObject<MScene>();
	pScene->SetRootNode(pRootNode);
	MWindowsRenderView* pView = new MWindowsRenderView();
	pView->Initialize(&engine, "Morty");
	pView->SetBackColor(MColor(0.25f, 0.25f, 0.25f, 1.0f));
	MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
	//pViewport->RegisterRenderProgram<MBasicRenderProgram>();
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
