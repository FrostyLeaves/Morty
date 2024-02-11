#pragma once

#include "Material/MMaterial.h"
#include "RenderProgram/MRenderInfo.h"
#include "Utility/MGlobal.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskNode.h"

MORTY_SPACE_BEGIN

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

class MMaterialMacroDefineFilter : public IMaterialFilter
{
public:
    MMaterialMacroDefineFilter(const std::unordered_map<MStringId, bool>& definedMacro) : m_definedMacro(definedMacro) {}
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

    virtual void Initialize(MEngine* pEngine) {MORTY_UNUSED(pEngine); }
    virtual void Release() {}

    virtual void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) = 0;
    virtual void UploadBuffer(MIRenderCommand* pCommand) = 0;
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

template<typename TYPE>
class MORTY_API MCullingTaskNode : public MTaskNode
{
public:

    void OnCreated() override
    {
        BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MCullingTaskNode<TYPE>::Culling, this));
    }

    void Initialize(MEngine* pEngine)
    {
        m_pCulling = std::make_shared<TYPE>();
        m_pCulling->Initialize(pEngine);
    }

    void Release()
    {
        m_pCulling->Release();
        m_pCulling = nullptr;
    }

    void SetInput(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) { m_vInstanceGroup = vInstanceGroup; }

    const std::shared_ptr<TYPE>& Get() const { return m_pCulling; }

    void Culling(MTaskNode* pNode)
    {
        MORTY_UNUSED(pNode);
        m_pCulling->Culling(m_vInstanceGroup);
    }

private:

    std::vector<MMaterialBatchGroup*> m_vInstanceGroup;

    std::shared_ptr<TYPE> m_pCulling;
};

MORTY_SPACE_END