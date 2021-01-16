#include "MRenderPassResource.h"

#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"
#include "MRenderPass.h"

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

	std::map<MString, uint32_t> tBackTextureRef;

	if (MVariantArray* pBackTexture = pStructRoot->FindMember<MVariantArray>("BackTexture"))
	{
		uint32_t nSize = pBackTexture->GetMemberCount();
		for (uint32_t i = 0; i < nSize; ++i)
		{
			if (MStruct* tex = pBackTexture->GetMember<MStruct>(i))
			{
				m_pRenderPass->m_vBackDesc.push_back(MPassTargetDescription());
				MPassTargetDescription& desc = m_pRenderPass->m_vBackDesc.back();

				if (MString* name = tex->FindMember<MString>("name"))
				{
					tBackTextureRef[*name] = i;
					desc.m_strName = *name;
				}

				if (Vector4* clearColor = tex->FindMember<Vector4>("clear"))
				{
					desc.bClearWhenRender = true;
					desc.cClearColor = *clearColor;
				}
				else
				{
					desc.bClearWhenRender = false;
				}
			}
		}
	}

	if (MStruct* pDepthTexture = pStructRoot->FindMember<MStruct>("DepthTexture"))
	{
		m_pRenderPass->m_DepthDesc.bClearWhenRender = true;
	}
	else
	{
		m_pRenderPass->m_DepthDesc.bClearWhenRender = false;
	}

	std::map<MString, uint32_t> tSubPassRef;

	if (MVariantArray* pSubpassArray = pStructRoot->FindMember<MVariantArray>("Subpass"))
	{
		uint32_t nSize = pSubpassArray->GetMemberCount();
		for (uint32_t i = 0; i < nSize; ++i)
		{
			m_pRenderPass->m_vSubpass.push_back(MSubpass());
			MSubpass& subpass = m_pRenderPass->m_vSubpass.back();

			if (MStruct* pSubpass = pSubpassArray->GetMember<MStruct>(i))
			{
				if (MString* pSubpassName = pSubpass->FindMember<MString>("name"))
				{
					tSubPassRef[*pSubpassName] = i;
					subpass.m_strName = *pSubpassName;
				}

				if (MVariantArray* pInputs = pSubpass->FindMember<MVariantArray>("input"))
				{
					uint32_t nInputSize = pInputs->GetMemberCount();
					for (uint32_t nInputIdx = 0; nInputIdx < nInputSize; ++nInputIdx)
					{
						if (MString* pInputName = pInputs->GetMember<MString>(nInputIdx))
						{
							auto find = tBackTextureRef.find(*pInputName);
							if (find != tBackTextureRef.end())
							{
								subpass.m_vInputIndex.push_back(find->second);
							}
						}
					}
				}

				if (MVariantArray* pOutputs = pSubpass->FindMember<MVariantArray>("output"))
				{
					uint32_t nOutputSize = pOutputs->GetMemberCount();
					for (uint32_t nOutputIdx = 0; nOutputIdx < nOutputSize; ++nOutputIdx)
					{
						if (MString* pOutputName = pOutputs->GetMember<MString>(nOutputIdx))
						{
							auto find = tBackTextureRef.find(*pOutputName);
							if (find != tBackTextureRef.end())
							{
								subpass.m_vOutputIndex.push_back(find->second);
							}
						}
					}
				}

				MVariantArray* pReference = pSubpass->FindMember<MVariantArray>("reference");
				int* pLoop = pSubpass->FindMember<int>("loop");

				if (pReference && pLoop)
				{
					std::vector<uint32_t> vAppendIdxs;

					uint32_t nRefSize = pReference->GetMemberCount();
					for (uint32_t nRefIdx = 0; nRefIdx < nRefSize; ++nRefIdx)
					{
						if (MString* pRefName = pReference->GetMember<MString>(nRefIdx))
						{
							auto find = tSubPassRef.find(*pRefName);
							if (find != tSubPassRef.end())
							{
								vAppendIdxs.push_back(find->second);
							}
						}
					}

					for (uint32_t nLoopIdx = 0; nLoopIdx < *pLoop; ++nLoopIdx)
					{
						for (const uint32_t& nSubpassIdx : vAppendIdxs)
						{
							m_pRenderPass->m_vSubpass.push_back(m_pRenderPass->m_vSubpass[nSubpassIdx]);
						}
					}
				}
			}
		}
	}
}

bool MRenderPassResource::SaveTo(const MString& strResourcePath)
{
	MStruct srt;


	if (MVariantArray* pBackTextures = srt.AppendMVariant<MVariantArray>("BackTexture"))
	{

	}

	if (MVariantArray* pSubpass = srt.AppendMVariant<MVariantArray>("Subpass"))
	{

	}

	return false;
}
