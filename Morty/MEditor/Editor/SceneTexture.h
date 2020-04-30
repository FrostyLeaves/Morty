#ifndef _SCENE_TEXTURE_H_
#define _SCENE_TEXTURE_H_

#include "MGlobal.h"
#include "Vector.h"

class MScene;
class MEngine;
class MViewport;
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
	void* GetTexture();

	MScene* GetScene() { return m_pScene; }
	MViewport* GetViewport() { return m_pRenderViewport; }

protected:

	Vector2 m_v2Size;

	MScene* m_pScene;
	MEngine* m_pEngine;

	MTextureRenderTarget* m_pTextureRenderTarget;
	MViewport* m_pRenderViewport;

};


#endif

