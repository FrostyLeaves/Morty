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

#include "MShader.h"
#include "MVertex.h"
#include "MMeshInstance.h"
#include "MInputManager.h"
#include "MLogManager.h"
#include "MIRenderView.h"
#include "MCamera.h"
#include "MIScene.h"
#include "MIViewport.h"
#include "MPointLight.h"
#include "MModelResource.h"
#include "MSkeletalAnimation.h"

#include "MBounds.h"


#include "Quaternion.h"

#include "MainEditor.h"

class MySpatial : public MModelInstance
{
public:

	MySpatial(){
		m_fTime = 0.0f;
	}

	virtual void OnTick(const float& fDelta)
	{
		MModelInstance::OnTick(fDelta);
// 		m_fTime += fDelta;
// 
// 		Quaternion quat = Quaternion(Vector3(1, 0, 0), -90);
// 		Quaternion quatb(Vector3(0, 1, 0), 10 * m_fTime);
// 		this->SetRotation(quat * quatb);

	}

private:

	float m_fTime;
};


class MyCamera : public MCamera
{
public:

	MyCamera()
	{
		m_bW = false;
		m_bS = false;
		m_bA = false;
		m_bD = false;
		m_bQ = false;
		m_bE = false;
		m_bL = false;
		m_bRB = false;
	}

	virtual void OnTick(const float& fDelta)
	{
		MPointLight* pLight = static_cast<MPointLight*>(GetRootNode()->FindFirstChildByName("Light"));

		const float speed = 25;

		if (true == m_bW)
		{
			this->SetPosition(this->GetPosition() + GetForward() * speed * fDelta);
		}
		if (true == m_bS)
		{
			this->SetPosition(this->GetPosition() + GetForward() * -speed * fDelta);
		}
		if (true == m_bA)
		{
			this->SetPosition(this->GetPosition() + GetRight() * -speed * fDelta);
		}
		if (true == m_bD)
		{
			this->SetPosition(this->GetPosition() + GetRight() * speed * fDelta);
		}
		if (true == m_bQ)
		{
			this->SetPosition(this->GetPosition() + GetUp() * -speed * fDelta);
		}
		if (true == m_bE)
		{
			this->SetPosition(this->GetPosition() + GetUp() * speed * fDelta);
		}
		if (true == m_bN)
		{
			//pLight->SetPosition(pLight->GetPosition() - Vector3(speed * fDelta, 0, 0));
			pLight->SetAmbientColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetDiffuseColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetSpecularColor(pLight->GetDiffuseColor().ToVector3() + MColor(1, 1, 1).ToVector3() * fDelta);
		}
		if (true == m_bM)
		{
			//pLight->SetPosition(pLight->GetPosition() + Vector3(speed * fDelta, 0, 0));
			pLight->SetAmbientColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetDiffuseColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
			pLight->SetSpecularColor(pLight->GetDiffuseColor().ToVector3() - MColor(1, 1, 1).ToVector3() * fDelta);
		}
		if (m_bL)
		{
			LookAt(static_cast<M3DNode*>(GetRootNode()->FindFirstChildByName("Teaport"))->GetPosition(), GetUp());
		}
// 		else if (m_bRB && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
// 		{
// 
// 			Vector3 up = Vector3(0, 1, 0);
// 			SetRotation(GetRotation() * Quaternion(up, m_v2MouseAddi.x * 0.25f));
// 
// 			Vector3 right = GetRight();
// 			SetRotation(GetRotation() * Quaternion(right, m_v2MouseAddi.y * 0.25f));
// 
// 			m_v2MouseAddi = Vector2(0, 0);
// 		}

	}


	bool m_bW;
	bool m_bS;
	bool m_bA;
	bool m_bD;
	bool m_bQ;
	bool m_bE;
	bool m_bL;
	bool m_bN;
	bool m_bM;

	bool m_bRB;

	Vector2 m_v2MouseAddi;


};

