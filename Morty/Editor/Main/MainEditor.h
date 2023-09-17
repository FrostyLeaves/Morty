#pragma once

#include "Variant/MVariant.h"
#include "Render/SceneTexture.h"

#include "SDLRenderView.h"

class MainView;
class GuizmoWidget;
class MNode;
class MScene;
class MViewport;
class ImGuiRenderable;
class BaseWidget;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class ModelConvertView;
class MessageWidget;
class MInputEvent;
class MTaskNode;
class MainEditor : public RenderViewContent
{
public:

	MainEditor() = default;
	virtual ~MainEditor() = default;

	bool Initialize(MEngine* pEngine);
	void Release();

	MEngine* GetEngine() const { return m_pEngine; }
	void SetScene(MScene* pScene);
	MScene* GetScene() const { return m_pScene; }
	MViewport* GetViewport() const;
	std::shared_ptr<SceneTexture> GetSceneTexture() const { return m_pSceneTexture; }

	void OnResize(Vector2 size) override;
	void OnRender(MIRenderCommand* pRenderCommand) override;
	void OnInput(MInputEvent* pEvent) override;
	void OnTick(float fDelta) override;

	static MString GetRenderProgramName() { return m_sRenderProgramName; }
	MTaskNode* GetRenderTask() override { return m_pRenderTask; }

	std::shared_ptr<SceneTexture> CreateSceneViewer(MScene* pScene);
	void DestroySceneViewer(std::shared_ptr<SceneTexture> pViewer);

	Vector4 GetCurrentWidgetSize() const;

protected:

	void UpdateSceneViewer(MIRenderCommand* pRenderCommand);

	void ShowMenu();
	void ShowShadowMapView();
	void ShowView(BaseWidget* pView);
	void ShowDialog();

private:

	MEngine* m_pEngine = nullptr;
	MScene* m_pScene = nullptr;
    std::vector<BaseWidget*> m_vChildView;
	std::set<std::shared_ptr<SceneTexture>> m_vSceneViewer;

	bool m_bShowRenderView = false;
	bool m_bShowDebugView = false;

	Vector4 m_v4RenderViewSize= Vector4(0,0,32,32);
	
	std::shared_ptr<SceneTexture> m_pSceneTexture = nullptr;
	MTaskNode* m_pRenderTask = nullptr;

	static MString m_sRenderProgramName;
};
