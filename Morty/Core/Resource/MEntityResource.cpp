#include "Resource/MEntityResource.h"

#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEntityResource, MResource)

MEntityResource::MEntityResource()
{

}

MEntityResource::~MEntityResource()
{

}

const MByte* MEntityResource::GetData() const
{
	if (auto ptr = static_cast<MEntityResourceData*>(m_pResourceData.get()))
	{
		return ptr->aEntityData.data();
	}

	return nullptr;
}

size_t MEntityResource::GetSize() const
{
	if (auto ptr = static_cast<MEntityResourceData*>(m_pResourceData.get()))
	{
		return ptr->aEntityData.size();
	}

	return 0;
}

bool MEntityResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
	m_pResourceData = std::move(pResourceData);
	return true;
}

bool MEntityResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	pResourceData = std::make_unique<MEntityResourceData>(*static_cast<MEntityResourceData*>(m_pResourceData.get()));
	return true;
}

const MType* MEntityResourceLoader::ResourceType() const
{
	return MEntityResource::GetClassType();
}

std::unique_ptr<MResourceData> MEntityResourceLoader::LoadResource(const MString& svFullPath)
{
	auto pResourceData = std::make_unique<MEntityResourceData>();

	if (!MFileHelper::ReadData(svFullPath, pResourceData->aEntityData))
	{
		return nullptr;
	}

	return pResourceData;
}


void MEntityResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
	aEntityData = buffer;
}

std::vector<MByte> MEntityResourceData::SaveBuffer() const
{
	return aEntityData;
}