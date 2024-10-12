#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MColor.h"

namespace morty
{

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

    void                                   Initialize(MScene* pScene, const MString& strRenderProgram);

    void                                   Release();

    void                                   SetRect(Vector2i pos, Vector2i size);

    std::shared_ptr<MTexture>              GetTexture();

    std::vector<std::shared_ptr<MTexture>> GetAllOutputTexture();

    void                                   UpdateTexture(MIRenderCommand* pRenderCommand);

    MScene*                                GetScene() const { return m_scene; }

    MViewport*                             GetViewport() const { return m_renderViewport; }

    void                                   Snapshot(const MString& strSnapshotPath);

    MTaskNode*                             GetRenderTask() const { return m_updateTask; }

    void                                   SetPauseUpdate(bool bPause) { m_pauseUpdate = bPause; }

    MIRenderProgram*                       GetRenderProgram() const { return m_renderProgram; }

protected:
    MScene*          m_scene = nullptr;

    MViewport*       m_renderViewport = nullptr;

    MIRenderProgram* m_renderProgram = nullptr;

    bool             m_pauseUpdate = false;
    bool             m_snapshot;
    MString          m_strSnapshotPath;

    MTaskNode*       m_updateTask = nullptr;
};

}// namespace morty