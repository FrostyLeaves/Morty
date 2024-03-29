/**
 * @File         MRenderWork
 * 
 * @Created      2021-08-16 11:50:15
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Math/Vector.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "Utility/MGlobal.h"

#include "Object/MObject.h"
#include "Render/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

MORTY_SPACE_BEGIN

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
	virtual std::shared_ptr<MTexture> GetTexture() = 0;
};

class MORTY_API IPropertyBlockAdapter
{
public:
    virtual ~IPropertyBlockAdapter() = default;
    virtual std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const = 0;
};

class MORTY_API MMeshBufferAdapter
{
public:
    virtual ~MMeshBufferAdapter() = default;
    virtual const MBuffer* GetVertexBuffer() const = 0;
    virtual const MBuffer* GetIndexBuffer() const = 0;
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
    virtual std::vector<std::shared_ptr<MTexture>> GetBackTextures() const = 0;
    virtual std::shared_ptr<MTexture> GetDepthTexture() const = 0;

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

    virtual std::shared_ptr<MMaterial> GetMaterial() const = 0;
    virtual const MBuffer* GetDrawIndirectBuffer() const = 0;
    virtual size_t GetOffset() const { return 0; }
    virtual size_t GetCount() const = 0;
};

class MORTY_API ICullingDispatchAdapter
{
public:
    virtual ~ICullingDispatchAdapter() = default;
    virtual MComputeDispatcher* GetCullingDispatcher() const = 0;
    virtual std::array<size_t, 3> GetComputeGroup() const = 0;

};

class MORTY_API IComputeDispatcherAdapter
{
public:
    virtual ~IComputeDispatcherAdapter() = default;
    virtual MComputeDispatcher* GetComputeDispatcher() const = 0;
    virtual std::array<size_t, 3> GetComputeGroup() const = 0;
    virtual std::vector<const MBuffer*> GetBarrierBuffer() const = 0;
};

class MORTY_API IShaderPropertyUpdateDecorator
{
public:
    virtual ~IShaderPropertyUpdateDecorator() = default;
    virtual void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) = 0;
    virtual void Update(const MRenderInfo& info) = 0;
};

class MGetTextureAdapter : public IGetTextureAdapter
{
public:

    MGetTextureAdapter(const std::shared_ptr<MTexture>& tex):pTexture(tex){}

    virtual std::shared_ptr<MTexture> GetTexture() { return pTexture; }

    std::shared_ptr<MTexture> pTexture = nullptr;
};

MORTY_SPACE_END