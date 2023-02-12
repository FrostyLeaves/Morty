#include "RenderProgram/MIRenderProgram.h"

MORTY_INTERFACE_IMPLEMENT(MIRenderProgram, MObject)

MIRenderProgram::MIRenderProgram()
	: MObject()
	, m_pViewport(nullptr)
{

}

std::vector<std::shared_ptr<MTexture>> MIRenderProgram::GetOutputTextures()
{
	if (std::shared_ptr<MTexture> pTexture = GetOutputTexture())
	{
		return { pTexture };
	}

	return {};
}
