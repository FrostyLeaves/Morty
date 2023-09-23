/**
 * @File         MRenderWork
 * 
 * @Created      2021-08-16 11:50:15
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Math/Vector.h"
#include "Utility/MGlobal.h"

#include "Object/MObject.h"
#include "RenderProgram/MRenderInfo.h"

class MComputeDispatcher;
class MIRenderCommand;
class MBuffer;
class MTexture;
class MMaterial;
class MRenderPass;
class MShaderPropertyBlock;
struct MMeshInstanceRenderProxy;

class MORTY_API ITextureInputAdapter
{
public:
	virtual ~ITextureInputAdapter() = default;
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


class MORTY_API IRenderWork : public MTypeClass
{
    MORTY_INTERFACE(IRenderWork)
public:

    virtual void Initialize(MEngine* pEngine) = 0;
    virtual void Release(MEngine* pEngine) = 0;
    virtual void Resize(Vector2 size) = 0;


};
