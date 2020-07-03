#include "MIRenderer.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MIRenderTarget.h"

MIRenderer::MIRenderer()
	: m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
{

}

