#include "MSkeletonResource.h"
#include "MResourceManager.h"

#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"

M_RESOURCE_IMPLEMENT(MSkeletonResource, MResource)

MSkeletonResource::MSkeletonResource()
    : MResource()
    , m_pSkeletonTemplate(nullptr)
{
	m_unResourceType = MResourceManager::MEResourceType::Skeleton;
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
        std::vector<MBone>& vBones = m_pSkeletonTemplate->m_vAllBones;
        
        srt.AppendMVariant("Bones", MVariantArray());
        MVariantArray* pArray = srt.FindMember("Bones")->GetArray();
        pArray->Resize(vBones.size());

        for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
        {
            const MBone& bone = vBones[i];
            (*pArray)[i] = MStruct();
            MStruct& boneSrt = *(*pArray)[i].GetStruct();

			MVariantArray vChildren;
			for (unsigned int unChildIdx : bone.vChildrenIndices)
				vChildren.AppendMVariant(unChildIdx);

            boneSrt.AppendMVariant("Name", bone.strName);
            boneSrt.AppendMVariant("Index", bone.unIndex);
            boneSrt.AppendMVariant("ParentIndex", bone.unParentIndex);
            boneSrt.AppendMVariant("matTrans", bone.m_matTransform);
            boneSrt.AppendMVariant("matOffset", bone.m_matOffsetMatrix);
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
                    MBone& bone = m_pSkeletonTemplate->m_vAllBones[i];

                    if(MString *pName = pBoneSrt->FindMember<MString>("Name"))
                        bone.strName = *pName;
                    
                    if (int* pIndex = pBoneSrt->FindMember<int>("Index"))
                        bone.unIndex = *pIndex;
                    
                    if (int* pParentIndex = pBoneSrt->FindMember<int>("ParentIndex"))
                        bone.unParentIndex = *pParentIndex;
                    
                    if (Matrix4* pMatTrans = pBoneSrt->FindMember<Matrix4>("matTrans"))
                        bone.m_matTransform = *pMatTrans;
                    
                    if (Matrix4* pMatOffset = pBoneSrt->FindMember<Matrix4>("matOffset"))
                            bone.m_matOffsetMatrix = *pMatOffset;

                    if (MVariantArray* pChildren = pBoneSrt->FindMember<MVariantArray>("Children"))
                    {
						unsigned int unChildrenCount = pChildren->GetMemberCount();
						bone.vChildrenIndices.resize(unChildrenCount);

						for (unsigned int cldIdx = 0; cldIdx < unChildrenCount; ++cldIdx)
						{
                            if(int* pIndex = pChildren->GetMember<int>(cldIdx))
							    bone.vChildrenIndices[cldIdx] = *pIndex;
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
        const std::vector<MBone>& vBones = m_pSkeletonTemplate->GetAllBones();
        
        MVariant var = MVariantArray();
        MVariantArray* pArray = var.GetArray();
        pArray->Resize(vBones.size());

        for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
        {
            const MBone& bone = vBones[i];
            (*pArray)[i] = MStruct();
            MStruct& srt = *(*pArray)[i].GetStruct();

			MVariantArray vChildren;
			for (unsigned int unChildIdx : bone.vChildrenIndices)
				vChildren.AppendMVariant(unChildIdx);

            srt.AppendMVariant("Name", bone.strName);
            srt.AppendMVariant("Index", bone.unIndex);
            srt.AppendMVariant("ParentIndex", bone.unParentIndex);
            srt.AppendMVariant("matTrans", bone.m_matTransform);
            srt.AppendMVariant("matOffset", bone.m_matOffsetMatrix);
            srt.AppendMVariant("Children", vChildren);         
        }

        return true;
    }

    return false;
}

