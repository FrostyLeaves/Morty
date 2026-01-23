#pragma once

#include "SDLRenderView.h"

namespace morty
{

class MEngine;
class MScene;
class MTaskNode;
class SceneViewer;
class MInputEvent;
class IRenderCommand;

class SimpleSceneContent : public RenderViewContent
{
public:
    SimpleSceneContent() = default;

    virtual ~SimpleSceneContent() = default;

    bool       Initialize(MEngine* engine, MScene* scene);

    void       Release();

    void       OnRender(IRenderCommand* pRenderCommand) override;

    void       OnResize(Vector2 size) override;

    void       OnInput(MInputEvent* pEvent) override;

    void       OnTick(float delta) override;

    MTaskNode* GetRenderTask() override;

private:
    MEngine*     m_engine      = nullptr;
    MScene*      m_scene       = nullptr;
    SceneViewer* m_sceneViewer = nullptr;
    MTaskNode*   m_renderTask  = nullptr;
    Vector2      m_viewSize    = Vector2(1, 1);
};

}// namespace morty