int main(int argc, char* argv[])
{

// 	Vector3 pos[3] = {
// 		{ 1, 1, 1 }, { 1, 9, 13 }, {9, 6, 7}
// 	};
// 	std::vector <MMesh<MVertex>*> vMesh;
// 	MMesh<MVertex>* ptest = new MMesh<MVertex>();
// 	vMesh.push_back(ptest);
// 
// 	ptest->CreateVertices(3);
// 	for (int i = 0; i < 3; ++i)
// 		ptest->GetVertices()[i].position = pos[i];
// 
// 	MBoundsOBB obb(vMesh);

	Quaternion qqq;

	qqq.x = 0.1;
	qqq.y = 0.2;
	qqq.z = 0.3;
	qqq.w = 0.6;
	qqq.Normalize();

	qqq.SetEulerAngle(Vector3(30, 120, 45));

	Quaternion ooo; 
	ooo.SetEulerAngle(Vector3(30 - 180.0f , 180.0f - 120, 45 - 180.0f));

	Vector3  v = qqq.GetEulerAngle();
	Vector3 n = ooo.GetEulerAngle();

	Quaternion xxx;
	xxx.SetEulerAngle(v);
	xxx.Normalize();
	Vector3 w = xxx.GetEulerAngle();

	qqq.x = 0.1176;
	qqq.y = 0.5428;
	qqq.z = 0.1176;
	qqq.w = 0.8232;

	v = qqq.GetEulerAngle();

	MEngine engine;

	engine.Initialize();

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
	pRootNode->SetName("RootNode");


	MModelResource* pResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->Load("./Model/Pikachu.fbx"));
	MModelInstance* pSpatial = engine.GetObjectManager()->CreateObject<MySpatial>();
	pSpatial->Load(pResource);
	pSpatial->SetPosition(Vector3(0, 0, 0));
	pSpatial->SetName("Teaport");

	for (int i = 0; i < pSpatial->GetChildren().size(); ++i)
	{

		MNode* pChild = pSpatial->GetChildren()[i];

		MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);

		MResource* pVSResource = engine.GetResourceManager()->Load("./Shader/animationModel.mvs");
		MResource* pPSResource = engine.GetResourceManager()->Load("./Shader/model.mps");
		MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(engine.GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
		pMaterialRes->LoadVertexShader(pVSResource);
		pMaterialRes->LoadPixelShader(pPSResource);

		MMaterial* pMaterial = engine.GetObjectManager()->CreateObject<MMaterial>();
		pMaterial->Load(pMaterialRes);

		if (MResource* pTexResource = engine.GetResourceManager()->Load("./Model/teaport.png"))
		{
			pMaterial->SetPixelTexutreParam("U_mat.texDiffuse", pTexResource);
		//	pMaterial->SetPixelTexutreParam("U_mat.texSpecular", pTexResource);
		}

		for(MShaderParam& param : pMaterial->GetPixelShaderParams())
		{
			if (param.strName == "cbMaterial")
			{
				if (MStruct* pStruct = param.var.GetByType<MStruct>())
				{
					if (MStruct* pMat = pStruct->FindMember("U_mat")->GetByType<MStruct>())
					{
						pMat->SetMember("fShininess", 10.0f);
					}
				}
			}
		}
		pMeshIns->SetMaterial(pMaterial);
	}

	pRootNode->AddNode(pSpatial);

	pSpatial->SetPlayAnimation("Armature|Walk");


	MPointLight* pLight = engine.GetObjectManager()->CreateObject<MPointLight>();
	pLight->SetName("Light");
	pLight->SetPosition(Vector3(10, 10, 200));
	pRootNode->AddNode(pLight);

	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetName("Camera");
	pRootNode->AddNode(pCamera);

	MainEditor* pEditorView = new MainEditor();
	pEditorView->Initialize(&engine, "Morty");
	engine.AddView(pEditorView);

	pEditorView->SetEditorNode(pRootNode);

// 	MInputListener* pListener = new MInputListener();
// 	pListener->m_function = [&](MInputEvent* pEvent){
// 
// 		if (MKeyBoardInputEvent* pKeyInput = dynamic_cast<MKeyBoardInputEvent*>(pEvent))
// 		{
// 			if (pKeyInput->GetKey() == 'W')
// 			{
// 				pCamera->m_bW = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'S')
// 			{
// 				pCamera->m_bS = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'A')
// 			{
// 				pCamera->m_bA = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'D')
// 			{
// 				pCamera->m_bD = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'Q')
// 			{
// 				pCamera->m_bQ = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'E')
// 			{
// 				pCamera->m_bE = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'L')
// 			{
// 				pCamera->m_bL = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'N')
// 			{
// 				pCamera->m_bN = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 			if (pKeyInput->GetKey() == 'M')
// 			{
// 				pCamera->m_bM = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
// 			}
// 		}
// 
// 		else if (MMouseInputEvent* pMouseInput = dynamic_cast<MMouseInputEvent*>(pEvent))
// 		{
// 			if (pMouseInput->GetType() == MMouseInputEvent::MouseMove && pCamera->m_bRB)
// 			{
// 				Vector2 addi = pMouseInput->GetMouseAddition();
// 				pCamera->m_v2MouseAddi = addi;
// 
// 				Vector3 up = Vector3(0, 1, 0);
// 				pCamera->SetRotation(pCamera->GetRotation()* Quaternion(up, pCamera->m_v2MouseAddi.x * 0.25f));
// 
// 				Vector3 right = pCamera->GetRight();
// 				pCamera->SetRotation(pCamera->GetRotation()* Quaternion(right, pCamera->m_v2MouseAddi.y * 0.25f));
// 
// 				pCamera->m_v2MouseAddi = Vector2(0, 0);
// 			}
// 			else
// 			{
// 				if (pMouseInput->GetButton() == MMouseInputEvent::MEMouseDownButton::RightButton)
// 				{
// 					pCamera->m_bRB = pMouseInput->GetType() == MMouseInputEvent::ButtonDown;
// 				}
// 			}
// 		}
// 
// 	};
// 
// 	engine.GetInputManager()->AddListener(pListener);
	
	engine.SetRootNode(pRootNode);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
