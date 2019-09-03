#include "MIRenderView.h"

MIRenderView::MIRenderView()
	: m_pRootNode(nullptr)
	, m_pCamera(nullptr)
{

}

MIRenderView::~MIRenderView()
{

}

void MIRenderView::SetRootNode(MNode* pNode)
{
	m_pRootNode = pNode;
}

void MIRenderView::SetCamera(MCamera* pCamera)
{
	m_pCamera = pCamera;
}
