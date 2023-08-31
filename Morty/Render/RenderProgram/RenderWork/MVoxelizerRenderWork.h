/**
 * @File         MVoxelizerRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"



class MORTY_API MVoxelizerRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MVoxelizerRenderWork)

public:

    void Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);


    static constexpr size_t VoxelTableSize = 2048;
};