#ifndef _SCENE_TEXTURE_H_
#define _SCENE_TEXTURE_H_

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MColor.h"


#include <array>

class MTaskNode;
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

	void Initialize(MEngine* pEngine, const MString& strRenderProgram, const size_t& nImageCount);
	void Release();

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() const { return m_v2Size; }

	std::shared_ptr<MTexture> GetTexture(const size_t& nImageIndex);
	std::vector<std::shared_ptr<MTexture>> GetAllOutputTexture(const size_t& nImageIndex);
	void UpdateTexture(const size_t& nImageIndex, MIRenderCommand* pRenderCommand);

	MScene* GetScene() const { return m_pScene; }
	MViewport* GetViewport() const { return m_pRenderViewport; }

	void SetBackColor(const MColor& cColor);

	void Snapshot(const MString& strSnapshotPath);

	MTaskNode* GetUpdateTask() const { return m_pUpdateTask; }

protected:

	Vector2 m_v2Size;

	MScene* m_pScene;
	MEngine* m_pEngine;

	MViewport* m_pRenderViewport;

	std::vector<MIRenderProgram*> m_vRenderProgram;

	size_t m_nImageCount;

	bool m_bSnapshot;
	MString m_strSnapshotPath;

	MTaskNode* m_pUpdateTask = nullptr;
};


#endif

