#ifndef _M_INSTANCE_CULLING_H_
#define _M_INSTANCE_CULLING_H_


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

class CameraFrustumCulling : public IMeshInstanceFilter
{
public:
    void UpdateCameraFrustum(Matrix4 cameraInvProj);

    bool Filter(const MMeshInstanceRenderProxy* instance) const override;

private:

	MCameraFrustum m_cameraFrustum;
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



#endif