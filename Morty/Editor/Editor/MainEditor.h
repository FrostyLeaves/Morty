#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MVariant.h"
#include "MRenderPass.h"
#include "SceneTexture.h"
#include "MIRenderView.h"

class MNode;
class MScene;
class MStaticMeshInstance;
class IBaseView;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class MainEditor : public MIRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();


	void SetEditorNode(MNode* pNode);
	MScene* GetScene() { return m_SceneTexture.GetScene(); }

	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	virtual void OnResize(const int& nWidth, const int& nHeight) override;

	virtual void Input(MInputEvent* pEvent) override;

	virtual void OnRenderBegin() override;
	virtual void OnRenderEnd() override;

	virtual int GetViewWidth() override { return m_v2DrawableSize.x; }
	virtual int GetViewHeight() override { return m_v2DrawableSize.y; }

    virtual bool GetMinimized() override { return m_bWindowMinimized; }

	virtual bool MainLoop(const float& fDelta) override;

	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;

public:

	void SetCloseCallback(const std::function<bool()>& callback) { m_funcCloseCallback = callback; }

public:

	void Notify_Edit_Material(const MVariant& var);

	void InitializeSDLWindow();

public:

	void ShowMenu();

	void ShowRenderView();
	void ShowNodeTree();
	void ShowProperty();
	void ShowMaterial();
	void ShowMessage();
	void ShowResource();

protected:

	NodeTreeView* m_pNodeTreeView;
	PropertyView* m_pPropertyView;
	MaterialView* m_pMaterialView;
	ResourceView* m_pResourceView;

	std::vector<IBaseView*> m_vChildView;


	Vector2 m_v2WindowSize;
    Vector2 m_v2DrawableSize;
    bool m_bWindowMinimized;
    
	Vector2 m_v2RenderViewPos;
	Vector2 m_v2RenderViewSize;

	unsigned int m_unTriangleCount;

	bool m_bShowMessage;
	bool m_bShowNodeTree;
	bool m_bShowProperty;
	bool m_bShowRenderView;
	bool m_bShowMaterial;
	bool m_bShowResource;


	SceneTexture m_SceneTexture;

	MRenderPass m_ImguiRenderPass;

	std::function<bool()> m_funcCloseCallback;

	struct SDL_Window* m_pSDLWindow;
};


#endif
