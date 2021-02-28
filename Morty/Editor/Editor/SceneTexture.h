#ifndef _SCENE_TEXTURE_H_
#define _SCENE_TEXTURE_H_

#include "MGlobal.h"
#include "Vector.h"
#include "Type/MColor.h"

#include <array>

class MScene;
class MEngine;
class MViewport;
class MRenderGraph;
class MIRenderProgram;
class MRenderTexture;
class MIRenderTexture;

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

	void SetRenderTextureName(const MString& strTextureName);
	void* GetTexture(const uint32_t& unFrameIndex);

	MScene* GetScene() { return m_pScene; }
	MViewport* GetViewport() { return m_pRenderViewport; }

	void SetBackColor(const MColor& cColor);

	MRenderGraph* GetRenderGraph();

protected:

	Vector2 m_v2Size;

	MScene* m_pScene;
	MEngine* m_pEngine;

	MViewport* m_pRenderViewport;


	MIRenderProgram* m_pRenderProgram;

};


#endif

