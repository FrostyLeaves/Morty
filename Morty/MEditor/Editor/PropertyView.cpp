#include "PropertyView.h"

#include "imgui.h"

#include "MObject.h"
#include "M3DNode.h"

PropertyView::PropertyView()
	: m_pEditorObject(nullptr)
	, m_unItemIDPool(0)
{

}

PropertyView::~PropertyView()
{

}

void PropertyView::SetEditorObject(MObject* pObject)
{
	m_pEditorObject = pObject;
}

void PropertyView::Render()
{
	if (nullptr == m_pEditorObject)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();
	
	unsigned int unID = 0;
	
	EditM3DNode(dynamic_cast<M3DNode*>(m_pEditorObject));

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();


}

bool PropertyView::ShowNodeBegin(const MString& strNodeName)
{
	ImGui::PushID(GetID(strNodeName));
	ImGui::AlignTextToFramePadding();  
	if (bool node_open = ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s", strNodeName.c_str()))
	{
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("my sailor is rich");
		ImGui::NextColumn();

		return true;
	}

	ImGui::PopID();
	return false;
}

void PropertyView::ShowNodeEnd()
{
	ImGui::TreePop();
	ImGui::PopID();
}

void PropertyView::ShowValueBegin(const MString& strValueName)
{
	ImGui::PushID(GetID(strValueName));
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx(strValueName.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
}

void PropertyView::ShowValueEnd()
{
	ImGui::NextColumn();
	ImGui::PopID();
}

bool PropertyView::EditM3DNode(M3DNode* pNode)
{
	if (nullptr == pNode)
		return false;

	if (ShowNodeBegin("3DNode"))
	{
		MTransform trans = pNode->GetTransform();
		if (EditTransform(trans))
		{
			pNode->SetTransform(trans);
		}

		ShowNodeEnd();
	}

	return true;
}

bool PropertyView::EditVector3(Vector3& value)
{
	return ImGui::DragFloat3("", value.m);
}

bool PropertyView::EditTransform(MTransform& trans)
{
	bool bModify = false;
	ShowValueBegin("Position");
	Vector3 position = trans.GetPosition();
	if (EditVector3(position))
	{
		trans.SetPosition(position);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Scale");
	Vector3 scale = trans.GetScale();
	if (EditVector3(scale))
	{
		trans.SetScale(scale);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Rotate");
	Quaternion quat = trans.GetRotation();
	Vector3 rotate = quat.GetEulerAngle();
	Vector3 old = rotate;
	if (EditVector3(rotate))
	{
		if (rotate.x > 90.0f)
			rotate.x = 90.0f;
		else if (rotate.x < -90.0f)
			rotate.x = -90.0f;

		quat.SetEulerAngle(rotate);
		quat.Normalize();
		trans.SetRotation(quat);

		bModify = true;
	}
	ShowValueEnd();

	return bModify;
}

unsigned int PropertyView::GetID(const MString& strItemName)
{
	unsigned int unID = m_tItemID[strItemName];
	if (unID != 0)
		return unID;
	
	m_tItemID[strItemName] = ++m_unItemIDPool;
	return m_unItemIDPool;
}

