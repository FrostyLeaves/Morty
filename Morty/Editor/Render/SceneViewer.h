#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/Vector.h"
#include "Object/MObject.h"
#include "Utility/MColor.h"


namespace morty
{

class MTaskNode;
class MScene;
class MEngine;
class MViewport;
class MTexture;
class IRenderCommand;
class MIRenderProgram;
class SceneViewer : public MObject
{
    MORTY_CLASS(SceneViewer)
public:
    void                      Initialize(const MString& viewName, MScene* scene, const MStringId& strRenderProgram);
    void                      OnDelete() override;

    void                      Tick(float fDelta);
    void                      Render(IRenderCommand* pRenderCommand);
    void                      SetRect(Vector2i pos, Vector2i size);
    void                      SetPauseUpdate(bool bPause) { m_pauseUpdate = bPause; }

    [[nodiscard]] MTexturePtr GetFinalOutputTexture() const;

    [[nodiscard]] MScene*     GetScene() const { return m_scene; }
    [[nodiscard]] MViewport*  GetViewport() const { return m_renderViewport; }
    [[nodiscard]] MTaskNode*  GetRenderTask() const { return m_updateTask; }
    [[nodiscard]] MIRenderProgram* GetRenderProgram() const { return m_renderProgram; }

protected:
    MScene*          m_scene          = nullptr;
    MViewport*       m_renderViewport = nullptr;
    MIRenderProgram* m_renderProgram  = nullptr;
    bool             m_pauseUpdate    = false;
    MTaskNode*       m_updateTask     = nullptr;

    static MString   m_defaultRenderGraphPath;
};

}// namespace morty