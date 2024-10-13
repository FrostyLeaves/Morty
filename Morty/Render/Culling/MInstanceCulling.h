#pragma once

#include "Utility/MGlobal.h"
#include "Material/MMaterial.h"
#include "Render/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MFunction.h"

namespace morty
{

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
    MMaterialTypeFilter(MEMaterialType eMaterialType)
        : m_materialType(eMaterialType)
    {}

    bool Filter(const std::shared_ptr<MMaterial>& material) const override;

private:
    MEMaterialType m_materialType;
};

class MMaterialMacroDefineFilter : public IMaterialFilter
{
public:
    MMaterialMacroDefineFilter(const std::unordered_map<MStringId, bool>& definedMacro)
        : m_definedMacro(definedMacro)
    {}

    bool Filter(const std::shared_ptr<MMaterial>& material) const override;

private:
    std::unordered_map<MStringId, bool> m_definedMacro;
};

class MInstanceBatchGroup;
class MMaterialBatchGroup;
class MORTY_API MInstanceCulling
{
public:
    virtual ~MInstanceCulling() = default;

    virtual void           Initialize(MEngine* pEngine) { MORTY_UNUSED(pEngine); }

    virtual void           Release() {}

    virtual void           Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) = 0;

    virtual void           UploadBuffer(MIRenderCommand* pCommand) = 0;

    virtual const MBuffer* GetDrawIndirectBuffer() = 0;

    virtual const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const = 0;
};

class MORTY_API MCameraFrustumCulling : public MInstanceCulling
{
public:
    void SetCommand(MIRenderCommand* pCommand) { m_command = pCommand; }

    void SetCameraFrustum(MCameraFrustum cameraFrustum) { m_cameraFrustum = cameraFrustum; }

    void SetCameraPosition(Vector3 v3CameraPosition) { m_cameraPosition = v3CameraPosition; }

protected:
    Vector3          m_cameraPosition;
    MCameraFrustum   m_cameraFrustum;
    MIRenderCommand* m_command = nullptr;
};

template<typename TYPE> class MORTY_API MCullingTaskNode : public MTaskNode
{
public:
    void OnCreated() override { BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MCullingTaskNode<TYPE>::Culling, this)); }

    void Initialize(MEngine* pEngine)
    {
        m_culling = std::make_shared<TYPE>();
        m_culling->Initialize(pEngine);
    }

    void Release()
    {
        m_culling->Release();
        m_culling = nullptr;
    }

    void SetInput(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) { m_instanceGroup = vInstanceGroup; }

    const std::shared_ptr<TYPE>& Get() const { return m_culling; }

    void                         Culling(MTaskNode* pNode)
    {
        MORTY_UNUSED(pNode);
        m_culling->Culling(m_instanceGroup);
    }

private:
    std::vector<MMaterialBatchGroup*> m_instanceGroup;

    std::shared_ptr<TYPE>             m_culling;
};

}// namespace morty