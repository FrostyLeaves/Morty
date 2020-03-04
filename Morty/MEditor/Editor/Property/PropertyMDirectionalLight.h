#ifndef _PROPERTY_MDIRECTIONALLIGHT_H_
#define _PROPERTY_MDIRECTIONALLIGHT_H_

#include "PropertyBase.h"
#include "MDirectionalLight.h"

#include "MScene.h"
#include "MShadowTextureRenderTarget.h"
#include "MTexture.h"
#include "MRenderStructure.h"


class PropertyMDirectionalLight : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MDirectionalLight* pNode = pObject->DynamicCast<MDirectionalLight>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pNode, "Ambient", MColor, GetAmbientColor, SetAmbientColor);
				PROPERTY_VALUE_EDIT(pNode, "Diffuse", MColor, GetDiffuseColor, SetDiffuseColor);
				PROPERTY_VALUE_EDIT(pNode, "Specular", MColor, GetSpecularColor, SetSpecularColor);


				ShowValueBegin("ShadowMap");
				MScene* pScene = pNode->GetScene();
				if (MShadowTextureRenderTarget* pShadowTextureRt = pScene->GetShadowRenderTarget())
				{
					if (MRenderDepthTexture* pDepthTexture = pShadowTextureRt->GetDepthTexture())
					{
						if (MTextureBuffer* pBuffer = pDepthTexture->GetBuffer())
						{
							ImGui::Image(pBuffer->m_pShaderResourceView, ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth()));
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