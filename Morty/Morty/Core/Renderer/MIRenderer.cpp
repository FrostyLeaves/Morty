#include "MIRenderer.h"

MIRenderer::MIRenderer()
	: m_pDefaultTexture(nullptr)
	, m_eRasterizerType(MERasterizerType::ESolid | MERasterizerType::ECullBack)
{

}
