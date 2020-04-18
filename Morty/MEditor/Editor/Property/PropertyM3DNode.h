#ifndef _PROPERTY_M3DNODE_H_
#define _PROPERTY_M3DNODE_H_

#include "PropertyBase.h"
#include "M3DNode.h"

#include "imgui.h"
#include "Light/MDirectionalLight.h"
#include "MScene.h"
#include "MShadowTextureRenderTarget.h"
#include "MTexture.h"
#include "MRenderStructure.h"

class PropertyM3DNode : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (M3DNode* pNode = pObject->DynamicCast<M3DNode>())
		{
			PROPERTY_NODE_EDIT(pNode, "Transform", MTransform, GetTransform, SetTransform);

			PROPERTY_VALUE_EDIT(pNode, "Visible", bool, GetVisible, SetVisible);


// 			ShowValueBegin("ShadowMap");
// 			MScene* pScene = pNode->GetScene();
// 			if (MShadowTextureRenderTarget* pShadowTextureRt = pScene->GetShadowRenderTarget())
// 			{
// 				if (MRenderDepthTexture* pDepthTexture = pShadowTextureRt->GetDepthTexture())
// 				{
// 					if (MTextureBuffer* pBuffer = pDepthTexture->GetBuffer())
// 					{
// 						ImGui::Image(pBuffer->m_pShaderResourceView, ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth()));
// 					}
// 				}
// 			}
// 			ShowValueEnd();
		}
	}
};


#endif