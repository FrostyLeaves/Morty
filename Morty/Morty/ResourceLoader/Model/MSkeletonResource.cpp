#include "MSkeletonResource.h"

#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"

M_RESOURCE_IMPLEMENT(MSkeletonResource, MResource)

MSkeletonResource::MSkeletonResource()
    : MResource()
    , m_pSkeletonTemplate(nullptr)
{
}

MSkeletonResource::~MSkeletonResource()
{
    Clean();
}

void MSkeletonResource::SetSkeleton(MSkeleton* pSkeleton)
{
    Clean();
    m_pSkeletonTemplate = pSkeleton;
}

void MSkeletonResource::Clean()
{
    if (m_pSkeletonTemplate)
    {
        delete m_pSkeletonTemplate;
        m_pSkeletonTemplate = nullptr;
    }
}

void MSkeletonResource::WriteToStruct(MStruct& srt)
{
    if (m_pSkeletonTemplate)
    {
        const std::vector<MBone*>& vBones = m_pSkeletonTemplate->GetAllBones();
        
        srt.AppendMVariant("Bones", MVariantArray());
        MVariantArray* pArray = srt.FindMember("Bones")->GetArray();
        pArray->Resize(vBones.size());

        for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
        {
            MBone* pBone = vBones[i];
            (*pArray)[i] = MStruct();
            MStruct& boneSrt = *(*pArray)[i].GetStruct();

			MVariantArray vChildren;
			for (unsigned int unChildIdx : pBone->vChildrenIndices)
				vChildren.AppendMVariant(unChildIdx);

            boneSrt.AppendMVariant("Name", pBone->strName);
            boneSrt.AppendMVariant("Index", pBone->unIndex);
            boneSrt.AppendMVariant("ParentIndex", pBone->unParentIndex);
            boneSrt.AppendMVariant("matTrans", pBone->m_matTransform);
            boneSrt.AppendMVariant("matOffset", pBone->m_matOffsetMatrix);
            boneSrt.AppendMVariant("Children", vChildren);
        }

    }
}

void MSkeletonResource::ReadFromStruct(MStruct& srt)
{
    Clean();
    m_pSkeletonTemplate = new MSkeleton();

    if (MVariant* pBonesVar = srt.FindMember("Bones"))
    {
        if (MVariantArray* pBonesArray = pBonesVar->GetArray())
        {
            unsigned int unBonesCount = pBonesArray->GetMemberCount();

            m_pSkeletonTemplate->m_vAllBones.resize(unBonesCount);

            for (unsigned int i = 0; i < unBonesCount; ++i)
            {
                MVariant& boneVar = pBonesArray->GetMember(i)->var;
                if (MStruct* pBoneSrt = boneVar.GetStruct())
                {
                    MBone* pBone = new MBone();
                    m_pSkeletonTemplate->m_vAllBones[i] = pBone;

                    if (MVariant* pName = pBoneSrt->FindMember("Name"))
                        pBone->strName = pName->GetString();
                    if (MVariant* pIndex = pBoneSrt->FindMember("Index"))
                        pBone->unIndex = pIndex->GetInt();
                    if (MVariant* pParentIndex = pBoneSrt->FindMember("ParentIndex"))
                        pBone->unParentIndex = pParentIndex->GetInt();
                    if (MVariant* pMatTrans = pBoneSrt->FindMember("matTrans"))
                        pBone->m_matTransform = pMatTrans->GetMatrix4();
                    if (MVariant* pMatOffset = pBoneSrt->FindMember("matOffset"))
                        pBone->m_matOffsetMatrix = pMatOffset->GetMatrix4();

                    if (MVariant* pChildren = pBoneSrt->FindMember("Children"))
                    {
                        if (MVariantArray* pChildrenArray = pChildren->GetArray())
                        {
                            unsigned int unChildrenCount = pChildrenArray->GetMemberCount();
                            pBone->vChildrenIndices.resize(unChildrenCount);

                            for (unsigned int cldIdx = 0; cldIdx < unChildrenCount; ++cldIdx)
                            {
                                pBone->vChildrenIndices[cldIdx] = pChildrenArray->GetMember(cldIdx)->var.GetInt();
                            }
                        }
                    }
                }
            }
        }
    }
}

bool MSkeletonResource::Load(const MString& strResourcePath)
{

    return false;
}

bool MSkeletonResource::SaveTo(const MString& strResourcePath)
{
    if (m_pSkeletonTemplate)
    {
        const std::vector<MBone*>& vBones = m_pSkeletonTemplate->GetAllBones();
        
        MVariant var = MVariantArray();
        MVariantArray* pArray = var.GetArray();
        pArray->Resize(vBones.size());

        for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
        {
            MBone* pBone = vBones[i];
            (*pArray)[i] = MStruct();
            MStruct& srt = *(*pArray)[i].GetStruct();

			MVariantArray vChildren;
			for (unsigned int unChildIdx : pBone->vChildrenIndices)
				vChildren.AppendMVariant(unChildIdx);

            srt.AppendMVariant("Name", pBone->strName);
            srt.AppendMVariant("Index", pBone->unIndex);
            srt.AppendMVariant("ParentIndex", pBone->unParentIndex);
            srt.AppendMVariant("matTrans", pBone->m_matTransform);
            srt.AppendMVariant("matOffset", pBone->m_matOffsetMatrix);
            srt.AppendMVariant("Children", vChildren);         
        }

        return true;
    }

    return false;
}

