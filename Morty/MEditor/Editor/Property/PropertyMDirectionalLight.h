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
				ShowValueBegin("ShadowMap");
				MScene* pScene = pNode->GetScene();
				if (MShadowTextureRenderTarget* pShadowTextureRt = pScene->GetShadowRenderTarget())
				{
					if (MRenderDepthTexture* pDepthTexture = pShadowTextureRt->GetDepthTexture())
					{
						if (MTextureBuffer* pBuffer = pDepthTexture->GetBuffer())
						{
							ImGui::Image(ImTextureID(pBuffer->GetResourceView(), 1), ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth()));
						}
					}
				}
				ShowValueEnd();
				ShowNodeEnd();
			}
		}
	}
};


#endif