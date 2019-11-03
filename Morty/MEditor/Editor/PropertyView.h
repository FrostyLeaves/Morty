#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "Vector.h"
#include "MString.h"
#include "MTransform.h"

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

	bool ShowNodeBegin(const MString& strNodeName, unsigned int& unID);
	void ShowNodeEnd();

	void ShowValueBegin(const MString& strValueName, unsigned int& unID);
	void ShowValueEnd();

	bool EditM3DNode(M3DNode* pNode, unsigned int& unID);

	bool EditVector3(Vector3& value);
	bool EditTransform(MTransform& trans, unsigned int& unID);

private:
	MObject* m_pEditorObject;
};







#endif