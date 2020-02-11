#ifndef _PROPERTY_MIMESHINSTANCE_H_
#define _PROPERTY_MIMESHINSTANCE_H_

#include "PropertyM3DNode.h"
#include "MMaterial.h"
#include "MIMeshInstance.h"


class PropertyMIMeshInstance : public PropertyM3DNode
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MIMeshInstance* pNode = pObject->DynamicCast<MIMeshInstance>())
		{
			if (ShowNodeBegin("Material"))
			{
				MMaterial* pMaterial = pNode->GetMaterial();

				
				


				EditMMaterial(pNode->GetMaterial());

				ShowNodeEnd();
			}
		}
	}
};


#endif