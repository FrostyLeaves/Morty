#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Object/MObject.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "Utility/MRenderGraphName.h"

namespace morty
{

class MBuffer;
class MTexture;
class MMaterial;
class MRenderPass;
class MIRenderCommand;
class MComputeDispatcher;
class MShaderPropertyBlock;
struct MMeshInstanceRenderProxy;

class IShaderPropertyUpdateDecorator;
class MORTY_API IGetTextureAdapter
{
public:
    virtual ~IGetTextureAdapter() = default;

    virtual MTexturePtr GetTexture() = 0;
};

class MORTY_API IPropertyBlockAdapter
{
public:
    virtual ~IPropertyBlockAdapter() = default;

    [[nodiscard]] virtual std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const = 0;
};

class MORTY_API MMeshBufferAdapter
{
public:
    virtual ~MMeshBufferAdapter() = default;

    [[nodiscard]] virtual const MBuffer* GetVertexBuffer() const = 0;
    [[nodiscard]] virtual const MBuffer* GetIndexBuffer() const  = 0;
};

class MORTY_API IRenderPassAdapter
{
public:
    virtual ~IRenderPassAdapter() = default;

    virtual MRenderPass* GetRenderPass() = 0;
};

class MORTY_API IGBufferAdapter
{
public:
    virtual ~IGBufferAdapter() = default;

    [[nodiscard]] virtual MTextureArray GetBackTextures() const = 0;
    [[nodiscard]] virtual MTexturePtr   GetDepthTexture() const = 0;
};

class MORTY_API IRenderable
{
public:
    virtual ~IRenderable() = default;

    virtual void Render(MIRenderCommand* pCommand) = 0;
};

class MORTY_API IRenderableFilter
{
public:
    virtual ~IRenderableFilter() = default;

    virtual bool Filter(const MMeshInstanceRenderProxy* instance) const = 0;
};

class MORTY_API IDrawIndirectAdapter
{
public:
    virtual ~IDrawIndirectAdapter() = default;

    [[nodiscard]] virtual std::shared_ptr<MMaterial> GetMaterial() const           = 0;
    [[nodiscard]] virtual const MBuffer*             GetDrawIndirectBuffer() const = 0;
    [[nodiscard]] virtual size_t                     GetOffset() const { return 0; }
    [[nodiscard]] virtual size_t                     GetCount() const = 0;
};

class MORTY_API ICullingDispatchAdapter
{
public:
    virtual ~ICullingDispatchAdapter() = default;

    [[nodiscard]] virtual MComputeDispatcher*   GetCullingDispatcher() const = 0;
    [[nodiscard]] virtual std::array<size_t, 3> GetComputeGroup() const      = 0;
};

class MORTY_API IComputeDispatcherAdapter
{
public:
    virtual ~IComputeDispatcherAdapter() = default;

    [[nodiscard]] virtual MComputeDispatcher*         GetComputeDispatcher() const = 0;
    [[nodiscard]] virtual std::array<size_t, 3>       GetComputeGroup() const      = 0;
    [[nodiscard]] virtual std::vector<const MBuffer*> GetBarrierBuffer() const     = 0;
};

class MORTY_API IShaderPropertyUpdateDecorator
{
public:
    virtual ~IShaderPropertyUpdateDecorator() = default;

    virtual void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) = 0;
    virtual void Update(const MRenderInfo& info)                                                 = 0;
};

class MGetTextureAdapter : public IGetTextureAdapter
{
public:
    explicit MGetTextureAdapter(const MTexturePtr& tex)
        : pTexture(tex)
    {}

    MTexturePtr GetTexture() override { return pTexture; }
    MTexturePtr pTexture = nullptr;
};

}// namespace morty