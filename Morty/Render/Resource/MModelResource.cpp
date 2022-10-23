#include "Resource/MModelResource.h"
#include "Engine/MEngine.h"
#include "Utility/MJson.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"

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

void MModelResource::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeleton)
{
    m_pSkeleton = pSkeleton;
}

void MModelResource::GetMeshResources(const std::vector<std::shared_ptr<MMeshResource>>& vMeshes)
{
    m_vMeshes = vMeshes;
}

void MModelResource::OnDelete()
{
    if (m_pSkeleton)
    {
        m_pSkeleton = nullptr;
    }

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
    
    if (MString* pSkePath = pStruct->GetValue<MString>("ske"))
    {
        m_pSkeleton = MTypeClass::DynamicCast<MSkeletonResource>(pResourceSystem->LoadResource(*pSkePath));
    }

    if (MVariantArray* pMeshPathArray = pStruct->GetValue<MVariantArray>("mesh"))
    {
        for (uint32_t i = 0; i < pMeshPathArray->GetMemberCount(); ++i)
        {
            if (MString* pMeshPath = pMeshPathArray->GetMember(i)->var.GetString())
            {
                if (std::shared_ptr<MResource> pRes = pResourceSystem->LoadResource(*pMeshPath))
                {
                    std::shared_ptr<MMeshResource> pMeshRes = MTypeClass::DynamicCast<MMeshResource>(pRes);

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
        srt.SetValue("ske", m_pSkeleton->GetResourcePath());
    }

    srt.SetValue("mesh", MVariantArray());
    if (MVariantArray* pMeshArray = srt.GetValue<MVariantArray>("mesh"))
    {
        for (std::shared_ptr<MMeshResource> pMeshRes : m_vMeshes)
        {
            pMeshArray->AppendValue(pMeshRes->GetResourcePath());
        }
    }

    MString code;
    MJson::MVariantToJson(var, code);

    return MFileHelper::WriteString(strResourcePath, code);
}
