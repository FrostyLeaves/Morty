#include "MainEditor.h"

#include "Utility/MGlobal.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "imgui_impl_sdl.h"
#include "ImGuiFileDialog.h"

#include "SDL.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/MRenderGlobal.h"
#include "Render/Vulkan/MVulkanDevice.h"
#include <SDL_vulkan.h>
#endif

#include "Render/MMesh.h"
#include "Math/Matrix.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskGraph.h"
#include "Input/MInputEvent.h"
#include "Render/MRenderCommand.h"


#include "Widget/NodeTreeView.h"
#include "Widget/PropertyView.h"
#include "Widget/MaterialView.h"
#include "Widget/ResourceView.h"
#include "Widget/ModelConvertView.h"
#include "Widget/MessageWidget.h"


#include "Component/MRenderMeshComponent.h"

#include "RenderProgram/MDeferredRenderProgram.h"
#include "Utility/MTimer.h"
#include "Widget/GuizmoWidget.h"
#include "Widget/MainView.h"
#include "Widget/RenderSettingView.h"
#include "Widget/TaskGraphView.h"

MString MainEditor::m_sRenderProgramName = MDeferredRenderProgram::GetClassTypeName();

bool MainEditor::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();
	m_pRenderTask = pMainGraph->AddNode<MTaskNode>(MStringId("Editor_Render"));
	m_pRenderTask->SetThreadType(METhreadType::ERenderThread);

	m_vChildView.push_back(new NodeTreeView());
	m_vChildView.push_back(new PropertyView());
	m_vChildView.push_back(new MaterialView());
	m_vChildView.push_back(new ResourceView());
	m_vChildView.push_back(new ModelConvertView());
	m_vChildView.push_back(new MessageWidget());
	m_vChildView.push_back(new MainView());

	auto pTaskGraphView = new TaskGraphView("Task Graph");
	pTaskGraphView->SetTaskGraph(GetEngine()->GetMainGraph());
	m_vChildView.push_back(pTaskGraphView);


	m_pRenderGraphView = new TaskGraphView("Render Graph");
	m_vChildView.push_back(m_pRenderGraphView);

	m_pRenderSettingView = new RenderSettingView();
	m_vChildView.push_back(m_pRenderSettingView);


	for (BaseWidget* pChild : m_vChildView)
	{
		pChild->Initialize(this);
	}

	return true;
}

void MainEditor::Release()
{
	if (m_pSceneTexture)
	{
		DestroySceneViewer(m_pSceneTexture);
		m_pSceneTexture = nullptr;
	}

	for (BaseWidget* pChild : m_vChildView)
	{
		pChild->Release();
		delete pChild;
	}

	m_vChildView.clear();
}

MViewport* MainEditor::GetViewport() const
{
	return m_pSceneTexture->GetViewport();
}

void MainEditor::SetScene(MScene* pScene)
{
	if (m_pScene == pScene)
	{
		return;
	}

	m_pScene = pScene;

	if (m_pSceneTexture)
	{
		DestroySceneViewer(m_pSceneTexture);
		m_pSceneTexture = nullptr;
	}

	m_pSceneTexture = CreateSceneViewer(m_pScene);

	m_pRenderGraphView->SetTaskGraph(m_pSceneTexture->GetRenderProgram()->GetRenderGraph());
	m_pRenderSettingView->SetRenderGraph(m_pSceneTexture->GetRenderProgram()->GetRenderGraph()->DynamicCast<MRenderGraph>()->GetRenderGraphSetting());
}

void MainEditor::OnResize(Vector2 size)
{
	MORTY_UNUSED(size);
}

void MainEditor::OnInput(MInputEvent* pEvent)
{
	m_pSceneTexture->GetViewport()->Input(pEvent);

}

void MainEditor::OnTick(float fDelta)
{
	m_pScene->Tick(fDelta);
}

std::shared_ptr<SceneTexture> MainEditor::CreateSceneViewer(MScene* pScene)
{
	std::shared_ptr<SceneTexture> pSceneTexture = std::make_shared<SceneTexture>();
	pSceneTexture->Initialize(pScene, MainEditor::GetRenderProgramName());
	m_vSceneViewer.insert(pSceneTexture);

	pSceneTexture->GetRenderTask()->ConnectTo(GetRenderTask());
	return pSceneTexture;
}

void MainEditor::DestroySceneViewer(std::shared_ptr<SceneTexture> pViewer)
{
	pViewer->Release();
	m_vSceneViewer.erase(pViewer);
}

void MainEditor::UpdateSceneViewer(MIRenderCommand* pRenderCommand)
{
	std::vector<MTexture*> vRenderTextures;
	for (auto pSceneViewer : m_vSceneViewer)
	{
		pSceneViewer->UpdateTexture(pRenderCommand);

		if (std::shared_ptr<MTexture> pRenderTexture = pSceneViewer->GetTexture())
		{
			vRenderTextures.push_back(pRenderTexture.get());
		}
	}

	pRenderCommand->AddRenderToTextureBarrier(vRenderTextures, METextureBarrierStage::EPixelShaderSample);
}

