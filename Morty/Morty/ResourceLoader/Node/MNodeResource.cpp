#include "MNodeResource.h"
#include "MFileHelper.h"

#include "MNode.h"
#include "MVariant.h"
#include "Json/MJson.h"

M_RESOURCE_IMPLEMENT(MNodeResource, MResource)

MNodeResource::MNodeResource()
    : MResource()
{
}

MNodeResource::~MNodeResource()
{
}

MNode* MNodeResource::CreateNode()
{
    MString code;
    MFileHelper::ReadString(GetResourcePath(), code);

	MVariant var;
	MJson::JsonToMVariant(code, var);

    if (MStruct* pStruct = var.GetStruct())
    {
        if (MNode* pNode = MNode::CreateNodeByVariant(m_pEngine, *pStruct))
        {
            pNode->Decode(code);
            return pNode;
        }
    }

    return nullptr;
}

void MNodeResource::SaveByNode(MNode* pNode)
{
    if (pNode)
    {
		MString code;
        pNode->Encode(code);

        MFileHelper::WriteString(GetResourcePath(), code);
    }
}

bool MNodeResource::Load(const MString& strResourcePath)
{
    return true;
}

