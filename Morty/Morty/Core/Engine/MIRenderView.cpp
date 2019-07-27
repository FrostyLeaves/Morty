#include "MIRenderView.h"

MIRenderView::MIRenderView()
	: m_pRootNode(nullptr)
{

}

MIRenderView::~MIRenderView()
{

}

void MIRenderView::SetRootNode(MNode* pNode)
{
	m_pRootNode = pNode;
}
