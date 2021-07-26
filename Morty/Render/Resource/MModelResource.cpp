#include "MModelResource.h"
#include "MEngine.h"
#include "MJson.h"
#include "MFileHelper.h"
#include "MResourceSystem.h"

MORTY_INTERFACE_IMPLEMENT(MModelResource, MResource)

MModelResource::MModelResource()
    : MResource()
    , m_pSkeleton(nullptr)
    , m_vMeshes()
{
}

MModelResource::~MModelResource()
{
}

void MModelResource::SetSkeletonResource(MSkeletonResource* pSkeleton)
{
    if (m_pSkeleton)
        m_pSkeleton->SubRef();
    
    m_pSkeleton = pSkeleton;

    if (m_pSkeleton)
        m_pSkeleton->AddRef();
}

void MModelResource::GetMeshResources(const std::vector<MMeshResource*>& vMeshes)
{
    for (MMeshResource* pMeshRes : m_vMeshes)
        pMeshRes->SubRef();

    m_vMeshes = vMeshes;

	for (MMeshResource* pMeshRes : m_vMeshes)
		pMeshRes->AddRef();
}

void MModelResource::OnDelete()
{
    if (m_pSkeleton)
    {
        m_pSkeleton->SubRef();
        m_pSkeleton = nullptr;
    }

	for (MMeshResource* pMeshRes : m_vMeshes)
		pMeshRes->SubRef();

    m_vMeshes.clear();
}

bool MModelResource::Load(const MString& strResourcePath)
{
    MResourceSystem* pResourceSystem = GetResourceSystem();

    MString code;
    MFileHelper::ReadString(strResourcePath, code);

    MVariant var;
    MJson::JsonToMVariant(code, var);

    MStruct* pStruct = var.GetStruct();

    if (!pStruct)
        return false;
    
    if (MString* pSkePath = pStruct->FindMember<MString>("ske"))
    {
        m_pSkeleton = pResourceSystem->LoadResource(*pSkePath)->DynamicCast<MSkeletonResource>();
        m_pSkeleton->AddRef();
    }

    if (MVariantArray* pMeshPathArray = pStruct->FindMember<MVariantArray>("mesh"))
    {
        for (uint32_t i = 0; i < pMeshPathArray->GetMemberCount(); ++i)
        {
            if (MString* pMeshPath = pMeshPathArray->GetMember(i)->var.GetString())
            {
                if (MResource* pRes = pResourceSystem->LoadResource(*pMeshPath))
                {
                    MMeshResource* pMeshRes = pRes->DynamicCast<MMeshResource>();
                    pMeshRes->AddRef();

                    m_vMeshes.push_back(pMeshRes);
                }
            }
        }
    }

    return true;
}

bool MModelResource::SaveTo(const MString& strResourcePath)
{
    MVariant var = MStruct();
    MStruct& srt = *var.GetStruct();

    if (m_pSkeleton)
    {
        srt.AppendMVariant("ske", m_pSkeleton->GetResourcePath());
    }

    if (MVariantArray* pMeshArray = srt.AppendMVariant<MVariantArray>("mesh"))
    {
        for (MMeshResource* pMeshRes : m_vMeshes)
        {
            pMeshArray->AppendMVariant(pMeshRes->GetResourcePath());
        }
    }

    MString code;
    MJson::MVariantToJson(var, code);

    return MFileHelper::WriteString(strResourcePath, code);
}
