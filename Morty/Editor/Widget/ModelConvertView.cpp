#include "ModelConvertView.h"

#include "Utility/MGlobal.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileDialog.h"

#include "Engine/MEngine.h"
#include "Thread/MThreadPool.h"
#include "Model/MModelConverter.h"

#include "Property/PropertyBase.h"

static const char* svModelFilter = ".fbx,.obj,.blend\0\0";

const char* svImportModelID = "Import Model File";
const char* svOutputFolderID = "Output Model Entity Dir";

ModelConvertView::ModelConvertView()
	: BaseWidget()
{
	m_strViewName = "ModelConvert";
}

void ModelConvertView::Render()
{
	PropertyBase prop;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();

	prop.ShowValueBegin("Source File");
	float fWidth = ImGui::GetContentRegionAvail().x;
	if (ImGui::Button(m_strSourcePath.c_str(), ImVec2(fWidth, 0)))
	{
		ImGuiFileDialog::Instance()->OpenModal(svImportModelID, "Import", svModelFilter, "");
	}
	prop.ShowValueEnd();

	prop.ShowValueBegin("Output Dir");
	if (ImGui::Button(m_strOutputDir.c_str(), ImVec2(fWidth, 0)))
	{
		ImGuiFileDialog::Instance()->OpenModal(svOutputFolderID, "Output", "", "");
	}
	prop.ShowValueEnd();

	prop.ShowValueBegin("Output Name");
	prop.EditMString(m_strOutputName);
	prop.ShowValueEnd();

	prop.ShowValueBegin("Material Type");
	prop.EditEnum({ "Forward Basic", "Deferred PBR" }, m_nMaterialTypeEnum);
	prop.ShowValueEnd();

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();


	{
		float fWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button("Add", ImVec2(fWidth, 0.0f)))
		{
			if (!m_strSourcePath.empty() && !m_strOutputDir.empty() && !m_strOutputName.empty())
			{
				MModelConvertInfo info;
				info.strResourcePath = m_strSourcePath;
				info.strOutputDir = m_strOutputDir;
				info.strOutputName = m_strOutputName;
				info.eMaterialType = m_nMaterialTypeEnum == 0 ? MModelConvertMaterialType::E_Default_Forward : MModelConvertMaterialType::E_PBR_Deferred;

				m_convertQueue.push(info);
			}
		}
	}

	if (ImGuiFileDialog::Instance()->Display(svImportModelID))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();

			for (auto pr : files)
			{
				m_strSourcePath = pr.second;
				break;
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}


	if (ImGuiFileDialog::Instance()->Display(svOutputFolderID))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			m_strOutputDir = ImGuiFileDialog::Instance()->GetFilePathName();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	if (!m_convertQueue.empty())
	{
		std::queue<MModelConvertInfo> convertQueue = m_convertQueue;
		m_convertQueue = {};

		MThreadWork work;
		work.eThreadType = METhreadType::ECurrentThread;
		work.funcWorkFunction = std::bind(&ModelConvertView::Convert, this, convertQueue);

		GetEngine()->GetThreadPool()->AddWork(work);
	}
}

void ModelConvertView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);
}

void ModelConvertView::Release()
{

}

void ModelConvertView::Convert(std::queue<MModelConvertInfo> convertQueue)
{
	while (!convertQueue.empty())
	{
		MModelConverter converter(GetEngine());
		converter.Convert(convertQueue.front());
		convertQueue.pop();
	}
}
