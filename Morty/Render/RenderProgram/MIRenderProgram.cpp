#include "RenderProgram/MIRenderProgram.h"

MORTY_INTERFACE_IMPLEMENT(MIRenderProgram, MObject)

MIRenderProgram::MIRenderProgram()
	: MObject()
	, m_pViewport(nullptr)
{

}

std::vector<MTexture*> MIRenderProgram::GetOutputTextures()
{
	if (MTexture* pTexture = GetOutputTexture())
	{
		return { pTexture };
	}

	return {};
}
