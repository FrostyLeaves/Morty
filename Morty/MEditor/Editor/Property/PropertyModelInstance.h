#ifndef _PROPERTY_MMODELINSTANCE_H_
#define _PROPERTY_MMODELINSTANCE_H_

#include "PropertyM3DNode.h"
#include "MModelInstance.h"

class PropertyMModelInstance : public PropertyM3DNode
{
public:
	virtual void EditObject(MObject* pObject) override
	{
// 		if (MModelInstance* pNode = pObject->DynamicCast<MModelInstance>())
// 		{
// 			if (ShowNodeBegin("ModelInstance"))
// 			{
// 				ShowNodeEnd();
// 			}
// 		}
	}
};


#endif