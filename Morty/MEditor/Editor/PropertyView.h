#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "Vector.h"
#include "MString.h"
#include "MTransform.h"
#include "MVariant.h"

#include <map>

class MObject;
class M3DNode;
class PropertyView
{
public:
	PropertyView();
	virtual ~PropertyView();


public:
	void SetEditorObject(MObject* pObject);

	void Render();

protected:

	bool ShowNodeBegin(const MString& strNodeName);
	void ShowNodeEnd();

	void ShowValueBegin(const MString& strValueName);
	void ShowValueEnd();

	bool EditM3DNode(M3DNode* pNode);

	bool EditVector3(Vector3& value, const float& fSpeed = 1.0f, const float& fMin = 0.0f, const float& fMax = 0.0f);
	bool EditTransform(MTransform& trans);

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
	MObject* m_pEditorObject;
	unsigned int m_unItemIDPool;
	std::map<MString, unsigned int> m_tItemID;

	std::map<unsigned int, MVariant> m_tTempValue;
};







#endif