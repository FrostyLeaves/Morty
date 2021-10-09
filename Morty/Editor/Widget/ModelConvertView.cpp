#include "ModelConvertView.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileDialog.h"

#include "PropertyBase.h"

static const char* svModelFilter = ".fbx\0.obj\0\0";

const char* svImportModelID = "Import Model File";
const char* svOutputFolderID = "Output Model Entity Dir";

ModelConvertView::ModelConvertView()
	: IBaseView()
	, m_convertQueue()
	, m_strSourcePath()
	, m_strOutputDir()
	, m_strOutputName()
	, m_nMaterialTypeEnum(0)
{

}

ModelConvertView::~ModelConvertView()
{

}

void ModelConvertView::Render()
{
	PropertyBase prop;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();

	prop.ShowValueBegin("Source File");
	if (ImGui::Button(m_strSourcePath.c_str()))
	{
		ImGuiFileDialog::Instance()->OpenModal(svImportModelID, "Import", svModelFilter, "");
	}
	prop.ShowValueEnd();

	prop.ShowValueBegin("Output Dir");
	if (ImGui::Button(m_strOutputDir.c_str()))
	{
		ImGuiFileDialog::Instance()->OpenModal(svOutputFolderID, "Import", "", "");
	}
	prop.ShowValueEnd();

	prop.ShowValueBegin("Output Name");
	ImGui::InputText("Output Name", &m_strOutputName);
	prop.ShowValueEnd();

	prop.ShowValueBegin("Material Type");
	prop.EditEnum({ "Forward Basic", "Deferred PBR" }, m_nMaterialTypeEnum);
	prop.ShowValueEnd();

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();


	ImGui::Button("Add");





	if (ImGuiFileDialog::Instance()->Display(svImportModelID))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();

			int a = 0;
			++a;
		}
		ImGuiFileDialog::Instance()->Close();
	}


	if (ImGuiFileDialog::Instance()->Display(svOutputFolderID))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string strFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string strCurrentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();


			int a = 0;
			++a;
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void ModelConvertView::Initialize(MEngine* pEngine)
{

}

void ModelConvertView::Release()
{

}
