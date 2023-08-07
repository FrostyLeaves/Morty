#pragma once

#include "Render/SceneTexture.h"
#include "View/MRenderView.h"

class ImGuiRenderable;
class MNode;
class MScene;
class MInputEvent;
class MTaskNode;
class MIRenderCommand;

class RenderViewContent
{
public:

	virtual void OnRender(MIRenderCommand* pRenderCommand) = 0;
	virtual void OnResize(Vector2 size) = 0;
	virtual void OnInput(MInputEvent* pEvent) = 0;
	virtual void OnTick(float fDelta) = 0;

	virtual MTaskNode* GetRenderTask() { return nullptr; }
};

class SDLRenderView : public MRenderView
{
public:

	SDLRenderView() = default;
	virtual ~SDLRenderView() = default;

	bool Initialize(MEngine* pEngine, const char* svWindowName);
	void Release();
	void BindSDLWindow();
	void UnbindSDLWindow();

	int GetViewWidth() const { return m_v2DrawableSize.x; }
	int GetViewHeight() const { return m_v2DrawableSize.y; }
    bool GetMinimized() const { return m_bWindowMinimized; }
	bool GetClosed() const { return m_bWindowClosed; }

	void Input(MInputEvent* pEvent);
	bool MainLoop(MTaskNode* pNode);
	void Render(MTaskNode* pNode);
	void Resize(const int& nWidth, const int& nHeight);

	void AppendContent(RenderViewContent* pContent);

protected:

	MTaskNode* GetEditorRenderTask() const { return m_pRenderTask; }


	
private:
	
    Vector2 m_v2DrawableSize = Vector2(80.0f, 48.0f);
    bool m_bWindowMinimized = false;
	bool m_bWindowResized = false;
	bool m_bWindowClosed = false;
	struct SDL_Window* m_pSDLWindow = nullptr;
	MTaskNode* m_pRenderTask = nullptr;
	ImGuiRenderable* m_pImGuiRender = nullptr;

	std::vector<RenderViewContent*> m_vContent;
};
