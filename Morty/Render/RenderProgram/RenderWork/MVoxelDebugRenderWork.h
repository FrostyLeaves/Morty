/**
 * @File         MVoxelDebugRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"
#include "Render/MRenderGlobal.h"
#include "MSinglePassRenderWork.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"
#include "Render/MBuffer.h"
#include <memory>


class IShaderPropertyUpdateDecorator;

class MORTY_API MVoxelDebugRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MVoxelDebugRenderWork)

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;
    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

    void Render(MRenderInfo& info, const MVoxelMapSetting& voxelSetting, const MBuffer* pVoxelizerBuffer, const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<MMaterial> GetVoxelDebugMaterial() const { return m_pVoxelDebugMaterial; }
    const MBuffer* GetVoxelDebugBuffer() const;
    std::shared_ptr<MTexture> GetVoxelGITexture() const;

protected:

    void InitializeBuffer();
    void ReleaseBuffer();

    void InitializeDispatcher();
    void ReleaseDispatcher();

    std::shared_ptr<IShaderPropertyUpdateDecorator> m_pFramePropertyUpdateDecorator = nullptr;

    MComputeDispatcher* m_pVoxelDebugIndirectGenerator = nullptr;
    std::shared_ptr<MMaterial> m_pVoxelDebugMaterial = nullptr;
    MBuffer m_drawIndirectBuffer;


    std::shared_ptr<MShaderConstantParam> m_pDebugVoxelMapSetting = nullptr;
    std::shared_ptr<MShaderStorageParam> m_pVoxelStorageBuffer = nullptr;


};
