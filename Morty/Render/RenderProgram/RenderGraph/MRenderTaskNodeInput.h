/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Basic/MTexture.h"
#include "Render/MRenderPass.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "Utility/MGlobal.h"

#include "RenderProgram/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskTarget;

class MORTY_API MRenderTaskNodeInput : public MTaskNodeInput
{
	MORTY_CLASS(MRenderTaskNodeInput)
public:

	void SetName(const MStringId& name) { m_strOutputName = name; }
	MStringId GetName() const { return m_strOutputName; }

private:

	MStringId m_strOutputName;
};
