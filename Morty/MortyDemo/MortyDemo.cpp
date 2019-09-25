// MortyDemo.cpp : 瀹氫箟鎺у埗鍙板簲鐢ㄧ▼搴忕殑鍏ュ彛鐐广€?
//

/*#include "vld.h"*/

#include "stdafx.h"
#include "MEngine.h"

#include "MObject.h"
#include "MMaterial.h"
#include "MSpatial.h"
#include "MResourceManager.h"
#include "MVariable.h"
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


#include "Quaternion.h"

#include "Renderer/DirectX11/MDirectX11Device.h"


class MySpatial : public MSpatial
{
public:

	MySpatial(){
		m_fTime = 0.0f;
	}

	virtual void OnTick(const float& fDelta)
	{
		m_fTime += fDelta;

		Quaternion quat = Quaternion(Vector3(1, 0, 0), 3.14 / 2 * 3);
		Quaternion quatb(Vector3(0, 1, 0), 0.5f * m_fTime);
		this->SetRotation(quat * quatb);

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
		m_bLB = false;
	}

	virtual void OnTick(const float& fDelta)
	{

		const float speed = 50;

		if (true == m_bW)
		{
			this->SetPosition(this->GetPosition() + Vector3(0, 0, speed * fDelta));
		}
		if (true == m_bS)
		{
			this->SetPosition(this->GetPosition() + Vector3(0, 0, -speed * fDelta));
		}
		if (true == m_bA)
		{
			this->SetPosition(this->GetPosition() + Vector3(-speed * fDelta, 0, 0));
		}
		if (true == m_bD)
		{
			this->SetPosition(this->GetPosition() + Vector3(speed * fDelta, 0, 0));
		}
		if (true == m_bQ)
		{
			this->SetPosition(this->GetPosition() + Vector3(0, -speed * fDelta ,0));
		}
		if (true == m_bE)
		{
			this->SetPosition(this->GetPosition() + Vector3(0, speed * fDelta, 0));
		}

		if (m_bLB && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
		{
			m_v3Rotate.x += 0.02f * m_v2MouseAddi.x;
			m_v3Rotate.y += 0.02f * m_v2MouseAddi.y;
			SetRotation(Quaternion(Vector3(0, 1, 0), m_v3Rotate.x) * Quaternion(Vector3(1, 0, 0), m_v3Rotate.y));
			m_v2MouseAddi = Vector2(0, 0);
		}
	}


	bool m_bW;
	bool m_bS;
	bool m_bA;
	bool m_bD;
	bool m_bQ;
	bool m_bE;

	bool m_bLB;

	Vector2 m_v2MouseAddi;
	Vector3 m_v3Rotate;


};

int main(int argc, char* argv[])
{
	MEngine engine;

	engine.Initialize();

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();


	MResource* pResource = engine.GetResourceManager()->Load("./Model/teaport.fbx");
	MSpatial* pSpatial = engine.GetObjectManager()->CreateObject<MySpatial>();
	pSpatial->Load(pResource);
	pSpatial->SetPosition(Vector3(0, -20, 200));


	for (int i = 0; i < pSpatial->GetChildren().size(); ++i)
	{

		MNode* pChild = pSpatial->GetChildren()[i];

		MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);

		MResource* pVSResource = engine.GetResourceManager()->Load("./Shader/defaultv.mvs");
		MResource* pPSResource = engine.GetResourceManager()->Load("./Shader/defaultp.mps");
		MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(engine.GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
		pMaterialRes->LoadVertexShader(pVSResource);
		pMaterialRes->LoadPixelShader(pPSResource);

		MMaterial* pMaterial = engine.GetObjectManager()->CreateObject<MMaterial>();
		pMaterial->Load(pMaterialRes);

		if (MResource* pTexResource = engine.GetResourceManager()->Load("./Model/teaport.png"))
		{
			pMaterial->SetPixelTexutreParam("texture0", pTexResource);
		}

		std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();



		pMeshIns->SetMaterial(pMaterial);
	}

	pRootNode->AddNode(pSpatial);


	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetName("Camera");
	pRootNode->AddNode(pCamera);

	MIScene* pScene = engine.GetObjectManager()->CreateObject<MIScene>();
	pScene->SetRootNode(pRootNode);

	engine.CreateView()->GetViewport()->SetScene(pScene);


	MInputListener* pListener = new MInputListener();
	pListener->m_function = [&](MInputEvent* pEvent){

		if (MKeyBoardInputEvent* pKeyInput = dynamic_cast<MKeyBoardInputEvent*>(pEvent))
		{
			if (pKeyInput->GetKey() == 'W')
			{
				pCamera->m_bW = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'S')
			{
				pCamera->m_bS = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'A')
			{
				pCamera->m_bA = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'D')
			{
				pCamera->m_bD = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'Q')
			{
				pCamera->m_bQ = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'E')
			{
				pCamera->m_bE = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'X')
			{
				pCamera->m_bLB = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
		}

		else if (MMouseInputEvent* pMouseInput = dynamic_cast<MMouseInputEvent*>(pEvent))
		{
			Vector2 addi = pMouseInput->GetMouseAddition();

			pCamera->m_v2MouseAddi = addi;
		}

	};

	engine.GetInputManager()->AddListener(pListener);
	
	engine.SetRootNode(pRootNode);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
