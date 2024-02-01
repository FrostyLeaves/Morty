/**
 * @File         MBlurRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderWork.h"

class MORTY_API MBlurRenderWork : public MBasicPostProcessRenderWork
{
	MORTY_CLASS(MBlurRenderWork)

	void InitDirection(bool bVertical) { m_bVertical = bVertical; }

	std::shared_ptr<MMaterial> CreateMaterial() override;

	void RenderSetup(const MRenderInfo& info) override;

	void RegisterSetting() override;

protected:

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

	bool m_bVertical = false;
};
