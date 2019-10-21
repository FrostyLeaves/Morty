#include "MIRenderView.h"
#include "MIViewport.h"

MIRenderView::MIRenderView()
	: m_pEngine(nullptr)
	, m_pViewport(nullptr)
{

}

MIRenderView::~MIRenderView()
{

}

void MIRenderView::SetViewport(MIViewport* pViewport)
{
	if (m_pViewport = pViewport)
	{
		m_pViewport->SetSize(GetRenderRectSize());
	}
}
