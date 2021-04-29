#ifndef _PROPERTY_SCENE_COMPONENT_H_
#define _PROPERTY_SCENE_COMPONENT_H_

#include "PropertyBase.h"
#include "MSceneComponent.h"

class PropertyMSceneComponent : public PropertyBase
{
public:
	virtual void EditNode(MNode* pNode) override
	{
		if (MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>())
		{
			PROPERTY_NODE_EDIT(pSceneComponent, "Transform", MTransform, GetTransform, SetTransform);
		}
	}
};


#endif