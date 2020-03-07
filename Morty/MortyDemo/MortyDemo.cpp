// MortyDemo.cpp : 瀹氫箟鎺у埗鍙板簲鐢ㄧ▼搴忕殑鍏ュ彛鐐广€?
//

/*#include "vld.h"*/

#include "stdafx.h"
#include "MEngine.h"
#include "MColor.h"

#include "MVertex.h"
#include "MMesh.h"

#include "MObject.h"
#include "MMaterial.h"
#include "MModelInstance.h"
#include "MResourceManager.h"
#include "MVariant.h"
#include "MMaterialResource.h"
#include "MTexture.h"
#include "MTextureResource.h"
#include "MInputNode.h"

#include "MShader.h"
#include "MVertex.h"
#include "MIMeshInstance.h"
#include "MInputManager.h"
#include "MLogManager.h"
#include "MIRenderView.h"
#include "MCamera.h"
#include "MScene.h"
#include "MViewport.h"
#include "MPointLight.h"
#include "MDirectionalLight.h"
#include "MModelResource.h"
#include "MSkeletalAnimation.h"
#include "MTypedClass.h"

#include "MBounds.h"


#include "Quaternion.h"

#include "MainEditor.h"

class MyCamera : public MCamera
{
public:
	MyCamera()
		: MCamera()
	{}

	virtual void OnTick(const float& fDelta)
	{
		MPointLight* pLight = static_cast<MPointLight*>(GetRootNode()->FindFirstChildByName("Light"));

		const float speed = 25;
		
		if (true == m_tKeyBoardDown['W'])
		{
			this->SetPosition(this->GetPosition() + GetForward() * speed * fDelta);
		}
		if (true == m_tKeyBoardDown['S'])
		{
			this->SetPosition(this->GetPosition() + GetForward() * -speed * fDelta);
		}
		if (true == m_tKeyBoardDown['A'])
		{
			this->SetPosition(this->GetPosition() + GetRight() * -speed * fDelta);
		}
		if (true == m_tKeyBoardDown['D'])
		{
			this->SetPosition(this->GetPosition() + GetRight() * speed * fDelta);
		}
		if (true == m_tKeyBoardDown['Q'])
		{
			this->SetPosition(this->GetPosition() + GetUp() * -speed * fDelta);
		}
		if (true == m_tKeyBoardDown['E'])
		{
			this->SetPosition(this->GetPosition() + GetUp() * speed * fDelta);
		}
		if (true == m_tKeyBoardDown['N'])
		{
			//pLight->SetPosition(pLight->GetPosition() - Vector3(speed * fDelta, 0, 0));
			pLight->SetAmbientColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetDiffuseColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetSpecularColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
		}
		if (true == m_tKeyBoardDown['M'])
		{
			//pLight->SetPosition(pLight->GetPosition() + Vector3(speed * fDelta, 0, 0));
			pLight->SetAmbientColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetDiffuseColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetSpecularColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
		}
		if (m_tKeyBoardDown['L'])
		{
			LookAt(static_cast<M3DNode*>(GetRootNode()->FindFirstChildByName("Teaport"))->GetPosition(), GetUp());
		}
		else if (m_tKeyBoardDown[MMouseInputEvent::MEMouseDownButton::RightButton] && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
		{

			Vector3 up = Vector3(0, 1, 0);

			Vector3 right = GetRight();
			SetRotation(GetRotation() * Quaternion(up, m_v2MouseAddi.x * 0.25f) * Quaternion(right, m_v2MouseAddi.y * 0.25f));

			m_v2MouseAddi = Vector2(0, 0);
		}

	}

	std::map<unsigned int, bool> m_tKeyBoardDown;
	Vector2 m_v2MouseAddi;


};

int main(int argc, char* argv[])
{
	MEngine engine;

	engine.Initialize();

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
	pRootNode->SetName("RootNode");

	
	MModelResource* pResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/Ground.fbx"));

// 	for (int i = 0; i < 1; ++i)
// 	{
// 		MModelInstance* pSpatial = engine.GetObjectManager()->CreateObject<MModelInstance>();
// 		pSpatial->Load(pResource);
// 		pSpatial->SetPosition(Vector3(0, 0, i * 10));
// 		pSpatial->SetName("Ground");
// 
// 		pRootNode->AddNode(pSpatial);
// 
// 		pSpatial->SetRotation(Quaternion(Vector3(0, 1, 0), 90.0f));
// 	}


	
	MString textureID[] = {"005","003","007","004","014","008","002","015","019"};

	MModelResource* pPikachuResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->LoadResource("./Model/gun/model.dae"));
	for (int i = 0; i < 9; ++i)
	{
		MMaterial* pMaterial = pPikachuResource->GetMeshDefaultMaterial(i);
		std::vector<MShaderTextureParam>& vParams = pMaterial->GetPixelTextureParams();

		MResource* pDiffuseRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_albedo.jpg");
		MResource* pNormalMapRes = engine.GetResourceManager()->LoadResource("./Model/gun/tex/Material." + textureID[i] + "_normal.png");

		pMaterial->SetPixelTexutreParam("U_mat.texDiffuse", pDiffuseRes);
		pMaterial->SetPixelTexutreParam("U_mat.texNormal", pNormalMapRes);

		for (MShaderParam& param : pMaterial->GetPixelShaderParams())
		{
			if (param.unCode == SHADER_PARAM_CODE_MATERIAL)
			{
				MStruct* pStruct = param.var.GetStruct()->FindMember("U_mat")->GetStruct();
				pStruct->SetMember("bUseNormalTex", true);
				
				param.SetDirty();
				continue;
			}
		}
	}
	
	MModelInstance* pPikachu = engine.GetObjectManager()->CreateObject<MModelInstance>();
	pPikachu->Load(pPikachuResource);
	pPikachu->SetPosition(Vector3(0, 0, 10));
	pPikachu->SetScale(Vector3(10, 10, 10));
	pPikachu->SetName("Pikachu");

	

	pRootNode->AddNode(pPikachu);

	


	MPointLight* pLight = engine.GetObjectManager()->CreateObject<MPointLight>();
	pLight->SetName("Light");
	pLight->SetPosition(Vector3(10, 10, 200));
	pRootNode->AddNode(pLight);

	MDirectionalLight* pDirLight = engine.GetObjectManager()->CreateObject<MDirectionalLight>();
	pDirLight->SetName("DirLight");
	pRootNode->AddNode(pDirLight);
	//pDirLight->SetRotation(Quaternion(Vector3(1,0,0), -90.0f));

	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetPosition(Vector3(0, 10, -100));
	pCamera->SetName("Camera");
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

// 	MScene* pScene = engine.GetObjectManager()->CreateObject<MScene>();
// 	pScene->SetRootNode(pRootNode);
// 	MWindowsRenderView* pView = new MWindowsRenderView();
// 	pView->Initialize(&engine, "Morty");
// 	MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
// 	pView->AppendViewport(pViewport);
// 	pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewWidth()));
// 	pViewport->SetScene(pScene);
//	engine.AddView(pView);


	engine.SetRootNode(pRootNode);

	
	while (engine.MainLoop());

	engine.Release();


	system("PAUSE");
	return 0;
}
