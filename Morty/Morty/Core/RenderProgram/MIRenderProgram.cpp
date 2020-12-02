#include "MIRenderProgram.h"
#include "MIRenderer.h"

M_I_OBJECT_IMPLEMENT(MIRenderProgram, MObject)

MIRenderProgram::MIRenderProgram()
	: m_pRenderTarget(nullptr)
{

}

void MIRenderProgram::BindRenderTarget(MIRenderTarget* pRenderTarget)
{
	m_pRenderTarget = pRenderTarget;
}
