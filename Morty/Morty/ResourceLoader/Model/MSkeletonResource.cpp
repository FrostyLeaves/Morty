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

                    if(MString *pName = pBoneSrt->FindMember<MString>("Name"))
                        pBone->strName = *pName;
                    
                    if (int* pIndex = pBoneSrt->FindMember<int>("Index"))
                        pBone->unIndex = *pIndex;
                    
                    if (int* pParentIndex = pBoneSrt->FindMember<int>("ParentIndex"))
                        pBone->unParentIndex = *pParentIndex;
                    
                    if (Matrix4* pMatTrans = pBoneSrt->FindMember<Matrix4>("matTrans"))
                        pBone->m_matTransform = *pMatTrans;
                    
                    if (Matrix4* pMatOffset = pBoneSrt->FindMember<Matrix4>("matOffset"))
                            pBone->m_matOffsetMatrix = *pMatOffset;

                    if (MVariantArray* pChildren = pBoneSrt->FindMember<MVariantArray>("Children"))
                    {
						unsigned int unChildrenCount = pChildren->GetMemberCount();
						pBone->vChildrenIndices.resize(unChildrenCount);

						for (unsigned int cldIdx = 0; cldIdx < unChildrenCount; ++cldIdx)
						{
                            if(int* pIndex = pChildren->GetMember<int>(cldIdx))
							    pBone->vChildrenIndices[cldIdx] = *pIndex;
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

