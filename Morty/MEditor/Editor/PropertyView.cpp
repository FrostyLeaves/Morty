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
	if (m_pEditorObject == pObject)
		return;

	m_pEditorObject = pObject;
	m_tTempValue.clear();
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

bool PropertyView::EditVector3(Vector3& value, const float& fSpeed, const float& fMin, const float& fMax)
{
	if (value.x == -0.0f) value.x = 0.0f;
	if (value.y == -0.0f) value.y = 0.0f;
	if (value.z == -0.0f) value.z = 0.0f;
	return ImGui::DragFloat3("", value.m, fSpeed, fMin, fMax);
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
	Vector3& rotate = GetTempValue<Vector3>("Rotate", trans.GetRotation().GetEulerAngle());

	if (EditVector3(rotate, 1.0f, -360.0f, 360.0f))
	{
		Quaternion quat;
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

