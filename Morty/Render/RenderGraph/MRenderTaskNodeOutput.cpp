#include "MRenderTaskNodeOutput.h"

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

MRenderTaskNodeOutput::MRenderTaskNodeOutput()
	: MTaskNodeOutput()
	, m_bClear(true)
	, m_clearColor(MColor::Black_T)
{

}

MRenderTaskNodeOutput::~MRenderTaskNodeOutput()
{

}
