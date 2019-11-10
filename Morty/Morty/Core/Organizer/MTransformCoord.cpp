#include "MTransformCoord.h"
#include "M3DNode.h"

#include "MEngine.h"
#include "MIRenderer.h"
#include "MPainter.h"
#include "MMaterial.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"

#include "MInputManager.h"

MTransformCoord3D::MTransformCoord3D()
	: MITransformCoord()
	, m_pTargetNode(nullptr)
{

}

MTransformCoord3D::~MTransformCoord3D()
{

}

void MTransformCoord3D::SetTarget3DNode(MNode* pNode)
{
	m_pTargetNode = dynamic_cast<M3DNode*>(pNode);
}

bool MTransformCoord3D::Input(MInputEvent* pEvent, MIViewport* pViewport)
{
	MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent);
	if (nullptr == pMouseEvent)
		return false;




	return false;
}

void MTransformCoord3D::Render(MIRenderer* pRenderer, MIViewport* pViewport)
{
	if (nullptr == m_pTargetNode)
		return;

	static MMaterial* TestMaterial = nullptr;
	if (nullptr == TestMaterial)
	{
		TestMaterial = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
		MResource* pVSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mvs");
		MResource* pPSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mps");
		MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(m_pEngine->GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
		pMaterialRes->LoadVertexShader(pVSResource);
		pMaterialRes->LoadPixelShader(pPSResource);

		TestMaterial->Load(pMaterialRes);
	}

	pRenderer->SetUseMaterial(TestMaterial);
	pRenderer->UpdateMaterialParam();

	MPainter2DLine lines[] = {
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(0, 0, 0), m_pTargetNode->GetWorldTransform() * Vector3(10, 0, 0), MColor(1, 0, 0, 1), 2.0f),
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(0, 0, 0), m_pTargetNode->GetWorldTransform() * Vector3(0, 10, 0), MColor(1, 0, 0, 1), 2.0f),
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(0, 0, 0), m_pTargetNode->GetWorldTransform() * Vector3(0, 0, 10), MColor(1, 0, 0, 1), 2.0f),
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(10, 10, 10), m_pTargetNode->GetWorldTransform() * Vector3(0, 10, 10), MColor(0, 1, 0, 1), 2.0f),
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(10, 10, 10), m_pTargetNode->GetWorldTransform() * Vector3(10, 0, 10), MColor(0, 1, 0, 1), 2.0f),
	MPainter2DLine(m_pTargetNode->GetWorldTransform() * Vector3(10, 10, 10), m_pTargetNode->GetWorldTransform() * Vector3(10, 10, 0), MColor(0, 1, 0, 1), 2.0f),
	};


	for (int i = 0; i < 6; ++i)
	{
		if (MIMesh* pMesh = lines[i].FillData(pViewport))
		{
			pRenderer->DrawMesh(pMesh);
			pMesh->DestroyBuffer(m_pEngine->GetDevice());
			delete pMesh;
		}
	}
}
