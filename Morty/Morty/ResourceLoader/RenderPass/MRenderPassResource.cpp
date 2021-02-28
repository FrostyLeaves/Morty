#include "MRenderPassResource.h"

#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"
#include "MRenderPass.h"

M_RESOURCE_IMPLEMENT(MRenderPassResource, MResource)

MRenderPassResource::MRenderPassResource()
	: MResource()
	, m_pRenderPass(new MRenderPass())
{

}

MRenderPassResource::~MRenderPassResource()
{
	if (m_pRenderPass)
	{
		delete m_pRenderPass;
		m_pRenderPass = nullptr;
	}
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
			if (MStruct* pSubpass = pSubpassArray->GetMember<MStruct>(i))
			{
				m_pRenderPass->m_vSubpass.push_back(MSubpass());
				MSubpass& subpass = m_pRenderPass->m_vSubpass.back();

// 				if (MString* pSubpassName = pSubpass->FindMember<MString>("name"))
// 				{
// 					tSubPassRef[*pSubpassName] = i;
// 					subpass.m_strName = *pSubpassName;
// 				}

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
			}
		}
	}
}

bool MRenderPassResource::SaveTo(const MString& strResourcePath)
{
	MStruct srt;

	if (MVariantArray* pBackTextures = srt.AppendMVariant<MVariantArray>("BackTexture"))
	{
		for (const MPassTargetDescription& desc : m_pRenderPass->m_vBackDesc)
		{
			if (MStruct* pStruct = pBackTextures->AppendMVariant<MStruct>())
			{
				if (desc.bClearWhenRender)
				{
					pStruct->AppendMVariant("clear", desc.cClearColor.ToVector4());
				}
			}
		}
	}

	if (MVariantArray* pSubpass = srt.AppendMVariant<MVariantArray>("Subpass"))
	{
		for (const MSubpass& subpass : m_pRenderPass->m_vSubpass)
		{
			if (MStruct* pStruct = pSubpass->AppendMVariant<MStruct>())
			{
				//				pStruct->AppendMVariant("name", subpass.m_strName);

				if (MVariantArray* inputs = pStruct->AppendMVariant<MVariantArray>("input"))
				{
					for (const uint32_t& unIdx : subpass.m_vInputIndex)
					{
						if (unIdx < m_pRenderPass->m_vBackDesc.size())
						{
							const MPassTargetDescription& desc = m_pRenderPass->m_vBackDesc[unIdx];
						}
					}
				}

				if (MVariantArray* outputs = pStruct->AppendMVariant<MVariantArray>("output"))
				{
					for (const uint32_t& unIdx : subpass.m_vOutputIndex)
					{
						if (unIdx < m_pRenderPass->m_vBackDesc.size())
						{
							const MPassTargetDescription& desc = m_pRenderPass->m_vBackDesc[unIdx];
						}
					}
				}
			}
		}
	}

	if (m_pRenderPass->m_DepthDesc.bClearWhenRender)
	{
		MStruct* pDepthTexture = srt.AppendMVariant<MStruct>("DepthTexture");
	}

	MString strJsonCode;
	MJson::MVariantToJson(srt, strJsonCode);

	return MFileHelper::WriteString(strResourcePath, strJsonCode);
}
