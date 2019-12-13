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
#include "MIMeshInstance.h"
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
	MEngine engine;

	engine.Initialize();

	M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
	pRootNode->SetName("RootNode");


	MModelResource* pResource = dynamic_cast<MModelResource*>(engine.GetResourceManager()->Load("./Model/teaport.fbx"));
	MModelInstance* pSpatial = engine.GetObjectManager()->CreateObject<MySpatial>();
	pSpatial->Load(pResource);
	pSpatial->SetPosition(Vector3(0, 0, 0));
	pSpatial->SetName("Teaport");

	pRootNode->AddNode(pSpatial);

	
// 	if (MModelResource* pModelResource = pSpatial->GetResource())
// 	{
// 		if(pSpatial->SetPlayAnimation((*pModelResource->GetAnimationsName())[9]))
// 		{
// 			MIAnimController* pController = pSpatial->GetSkeletalAnimationController();
// 			pController->SetLoop(true);
// 			pController->Play();
// 		}	
// 	}

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

	engine.SetRootNode(pRootNode);
	
	while (engine.MainLoop());

	engine.Release();

	return 0;
}
