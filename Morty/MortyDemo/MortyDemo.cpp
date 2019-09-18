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
#include "MTexture.h"
#include "MTextureResource.h"

#include "MShader.h"
#include "MVertex.h"
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
	}

	virtual void OnTick(const float& fDelta)
	{
		m_fTime += fDelta;

		Quaternion quat = Quaternion(Vector3(1, 0, 0), 3.14 / 2 * 3);
		Quaternion quatb(Vector3(0, 1, 0), 0.5f * m_fTime);
		this->SetRotation(quat * quatb);

		for (MNode* pChild : this->GetChildren())
		{
			MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pChild);
			MMaterial* pMaterial = pMeshIns->GetMaterial();

			std::vector<MShaderParam>& vPixParams = pMaterial->GetPixelShaderParams();
			for (MShaderParam& param : vPixParams)
			{
				if (param.strName == "cbLight")
				{
					param.var.GetByType<MStruct>()->SetMember("AmbientLightColor", Vector3(1, 1, 1));

					param.var.GetByType<MStruct>()->SetMember("DiffuseLightPos", Vector3(100, 100, 200));
					param.var.GetByType<MStruct>()->SetMember("DiffuseLightColor", Vector3(1, 1, 1));

					if(MCamera* pCamera = dynamic_cast<MCamera*>(GetRootNode()->FindFirstChildByName("Camera")))
					{
						param.var.GetByType<MStruct>()->SetMember("CameraWorldPos", pCamera->GetPosition());
					}
				}
			}
		}

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

//		MLogManager::GetInstance()->Log("Pos: %f,%f,%f", this->GetPosition().x, this->GetPosition().y, this->GetPosition().z);
	}


	bool m_bW;
	bool m_bS;
	bool m_bA;
	bool m_bD;
	bool m_bQ;
	bool m_bE;
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

			std::vector<MShaderTextureParam>& vTexParams = pMaterial->GetPixelTextureParams();
			for (MShaderTextureParam& param : vTexParams)
			{
				if (param.strName == "texture0")
				{
					param.pTexture = dynamic_cast<MTextureResource*>(pTexResource)->GetTextureTemplate();
				}
			}
		}

		std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();



		pMeshIns->SetMaterial(pMaterial);
	}

	/*
	MString vTexturePath[] = {
		"TX_CH_Main_Juliet_Hair_D_Cos22.png",
		"TX_CH_Main_Juliet_Skin_D_Cos22.png",
		"empty.png",
		"TX_CH_Main_Juliet_Cloth_D_Cos22.png",
		
	};

	
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

		if (MResource* pTexResource = engine.GetResourceManager()->Load("./Model/Juliet_Sea_Shell/" + vTexturePath[i]))
		{

			std::vector<MShaderTextureParam>& vTexParams = pMaterial->GetPixelTextureParams();
			for (MShaderTextureParam& param : vTexParams)
			{
				if (param.strName == "texture0")
				{
					param.pTexture = dynamic_cast<MTextureResource*>(pTexResource)->GetTextureTemplate();
				}
			}
		}

		std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();



		pMeshIns->SetMaterial(pMaterial);
	}

	*/

	pRootNode->AddNode(pSpatial);


	MyCamera* pCamera = engine.GetObjectManager()->CreateObject<MyCamera>();
	pCamera->SetName("Camera");
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
			if (pKeyInput->GetKey() == 'Q')
			{
				pCamera->m_bQ = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
			if (pKeyInput->GetKey() == 'E')
			{
				pCamera->m_bE = pKeyInput->GetType() == MKeyBoardInputEvent::KeyBoardDown;
			}
		}

	};

	engine.GetInputManager()->AddListener(pListener);
	
	engine.SetRootNode(pRootNode);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
