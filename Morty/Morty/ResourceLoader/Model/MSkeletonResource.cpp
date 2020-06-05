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

        return;
    }
}

void MSkeletonResource::ReadFromStruct(MStruct& srt)
{

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

