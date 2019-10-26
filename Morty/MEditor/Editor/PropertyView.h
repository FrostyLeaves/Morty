#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_


class MObject;
class PropertyView
{
public:
	PropertyView();
	virtual ~PropertyView();


public:
	void SetEditorObject(MObject* pObject);



private:
	MObject* m_pEditorObject;
};







#endif