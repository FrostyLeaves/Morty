#include "MIRenderer.h"

MIRenderer::MIRenderer()
	: m_eRasterizerType(MERasterizerType::ESolid | MERasterizerType::ECullBack)
	, m_eBlendType(MEBlendType::ENormal)
{

}
