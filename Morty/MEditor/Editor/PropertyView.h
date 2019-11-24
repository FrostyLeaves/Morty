#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "Vector.h"
#include "MString.h"
#include "MTransform.h"

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

	bool EditVector3(Vector3& value);
	bool EditTransform(MTransform& trans);

	unsigned int GetID(const MString& strItemName);

private:
	MObject* m_pEditorObject;
	unsigned int m_unItemIDPool;
	std::map<MString, unsigned int> m_tItemID;
};







#endif