#include "PropertyBase.h"

#include "imgui.h"

unsigned int PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_tItemID = std::map<MString, unsigned int>();

bool PropertyBase::ShowNodeBegin(const MString& strNodeName)
{
	ImGui::PushID(GetID(strNodeName));
	ImGui::AlignTextToFramePadding();
	if (bool node_open = ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s", strNodeName.c_str()))
	{
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();
		//ImGui::Text("0.0");
		ImGui::NextColumn();

		return true;
	}

	ImGui::PopID();
	return false;
}

void PropertyBase::ShowNodeEnd()
{
	ImGui::TreePop();
	ImGui::PopID();
}

void PropertyBase::ShowValueBegin(const MString& strValueName)
{
	ImGui::PushID(GetID(strValueName));
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx(strValueName.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
}

void PropertyBase::ShowValueEnd()
{
	ImGui::NextColumn();
	ImGui::PopID();
}

bool PropertyBase::Editbool(bool& value)
{
	return ImGui::Checkbox("", &value);
}

bool PropertyBase::Editfloat(float& value, const float& fSpeed, const float& fMin, const float& fMax)
{
	if (value == -0.0f) value = 0.0f;
	return ImGui::DragFloat("", &value, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector2(Vector2& value, const float& fSpeed /*= 1.0f*/, const float& fMin /*= 0.0f*/, const float& fMax /*= 0.0f*/)
{
	if (value.x == -0.0f) value.x = 0.0f;
	if (value.y == -0.0f) value.y = 0.0f;
	return ImGui::DragFloat2("", value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector3(Vector3& value, const float& fSpeed, const float& fMin, const float& fMax)
{
	if (value.x == -0.0f) value.x = 0.0f;
	if (value.y == -0.0f) value.y = 0.0f;
	if (value.z == -0.0f) value.z = 0.0f;
	return ImGui::DragFloat3("", value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditMTransform(MTransform& trans)
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

bool PropertyBase::EditEnum(const std::vector<MString>& select, unsigned int& index)
{
	unsigned int unNewIndex = index;
	if (ImGui::BeginCombo("", select[index].c_str())) {

		for (int i = 0; i < select.size(); ++i)
		{
			if (ImGui::Selectable(select[i].c_str(), (index == i)))\
			{
				unNewIndex = i;
			}
		}

		ImGui::EndCombo();
	}

	if (unNewIndex != index)
	{
		index = unNewIndex;
		return true;
	}

	return false;
}

bool PropertyBase::EditMColor(MColor& value)
{
	return ImGui::ColorEdit4("", value.m);
}

unsigned int PropertyBase::GetID(const MString& strItemName)
{
	unsigned int unID = m_tItemID[strItemName];
	if (unID != 0)
		return unID;

	m_tItemID[strItemName] = ++m_unItemIDPool;
	return m_unItemIDPool;
}

