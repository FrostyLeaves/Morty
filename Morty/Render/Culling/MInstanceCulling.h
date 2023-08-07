#pragma once

#include "Material/MMaterial.h"
#include "Render/MBuffer.h"
#include "RenderProgram/MRenderInfo.h"


struct MMeshInstanceRenderProxy;
class MORTY_API IMeshInstanceFilter
{
public:
    virtual ~IMeshInstanceFilter() = default;
    virtual bool Filter(const MMeshInstanceRenderProxy* instance) const = 0;
};

class MORTY_API IMaterialFilter
{
public:

    virtual ~IMaterialFilter() = default;
    virtual bool Filter(const std::shared_ptr<MMaterial>& material) const = 0;
};

class MMaterialTypeFilter : public IMaterialFilter
{
public:
    MMaterialTypeFilter(MEMaterialType eMaterialType) : m_eMaterialType(eMaterialType) {}
    bool Filter(const std::shared_ptr<MMaterial>& material) const override;

private:
    MEMaterialType m_eMaterialType;
};

class MInstanceBatchGroup;
class MMaterialBatchGroup;
class MORTY_API MInstanceCulling
{
public:

    virtual void Initialize(MEngine* pEngine) {}
    virtual void Release() {}

    virtual void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) = 0;
    virtual const MBuffer* GetDrawIndirectBuffer() = 0;
    virtual const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const = 0;

};

class MORTY_API MCameraFrustumCulling : public MInstanceCulling
{
public:
    void SetCommand(MIRenderCommand* pCommand) { m_pCommand = pCommand; }
    void SetCameraFrustum(MCameraFrustum cameraFrustum) { m_cameraFrustum = cameraFrustum; }
    void SetCameraPosition(Vector3 v3CameraPosition) { m_v3CameraPosition = v3CameraPosition; }

protected:
    Vector3 m_v3CameraPosition;
    MCameraFrustum m_cameraFrustum;
    MIRenderCommand* m_pCommand = nullptr;
};
