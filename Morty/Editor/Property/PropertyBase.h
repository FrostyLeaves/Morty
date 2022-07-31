#ifndef _PROPERTY_BASE_H_
#define _PROPERTY_BASE_H_

#include "Scene/MEntity.h"
#include "Utility/MString.h"
#include "Math/Vector.h"
#include "Utility/MVariant.h"
#include "Utility/MTransform.h"
#include "Utility/MColor.h"
#include "System/MResourceSystem.h"

#include <map>
#include <functional>

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

	virtual void EditEntity(MEntity* pObject) {};

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
	bool EditMTransform(MTransform& trans);
	bool EditEnum(const std::vector<MString>& select, int& index);
	bool EditMColor(MColor& value);
	bool EditMString(MString& value);

	bool EditMMaterial(std::shared_ptr<MMaterial> pMaterial);
	void EditMResource(const MString& strDlgID, const MString& strResourceType, const std::vector<MString>& vSuffixList,  std::shared_ptr<MResource> pDefaultResource, const std::function<void(const MString& strNewFilePath)>& funcLoadResource);
	void EditSaveMResource(const MString& stringID, const MString& strResourceType, const std::vector<MString>& vSuffixList, std::shared_ptr<MResource> pResource);

	void ShowTexture(MTexture* pTexture, const Vector2& v2Size);

	//auto call ShowValue/ShowNode
	bool EditMVariant(const MString& strVariantName, MVariant& value);


	unsigned int GetID(const MString& strItemName);

	template<class T>
	T& GetTempValue(const MString& strValueName, const T& defaultValue)
	{
		unsigned int unRotateID = GetID(strValueName);
		if (m_tTempValue[unRotateID].GetType() == MVariant::MEVariantType::ENone)
			m_tTempValue[unRotateID] = MVariant(defaultValue);

		T& result = *((T*)(m_tTempValue[unRotateID].GetData()));

		return result;
	}

private:

	static unsigned int m_unItemIDPool;
	static std::map<MString, unsigned int> m_tItemID;

	std::map<unsigned int, MVariant> m_tTempValue;
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

#endif
