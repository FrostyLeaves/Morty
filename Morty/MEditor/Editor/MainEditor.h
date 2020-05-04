#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MVariant.h"
#include "SceneTexture.h"
#include "MWindowsRenderView.h"

class MNode;
class MScene;
class MStaticMeshInstance;
class IBaseView;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class MainEditor : public MWindowsRenderView
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

	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;


public:

	void Notify_Edit_Material(const MVariant& var);


public:

	void ShowMenu();

	void ShowRenderView();
	void ShowNodeTree();
	void ShowProperty();
	void ShowMaterial();
	void ShowMessage();
	void ShowResource();

public:
	virtual LRESULT CALLBACK ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

protected:

	NodeTreeView* m_pNodeTreeView;
	PropertyView* m_pPropertyView;
	MaterialView* m_pMaterialView;
	ResourceView* m_pResourceView;

	std::vector<IBaseView*> m_vChildView;

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
};


#endif