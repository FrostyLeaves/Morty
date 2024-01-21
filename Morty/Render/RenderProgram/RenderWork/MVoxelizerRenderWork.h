/**
 * @File         MVoxelizerRenderWork
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

class MORTY_API MVoxelizerRenderWork : public ISinglePassRenderWork
{
    MORTY_CLASS(MVoxelizerRenderWork)

    static const MStringId VoxelizerBufferOutput;

public:

    void Initialize(MEngine* pEngine) override;
	void Release() override;
    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;
    void Resize(Vector2i size) override;

    void Render(const MRenderInfo& info) override;
    void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    const std::unordered_map<MStringId, std::shared_ptr<MMaterial>>& GetVoxelizerMaterial() const { return m_tVoxelizerMaterial; }
    const MBuffer* GetVoxelTableBuffer() const;
    const MBuffer* GetVoxelDebugBuffer() const;
    std::shared_ptr<MTexture> GetVoxelGITexture() const;
    MBoundsAABB GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const;

    void SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx);

    MVoxelMapSetting GetVoxelSetting() const { return m_voxelSetting; }

protected:

    void InitializeBuffer();
    void ReleaseBuffer();

    void InitializeDispatcher();
    void ReleaseDispatcher();

    void InitializeVoxelTextureDispatcher();
    void ReleaseVoxelTextureDispatcher();

    void InitializeRenderPass();


    void BindTarget() override;

    std::vector<MStringId> GetInputName() override;

    std::vector<MRenderTaskOutputDesc> GetOutputName() override;


    MVoxelMapSetting m_voxelSetting;
    std::shared_ptr<IShaderPropertyUpdateDecorator> m_pFramePropertyUpdateDecorator = nullptr;

    MComputeDispatcher* m_pVoxelTextureGenerator = nullptr;
    std::shared_ptr<MShaderConstantParam> m_pVoxelizerVoxelMapSetting = nullptr;
    std::unordered_map<MStringId, std::shared_ptr<MMaterial>> m_tVoxelizerMaterial = {};
    MBuffer m_voxelizerBuffer;

    std::shared_ptr<MTexture> m_voxelGITexture = nullptr;
};