void MainEditor::ShowMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", ""))
			{
				ImGuiFileDialog::Instance()->OpenModal("OpenFile", "Open", "entity\0\0", ".");
			}

			if (ImGui::MenuItem("Save", ""))
			{

			}

			if (ImGui::MenuItem("Save as", ""))
			{
				ImGuiFileDialog::Instance()->OpenModal("Save As", "Open", "entity\0\0", "new");
			}


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Render", "", &m_bShowRenderView)) {}
			if (ImGui::MenuItem("DebugTexture", "", &m_bShowDebugView)) {}

			for (BaseWidget* pView : m_vChildView)
			{
				bool bVisible = pView->GetVisible();
				if (ImGui::MenuItem(pView->GetName().c_str(), "", &bVisible)) {}
				pView->SetVisible(bVisible);
			}
			
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Convert model"))
			{

			}

			if (ImGui::MenuItem("Load model"))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tool"))
		{
			if (ImGui::MenuItem("Snip shot"))
			{
				auto t = std::time(nullptr);
				tm outtm;
				MORTY_ASSERT(0 == MTimer::LocalTime(t, outtm));
				std::ostringstream oss;
				oss << std::put_time(&outtm, "%d-%m-%Y %H-%M-%S");
			    auto str = oss.str();

				if (m_pSceneTexture)
				{
					m_pSceneTexture->Snapshot("./Snipshot-" + str + ".png");
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void MainEditor::ShowShadowMapView()
{
	if (!m_bShowDebugView)
		return;

	if (ImGui::Begin("DebugView", &m_bShowDebugView))
	{
		std::vector<std::shared_ptr<MTexture>> vTexture = m_pSceneTexture->GetAllOutputTexture();
		if(!vTexture.empty())
		{
			size_t nImageSize = 0;
			for (std::shared_ptr<MTexture> pTexture : vTexture)
			{
				if (pTexture)
				{
					nImageSize += pTexture->GetSize().z;
				}
			}
			
			// n * n
			size_t nRowCount = std::ceil(std::sqrt(nImageSize));

			Vector4 v4Rect = GetCurrentWidgetSize();

			Vector2 v2Size = Vector2((v4Rect.z) / nRowCount, (v4Rect.w) / nRowCount);

			ImGui::Columns(static_cast<int>(nRowCount));
			for (size_t nTexIdx = 0; nTexIdx < vTexture.size(); ++nTexIdx)
			{
				for (size_t nLayerIdx = 0; nLayerIdx < static_cast<size_t>(vTexture[nTexIdx]->GetSize().z); ++nLayerIdx)
				{
					ImGui::Image({ vTexture[nTexIdx], intptr_t(vTexture[nTexIdx].get()), nLayerIdx }, ImVec2(v2Size.x, v2Size.y));
					ImGui::Text("%s (%d)", vTexture[nTexIdx]->GetName().c_str(), static_cast<int>(nLayerIdx));

					ImGui::NextColumn();
				}
			}
			ImGui::Columns(1);
		
		}
	}

	ImGui::End();
}

void MainEditor::ShowView(BaseWidget* pView)
{
	bool bVisible = pView->GetVisible();

	if (bVisible)
	{
		if (ImGui::Begin(pView->GetName().c_str(), &bVisible))
		{
			pView->Render();
		}

		pView->SetVisible(bVisible);
		ImGui::End();
	}
	else if (pView->GetRenderInHidden())
	{
		pView->Render();
	}
}

void MainEditor::ShowDialog()
{
	if (ImGuiFileDialog::Instance()->Display("OpenFile"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();
			
		}
		ImGuiFileDialog::Instance()->Close();
	}


	if (ImGuiFileDialog::Instance()->Display("Save As"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string strFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string strCurrentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

		}
		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("Convert Model"))
	{

	}
}

Vector4 MainEditor::GetCurrentWidgetSize() const
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 v2RenderViewPos = ImGui::GetWindowPos();
	ImVec2 v2RenderViewSize = ImGui::GetWindowSize();

	v2RenderViewPos.x += style.WindowPadding.x;
	v2RenderViewPos.y += ImGui::GetItemRectSize().y;

	v2RenderViewSize.x -= style.WindowPadding.x * 2.0f;
	v2RenderViewSize.y -= (style.WindowPadding.y * 2.0f + ImGui::GetItemRectSize().y * 2.0f);

	return Vector4(v2RenderViewPos.x, v2RenderViewPos.y, v2RenderViewSize.x, v2RenderViewSize.y);
}

void MainEditor::OnRender(MIRenderCommand* pRenderCommand)
{
	if (m_pSceneTexture)
	{
//		m_pSceneTexture->SetRect(Vector2(m_v4RenderViewSize.x, m_v4RenderViewSize.y), Vector2(m_v4RenderViewSize.z, m_v4RenderViewSize.w));
	}

	//update all scene viewer.
	UpdateSceneViewer(pRenderCommand);

	ShowMenu();
	ShowShadowMapView();

	for (BaseWidget* pBaseView : m_vChildView)
	{
		ShowView(pBaseView);
	}

	ShowDialog();
}
