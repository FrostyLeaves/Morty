// MortyDemo.cpp : 瀹氫箟鎺у埗鍙板簲鐢ㄧ▼搴忕殑鍏ュ彛鐐广€?
//

//#include <vld.h>

#include "stdafx.h"
#include "MEngine.h"

#include "MObject.h"
#include "MMaterial.h"
#include "MSpatial.h"
#include "MResourceManager.h"
#include "MVariable.h"
#include "MMaterialResource.h"

#include "MShader.h"
#include "MVertex.h"
#include "MIRenderer.h"
#include "MMeshInstance.h"
#include "MInputManager.h"
#include "MLogManager.h"
#include "MIRenderView.h"
#include "MCamera.h"


#include "Quaternion.h"


class MySpatial : public MSpatial
{
public:

	MySpatial(){
		m_fTime = 0.0f;
		m_color = Vector4(0.5, 0.5, 0.5, 1);
	}


	virtual void OnTick(const float& fDelta)
	{
		m_fTime += fDelta;

		Quaternion quat = this->GetRotation();
		Quaternion quatb(Vector3(1, 0, 0), 2.5f * fDelta);
		this->SetRotation(quat * quatb);

		for (MNode* pChild : this->GetChildren())
		{
			MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
			MMaterial* pMaterial = pMeshIns->GetMaterial();

			std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();
			for (MShaderParam& param : vParams)
			{
				if (param.strName == "cbPerObject")
				{
					param.var.GetStruct()->SetMember("TestColor", m_color);
				}
			}
		}

	}

private:

	Vector4 m_color;
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


//		MLogManager::GetInstance()->Log("Pos: %f,%f,%f", this->GetPosition().x, this->GetPosition().y, this->GetPosition().z);
	}


	bool m_bW;
	bool m_bS;
	bool m_bA;
	bool m_bD;
};

int main(int argc, char* argv[])
{

	MEngine engine;

	engine.Initialize();

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();

	
	MResource* pVSResource = engine.GetResourceManager()->Load("./Shader/defaultv.mvs");
	MResource* pPSResource = engine.GetResourceManager()->Load("./Shader/defaultp.mps");
	MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(engine.GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
	pMaterialRes->LoadVertexShader(pVSResource);
	pMaterialRes->LoadPixelShader(pPSResource);

	MMaterial* pMaterial = engine.GetObjectManager()->CreateObject<MMaterial>();
	pMaterial->Load(pMaterialRes);

	std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();

	for (MShaderParam& param : vParams)
	{
		if (param.strName == "cbPerObject")
		{
			param.var.GetStruct()->SetMember("testColor", Vector4(1, 1, 1, 1));
		}
	}

	MResource* pResource = engine.GetResourceManager()->Load("./Model/cat.fbx");
	MSpatial* pSpatial = engine.GetObjectManager()->CreateObject<MySpatial>();
	pSpatial->Load(pResource);
	pSpatial->SetPosition(Vector3(0, 0, 600));
	pSpatial->SetRotation(Quaternion(Vector3(1, 0, 0), 1));
	for (MNode* pChild : pSpatial->GetChildren())
	{
		MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
		pMeshIns->SetMaterial(pMaterial);
	}

	pRootNode->AddNode(pSpatial);


	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();

	pRootNode->AddNode(pCamera);

	engine.CreateView()->SetCamera(pCamera);


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
		}

	};

	engine.GetInputManager()->AddListener(pListener);
	
	engine.SetRootNode(pRootNode);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
