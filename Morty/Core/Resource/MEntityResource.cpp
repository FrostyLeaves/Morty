#include "Resource/MEntityResource.h"

#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

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

bool MEntityResource::Load(std::unique_ptr<MResourceData>& pResourceData)
{
	m_pResourceData = std::move(pResourceData);
	return true;
}

bool MEntityResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	pResourceData = std::make_unique<MEntityResourceData>(*static_cast<MEntityResourceData*>(m_pResourceData.get()));
	return true;
}

std::shared_ptr<MResource> MEntityResourceLoader::Create(MResourceSystem* pManager)
{
	return pManager->CreateResource<MEntityResource>();
}

std::unique_ptr<MResourceData> MEntityResourceLoader::LoadResource(const MString& svFullPath, const MString& svPath)
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