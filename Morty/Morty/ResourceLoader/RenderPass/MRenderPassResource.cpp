#include "MRenderPassResource.h"

#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"

M_RESOURCE_IMPLEMENT(MRenderPassResource, MResource)

MRenderPassResource::MRenderPassResource()
	: MResource()
	, m_pRenderPass(nullptr)
{

}

MRenderPassResource::~MRenderPassResource()
{

}

bool MRenderPassResource::Load(const MString& strResourcePath)
{
	MVariant var;
	MString strCode;

	if (!MFileHelper::ReadString(strResourcePath, strCode))
		return false;

	if (!MJson::JsonToMVariant(strCode, var))
		return false;

	MStruct* pStructRoot = var.GetStruct();
	if (!pStructRoot)
		return false;

	if (MStruct* pBackTexture = pStructRoot->FindMember<MStruct>("BackTexture"))
	{

	}

	if (MStruct* pDepthTexture = pStructRoot->FindMember<MStruct>("DepthTexture"))
	{

	}

	if (MVariantArray* pSubpassArray = pStructRoot->FindMember<MVariantArray>("Subpass"))
	{

	}
}
