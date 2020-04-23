#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MWindowsRenderView.h"

class MNode;
class MScene;
class MStaticMeshInstance;
class NodeTreeView;
class PropertyView;
class MaterialView;
class MainEditor : public MWindowsRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();


	void SetEditorNode(MNode* pNode);
	MScene* GetScene() { return m_pScene; }

	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	virtual void OnResize(const int& nWidth, const int& nHeight) override;

	virtual void Input(MInputEvent* pEvent) override;

	virtual void OnRenderBegin() override;
	virtual void OnRenderEnd() override;

	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;

public:

	void ShowMenu();

	void ShowRenderView();
	void ShowNodeTree();
	void ShowProperty();
	void ShowMaterial();
	void ShowMessage();

public:
	virtual LRESULT CALLBACK ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

protected:

	MScene* m_pScene;
	NodeTreeView* m_pNodeTreeView;
	PropertyView* m_pPropertyView;
	MaterialView* m_pMaterialView;
	class MTextureRenderTarget* m_pTextureRenderTarget;
	class MViewport* m_pRenderViewport;

	Vector2 m_v2RenderViewPos;
	Vector2 m_v2RenderViewSize;

	unsigned int m_unTriangleCount;

	bool m_bShowMessage;
	bool m_bShowNodeTree;
	bool m_bShowProperty;
	bool m_bShowRenderView;
	bool m_bShowMaterial;
};


#endif