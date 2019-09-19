#include "MIScene.h"
#include "MEngine.h"
#include "MCamera.h"


MIScene::MIScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pUsingCamera(nullptr)
	, m_pDefaultCamera(nullptr)
{
	
}

void MIScene::OnCreated()
{
	//Init Default Camera.
	m_pDefaultCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();
}

void MIScene::OnAddNode(MNode* pNode)
{
	

}

MIScene::~MIScene()
{

}

void MIScene::SetRootNode(MNode* pRootNode)
{
	m_pRootNode = pRootNode;
}

MCamera* MIScene::GetCamera()
{
	if (m_pUsingCamera)
		return m_pUsingCamera;
	return m_pDefaultCamera;
}
