#ifndef _SCENE_TEXTURE_H_
#define _SCENE_TEXTURE_H_

#include "MGlobal.h"
#include "Vector.h"
#include "Type/MColor.h"

#include <array>

class MScene;
class MEngine;
class MViewport;
class MIRenderProgram;
class MRenderBackTexture;
class MIRenderBackTexture;
class MRenderDepthTexture;
class MTextureRenderTarget;

class SceneTexture
{
public:
	SceneTexture();
	virtual ~SceneTexture();

	void Initialize(MEngine* pEngine);
	void Release();

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() const { return m_v2Size; }

	void UpdateTexture();
	void* GetTexture(const uint32_t& unFrameIndex);

	MScene* GetScene() { return m_pScene; }
	MViewport* GetViewport() { return m_pRenderViewport; }

	void SetBackColor(const MColor& cColor);

protected:

	Vector2 m_v2Size;

	MScene* m_pScene;
	MEngine* m_pEngine;

	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_vBackTexture;
	std::array<MRenderDepthTexture*, M_BUFFER_NUM> m_vDepthTexture;

	MTextureRenderTarget* m_pTextureRenderTarget;
	MViewport* m_pRenderViewport;


	MIRenderProgram* m_pRenderProgram;

};


#endif

