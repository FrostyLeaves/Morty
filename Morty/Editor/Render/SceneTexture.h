#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MColor.h"

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

	void Initialize(MScene* pScene, const MString& strRenderProgram);
	void Release();

	void SetRect(Vector2i pos, Vector2i size);

	std::shared_ptr<MTexture> GetTexture();
	std::vector<std::shared_ptr<MTexture>> GetAllOutputTexture();
	void UpdateTexture(MIRenderCommand* pRenderCommand);

	MScene* GetScene() const { return m_pScene; }
	MViewport* GetViewport() const { return m_pRenderViewport; }

	void Snapshot(const MString& strSnapshotPath);

	MTaskNode* GetRenderTask() const { return m_pUpdateTask; }

	void SetPauseUpdate(bool bPause) { m_bPauseUpdate = bPause; }

	MIRenderProgram* GetRenderProgram() const { return m_pRenderProgram; }

protected:

	MScene* m_pScene = nullptr;

	MViewport* m_pRenderViewport = nullptr;

	MIRenderProgram* m_pRenderProgram = nullptr;

	bool m_bPauseUpdate = false;
	bool m_bSnapshot;
	MString m_strSnapshotPath;

	MTaskNode* m_pUpdateTask = nullptr;
};
