#ifndef _PROPERTY_BASE_H_
#define _PROPERTY_BASE_H_

#include "MString.h"
#include "Vector.h"
#include "MVariant.h"
#include "MTransform.h"
#include "MColor.h"

#include <map>

class MObject;
class PropertyBase
{
public:

	PropertyBase() {}
	virtual ~PropertyBase() {};

	virtual void EditObject(MObject* pObject) = 0;

	bool ShowNodeBegin(const MString& strNodeName);
	void ShowNodeEnd();

	void ShowValueBegin(const MString& strValueName);
	void ShowValueEnd();

	bool Editbool(bool& value);
	bool Editfloat(float& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector2(Vector2& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditVector3(Vector3& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditMTransform(MTransform& trans);
	bool EditEnum(const std::vector<MString>& select, unsigned int& index);

	bool EditMColor(MColor& value);

	unsigned int GetID(const MString& strItemName);

	template<class T>
	T& GetTempValue(const MString& strValueName, const T& defaultValue)
	{
		unsigned int unRotateID = GetID(strValueName);
		if (m_tTempValue[unRotateID].GetType() == MVariant::ENone)
			m_tTempValue[unRotateID] = MVariant(defaultValue);

		T& result = *(static_cast<T*>(m_tTempValue[unRotateID].GetData()));

		return result;
	}

private:

	static unsigned int m_unItemIDPool;
	static std::map<MString, unsigned int> m_tItemID;

	std::map<unsigned int, MVariant> m_tTempValue;
};

#define PROPERTY_NODE_EDIT(  NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC) \
if (ShowNodeBegin(KEY_NAME))	{ \
	TYPE value = NODE->##GET_FUNC(); \
	if (Edit##TYPE(value)) {\
		NODE->##SET_FUNC(value);	\
	}	\
	ShowNodeEnd();	\
}

#define PROPERTY_VALUE_EDIT( NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC) \
	ShowValueBegin(KEY_NAME); {\
	TYPE value = NODE->##GET_FUNC(); \
	if (Edit##TYPE(value)) {\
		NODE->##SET_FUNC(value);	\
	} \
	ShowValueEnd();	\
}


#endif