#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MVariant.h"
#include "MRenderPass.h"
#include "SceneTexture.h"
#include "MTexture.h"

#include "MRenderView.h"

class MNode;
class MScene;
class ImGuiRenderable;
class MStaticMeshInstance;
class IBaseView;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class TaskGraphView;
class MInputEvent;
class MTaskNode;
class MainEditor : public MRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();

	MScene* GetScene() { return m_SceneTexture.GetScene(); }

	bool Initialize(MEngine* pEngine, const char* svWindowName);
	void Release();

	void OnResize(const int& nWidth, const int& nHeight);

	void Input(MInputEvent* pEvent);

	int GetViewWidth() { return m_v2DrawableSize.x; }
	int GetViewHeight() { return m_v2DrawableSize.y; }

    bool GetMinimized() { return m_bWindowMinimized; }

	bool MainLoop(MTaskNode* pNode);

	void Render(MTaskNode* pNode);
	void SceneRender(MTaskNode* pNode);

public:

	void SetCloseCallback(const std::function<bool()>& callback) { m_funcCloseCallback = callback; }

public:

	void InitializeSDLWindow();

public:

	void ShowMenu();

	void ShowRenderView(const size_t& nImageCount);
	void ShowShadowMapView(const size_t& nImageCount);
	void ShowNodeTree();
	void ShowProperty();
	void ShowMaterial();
	void ShowMessage();
	void ShowResource();
	void ShowRenderGraphView();

protected:

	Vector4 GetWidgetSize();

protected:

	NodeTreeView* m_pNodeTreeView;
	PropertyView* m_pPropertyView;
	MaterialView* m_pMaterialView;
	ResourceView* m_pResourceView;
	TaskGraphView* m_pTaskGraphView;

	std::vector<IBaseView*> m_vChildView;


    Vector2 m_v2DrawableSize;
    bool m_bWindowMinimized;
    
	Vector2 m_v2RenderViewPos;
	Vector2 m_v2RenderViewSize;

	unsigned int m_unTriangleCount;

	bool m_bShowMessage;
	bool m_bShowNodeTree;
	bool m_bShowProperty;
	bool m_bShowRenderView;
	bool m_bRenderToWindow;
	bool m_bShowMaterial;
	bool m_bShowResource;
	bool m_bShowRenderGraph;
	bool m_bShowShadowMap;

	SceneTexture m_SceneTexture;

	std::function<bool()> m_funcCloseCallback;

	struct SDL_Window* m_pSDLWindow;

	ImGuiRenderable* m_pImGuiRenderable;

};


#endif
