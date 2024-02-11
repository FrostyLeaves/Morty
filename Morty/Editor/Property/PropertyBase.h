#pragma once

#include "Scene/MEntity.h"
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Math/Vector.h"
#include "Variant/MVariant.h"
#include "Utility/MTransform.h"
#include "Utility/MColor.h"
#include "System/MResourceSystem.h"

#include <map>
#include <any>
#include <functional>
#include <stdint.h>

MORTY_SPACE_BEGIN

class MShaderPropertyBlock;
class MEntity;
class MObject;
class MMaterial;
class MResource;
class MTexture;

class PropertyBase
{
public:

	PropertyBase() {}
	virtual ~PropertyBase() {};

	virtual void EditEntity(MEntity* pObject) { MORTY_UNUSED(pObject); };

	bool ShowNodeBegin(const MString& strNodeName);
	bool ShowNodeBeginWithEx(const MString& strNodeName);
	void ShowNodeExBegin(const MString& strExID);
	void ShowNodeExEnd();
	void ShowNodeEnd();

	void ShowValueBegin(const MString& strValueName);
	void ShowValueEnd();

	//normal
	bool Editbool(bool& value);
	bool Editbool(int& value);
	bool Editfloat(float& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector2(Vector2& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector3(Vector3& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector3(float* pValue, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector4(Vector4& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector4(float* pValue, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditMTransform(MTransform& trans);
	bool EditEnum(const std::vector<MString>& select, size_t& index);
	bool EditMColor(MColor& value);
	bool EditMString(MString& value);

	bool EditMMaterial(std::shared_ptr<MMaterial> pMaterial);
	bool EditShaderProperty(const std::shared_ptr<MShaderPropertyBlock>& pProperty);
	void EditMResource(const MString& strDlgID, const MString& strResourceType, const std::vector<MString>& vSuffixList,  std::shared_ptr<MResource> pDefaultResource, const std::function<void(const MString& strNewFilePath)>& funcLoadResource);
	void EditSaveMResource(const MString& stringID, const MString& strResourceType, const std::vector<MString>& vSuffixList, std::shared_ptr<MResource> pResource);

	void ShowTexture(std::shared_ptr<MTexture> pTexture, const Vector2& v2Size);

	//auto call ShowValue/ShowNode
	bool EditMVariant(const MString& strVariantName, MVariant& value);

	unsigned int GetID(const MString& strItemName);

	template<typename TYPE>
	TYPE GetTemporaryValue(const MString& strValueName, const TYPE& defaultValue);
	template<typename TYPE>
	void SetTemporaryValue(const MString& strValueName, const TYPE& valuealue);


private:

	static unsigned int m_unItemIDPool;
	static std::map<MString, unsigned int> m_tItemID;

	std::map<MString, std::any> m_tTemporaryValue;
};

#define PROPERTY_NODE_EDIT(  NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC) \
if (ShowNodeBegin(KEY_NAME))	{ \
	TYPE value = NODE->GET_FUNC(); \
	if (Edit##TYPE(value)) {\
		NODE->SET_FUNC(value);	\
	}	\
	ShowNodeEnd();	\
}

#define PROPERTY_VALUE_EDIT( NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC) \
	ShowValueBegin(KEY_NAME); {\
	TYPE value = NODE->GET_FUNC(); \
	if (Edit##TYPE(value)) {\
		NODE->SET_FUNC(value);	\
	} \
	ShowValueEnd();	\
}

#define PROPERTY_VALUE_EDIT_SPEED_MIN_MAX( NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC, SPEED, MIN_VAR, MAX_VAR) \
	ShowValueBegin(KEY_NAME); {\
	TYPE value = NODE->GET_FUNC(); \
	if (Edit##TYPE(value, SPEED, MIN_VAR, MAX_VAR)) {\
		NODE->SET_FUNC(value);	\
	} \
	ShowValueEnd();	\
}

template<typename TYPE>
TYPE PropertyBase::GetTemporaryValue(const MString& strValueName, const TYPE& defaultValue)
{
	if (m_tTemporaryValue.find(strValueName) == m_tTemporaryValue.end())
	{
	    m_tTemporaryValue[strValueName] = defaultValue;
		return defaultValue;
	}

	try
	{
		TYPE result = std::any_cast<TYPE>(m_tTemporaryValue[strValueName]);
		return result;
	}
	catch (const std::bad_any_cast& e)
	{
		return defaultValue;
	}

	return defaultValue;
}

template<typename TYPE>
void PropertyBase::SetTemporaryValue(const MString& strValueName, const TYPE& valuealue)
{
	m_tTemporaryValue[strValueName] = valuealue;
}

MORTY_SPACE_END