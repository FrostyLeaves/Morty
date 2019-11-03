#include "PropertyView.h"

#include "imgui.h"

#include "MObject.h"
#include "M3DNode.h"

PropertyView::PropertyView()
	: m_pEditorObject(nullptr)
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
	
	EditM3DNode(dynamic_cast<M3DNode*>(m_pEditorObject), unID);

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();


}

bool PropertyView::ShowNodeBegin(const MString& strNodeName, unsigned int& unID)
{
	ImGui::PushID(unID);
	ImGui::AlignTextToFramePadding();  
	if (bool node_open = ImGui::TreeNode("Object", "%s", strNodeName.c_str()))
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

void PropertyView::ShowValueBegin(const MString& strValueName, unsigned int& unID)
{
	ImGui::PushID(unID);
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

bool PropertyView::EditM3DNode(M3DNode* pNode, unsigned int& unID)
{
	if (nullptr == pNode)
		return false;

	if (ShowNodeBegin("3DNode", ++unID))
	{
		MTransform trans = pNode->GetTransform();
		if (EditTransform(trans, ++unID))
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

bool PropertyView::EditTransform(MTransform& trans, unsigned int& unID)
{
	bool bModify = false;
	ShowValueBegin("Position", ++unID);
	Vector3 position = trans.GetPosition();
	if (EditVector3(position))
	{
		trans.SetPosition(position);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Scale", ++unID);
	Vector3 scale = trans.GetScale();
	if (EditVector3(scale))
	{
		trans.SetScale(scale);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Rotate", ++unID);
	Quaternion quat = trans.GetRotation();
	Vector3 rotate = quat.GetEulerAngle();
	Vector3 old = rotate;
	if (EditVector3(rotate))
	{
		if (rotate.x != old.x)
		{
			quat = quat * Quaternion(trans.GetRight(), rotate.x - old.x);
		}
		if (rotate.y != old.y)
		{
			quat = quat * Quaternion(trans.GetUp(), rotate.y - old.y);
		}
		if (rotate.z != old.z)
		{
			quat = quat * Quaternion(trans.GetForward(), rotate.z - old.z);
		}
		trans.SetRotation(quat);
		bModify = true;
	}
	ShowValueEnd();

	return bModify;
}

