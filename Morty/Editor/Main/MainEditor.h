#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "Variant/MVariant.h"
#include "Render/MRenderPass.h"
#include "Render/SceneTexture.h"
#include "Basic/MTexture.h"

#include "View/MRenderView.h"

class MNode;
class MScene;
class MViewport;
class ImGuiRenderable;
class IBaseView;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class ModelConvertView;
class MessageView;
class MInputEvent;
class MTaskNode;
class MainEditor : public MRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();

	MScene* GetScene() const { return m_SceneTexture.GetScene(); }
	MViewport* GetViewport() const { return m_SceneTexture.GetViewport(); }

	bool Initialize(MEngine* pEngine, const char* svWindowName);
	void Release();

	void OnResize(const int& nWidth, const int& nHeight);

	void Input(MInputEvent* pEvent);

	int GetViewWidth() { return m_v2DrawableSize.x; }
	int GetViewHeight() { return m_v2DrawableSize.y; }

    bool GetMinimized() { return m_bWindowMinimized; }

	bool MainLoop(MTaskNode* pNode);

	void Render(MTaskNode* pNode);

public:

	void SetCloseCallback(const std::function<bool()>& callback) { m_funcCloseCallback = callback; }

	static MString GetRenderProgramName() { return m_sRenderProgramName; }

public:

	void InitializeSDLWindow();

public:

	void ShowMenu();

	void ShowRenderView(const size_t& nImageCount);
	void ShowShadowMapView(const size_t& nImageCount);
	void ShowView(IBaseView* pView);
	void ShowGuizmo();

	void ShowDialog();

protected:

	Vector4 GetWidgetSize();

protected:

	NodeTreeView* m_pNodeTreeView;
	PropertyView* m_pPropertyView;
	MaterialView* m_pMaterialView;
	ResourceView* m_pResourceView;
	ModelConvertView* m_pModelConvertView;
	MessageView* m_pMessageView;

	std::vector<IBaseView*> m_vChildView;


    Vector2 m_v2DrawableSize;
    bool m_bWindowMinimized;
	bool m_bWindowResized = false;
    
	Vector2 m_v2RenderViewPos;
	Vector2 m_v2RenderViewSize;
	
	bool m_bRenderToWindow;
	bool m_bShowRenderView;
	bool m_bShowDebugView;
	
	SceneTexture m_SceneTexture;

	std::function<bool()> m_funcCloseCallback;

	struct SDL_Window* m_pSDLWindow;
	static MString m_sRenderProgramName;

	ImGuiRenderable* m_pImGuiRenderable;

};


#endif
