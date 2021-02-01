/**
 * @File         MRenderTarget
 * 
 * @Created      2019-12-30 11:43:46
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIRENDERTARGET_H_
#define _M_MIRENDERTARGET_H_
#include "MGlobal.h"
#include "MEngine.h"
#include "MIRenderer.h"
#include "Type/MColor.h"
#include "MIRenderProgram.h"

#include <array>
#include <vector>
#include <functional>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif

class MIDevice;
class MIRenderTexture;

class MORTY_API MIRenderTarget : public MObject
{
public:
	M_I_OBJECT(MIRenderTarget)

	MIRenderTarget();
	virtual ~MIRenderTarget() {}

	virtual void OnDelete() override;

public:

	virtual uint32_t GetFrameBufferIndex();

	virtual void Resize(const Vector2& v2Size) { m_v2Size = v2Size; }
	Vector2 GetSize() const { return m_v2Size; }

	template <typename Type>
	void RegisterRenderProgram();

	MIRenderProgram* GetRenderProgram() { return m_pRenderProgram; }

public:

	virtual void OnRenderBefore(MIRenderer* pRenderer) {}
	virtual void OnRenderAfter(MIRenderer* pRenderer) {}
	virtual void OnRender(MIRenderer* pRenderer) { if(m_funcRenderFunction) m_funcRenderFunction(pRenderer); }
	std::function<void(MIRenderer*)> m_funcRenderFunction;

public:

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() = 0;
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() = 0;
#elif RENDER_GRAPHICS == MORTY_VULKAN


	VkExtent2D m_VkExtend;
	VkFormat m_VkColorFormat;

	std::array<VkCommandBuffer, M_BUFFER_NUM> m_VkCommandBuffers;
	std::array<VkSemaphore, M_BUFFER_NUM> m_aVkRenderFinishedSemaphore;

	std::vector<VkSemaphore> m_vWaitSemaphoreBeforeSubmit;

#endif

	Vector2 m_v2Size;

	MIRenderProgram* m_pRenderProgram;
};

template <typename Type>
void MIRenderTarget::RegisterRenderProgram()
{
	if (MTypedClass::IsType<Type, MIRenderProgram>())
	{
		if (m_pRenderProgram)
		{
			m_pRenderProgram->DeleteLater();
			m_pRenderProgram = nullptr;
		}

		m_pRenderProgram = (Type*)(GetEngine()->GetObjectManager()->CreateObject<Type>());
	}
}

#endif
