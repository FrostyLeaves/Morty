#ifndef _SCENE_TEXTURE_H_
#define _SCENE_TEXTURE_H_

#include "MGlobal.h"
#include "Vector.h"
#include "MColor.h"


#include <array>

class MScene;
class MEngine;
class MViewport;
class MTexture;
class MIRenderCommand;
class MIRenderProgram;
class MRenderTexture;
class MIRenderTexture;
class MForwardRenderProgram;
class SceneTexture
{
public:
	SceneTexture();
	virtual ~SceneTexture();

	void Initialize(MEngine* pEngine, const size_t& nImageCount);
	void Release();

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() const { return m_v2Size; }

	MTexture* GetTexture(const size_t& nImageIndex);
	MTexture* GetShadowmapTexture(const size_t& nImageIndex);
	void UpdateTexture(const size_t& nImageIndex, MIRenderCommand* pRenderCommand);

	MScene* GetScene() { return m_pScene; }
	MViewport* GetViewport() { return m_pRenderViewport; }

	void SetBackColor(const MColor& cColor);

protected:

	Vector2 m_v2Size;

	MScene* m_pScene;
	MEngine* m_pEngine;

	MViewport* m_pRenderViewport;

	std::vector<MForwardRenderProgram*> m_vRenderProgram;

	size_t m_nImageCount;
};


#endif

