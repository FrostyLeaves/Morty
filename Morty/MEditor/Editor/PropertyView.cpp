#include "PropertyView.h"

#include "MObject.h"

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
	MString strClassName = pObject->GetClassName();


}

