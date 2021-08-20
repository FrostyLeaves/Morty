#ifndef _PROPERTY_SCENE_COMPONENT_H_
#define _PROPERTY_SCENE_COMPONENT_H_

#include "PropertyBase.h"
#include "MSceneComponent.h"

class PropertyMSceneComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>())
		{
			if (ShowNodeBegin("SceneComponent"))
			{
				PROPERTY_NODE_EDIT(pSceneComponent, "Transform", MTransform, GetTransform, SetTransform);
				ShowNodeEnd();
			}
		}
	}
};


#endif