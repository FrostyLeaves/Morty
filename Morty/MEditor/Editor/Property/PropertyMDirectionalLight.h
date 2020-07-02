#ifndef _PROPERTY_MDIRECTIONALLIGHT_H_
#define _PROPERTY_MDIRECTIONALLIGHT_H_

#include "PropertyBase.h"
#include "Light/MDirectionalLight.h"



class PropertyMDirectionalLight : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MDirectionalLight* pNode = pObject->DynamicCast<MDirectionalLight>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pNode, "Diffuse", MColor, GetDiffuseColor, SetDiffuseColor);
				PROPERTY_VALUE_EDIT(pNode, "Specular", MColor, GetSpecularColor, SetSpecularColor);

				ShowNodeEnd();

				ShowNodeBegin("Shadow");
				MScene* pScene = pNode->GetScene();
				std::vector<MViewport*> viewports = pScene->GetViewports();
				for (MViewport* pViewport : viewports)
				{
// 					ShowValueBegin("ShadowMap");
// 					if (MRenderDepthTexture* pDepthTexture = pViewport->GetRenderProgram())
// 					{
// 						if (MTextureBuffer* pBuffer = pDepthTexture->GetBuffer())
// 						{
// 							ImGui::Image(ImTextureID(pBuffer->GetResourceView(), 1), ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth()));
// 						}
// 					}
// 					ShowValueEnd();
				}


				
				ShowNodeEnd();
			}
		}
	}
};


#endif