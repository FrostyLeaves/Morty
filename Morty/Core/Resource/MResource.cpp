#include "Resource/MResource.h"

#include <Flatbuffer/MResourceRef_generated.h>

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

MORTY_INTERFACE_IMPLEMENT(MResource, MTypeClass)


void MFbResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)buffer.data(), buffer.size());
	Deserialize(fbb.GetCurrentBufferPointer());
}

std::vector<MByte> MFbResourceData::SaveBuffer() const
{
	flatbuffers::FlatBufferBuilder fbb;
	auto fbData = Serialize(fbb);
	fbb.Finish(fbData);

	std::vector<MByte> data(fbb.GetSize());
	memcpy(data.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

	return data;
}


MResource::MResource()
: m_unResourceID(0)
, m_pEngine(nullptr)
{
    
}

MResource::~MResource()
{
	for (MResourceRef* pKeeper : m_vKeeper)
	{
		pKeeper->m_pResource = nullptr;
	}
}

MString MResource::GetSuffix(const MString& strPath)
{
	size_t index = strPath.find_last_of('.');
	if (index >= strPath.size())
		return "";

	MString suffix = strPath.substr(index + 1, strPath.size());
	for (char& c : suffix)
	{
		if ('A' <= c && c <= 'Z')
			c += ('a' - 'A');
	}

	return suffix;
}

MString MResource::GetFolder(const MString& strPath)
{
	MString strRegularPath = strPath;
	for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
	{
		if (*iter == '\\' || *iter == '/')
		{
			return MString(strRegularPath.begin(), iter.base() - 1);
		}
	}

	return MString();
}

MString MResource::GetFileName(const MString& strPath)
{
	MString strRegularPath = strPath;
	for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
	{
		if (*iter == '\\' || *iter == '/')
		{
			return MString(iter.base(), strRegularPath.end());
		}
	}

	return strPath;
}

MResourceSystem* MResource::GetResourceSystem()
{
	if (nullptr == m_pEngine)
		return nullptr;

	if (MISystem* pSystem = m_pEngine->FindSystem(MResourceSystem::GetClassType()))
	{
		return pSystem->template DynamicCast<MResourceSystem>();
	}

	return nullptr;
}

std::shared_ptr<MResource> MResource::GetShared() const
{
	return m_self.lock();
}

void MResource::ReplaceFrom(std::shared_ptr<MResource> pResource)
{
	if (pResource->GetType() != GetType())
		return;

	std::vector<MResourceRef*> keeps = m_vKeeper;
	m_vKeeper.clear();

	for (MResourceRef* pKeeper : keeps)
	{
		//pKeeper->SetResource(pResource);

		pKeeper->m_pResource = pResource;
		pResource->m_vKeeper.push_back(pKeeper);

		if (pKeeper->m_funcReloadCallback)
		{
			pKeeper->m_funcReloadCallback();
		}
	}
}

#if MORTY_DEBUG
const char* MResource::GetDebugName() const
{
	return GetResourcePath().c_str();
}
#endif

void MResource::OnReload()
{
	for (MResourceRef* pKeeper : m_vKeeper)
	{
		if (pKeeper->m_funcReloadCallback)
			pKeeper->m_funcReloadCallback();
	}
}

MResourceRef::MResourceRef()
	: m_funcReloadCallback(nullptr)
	, m_pResource(nullptr)
{

}

MResourceRef::MResourceRef(std::shared_ptr<MResource> pResource)
	: m_funcReloadCallback(nullptr)
	, m_pResource(nullptr)
{
	SetResource(pResource);
}

MResourceRef::MResourceRef(const MResourceRef& cHolder)
	: m_funcReloadCallback(cHolder.m_funcReloadCallback)
	, m_pResource(nullptr)
{
	SetResource(cHolder.m_pResource);
}

MResourceRef::~MResourceRef()
{
	SetResource(nullptr);
}

void MResourceRef::SetResource(std::shared_ptr<MResource> pResource)
{
	std::shared_ptr<MResource> pOldResource = m_pResource;
	if (m_pResource)
	{
		std::vector<MResourceRef*>::iterator iter = std::find(m_pResource->m_vKeeper.begin(), m_pResource->m_vKeeper.end(), this);
		if (m_pResource->m_vKeeper.end() != iter)
		{
			m_pResource->m_vKeeper.erase(iter);
		}
	}

	m_pResource = pResource;
	if (m_pResource)
	{
		m_pResource->m_vKeeper.push_back(this);
	}
}

const MResourceRef& MResourceRef::operator=(const MResourceRef& keeper)
{
	m_funcReloadCallback = keeper.m_funcReloadCallback;
	SetResource(keeper.m_pResource);

	return keeper;
}

std::shared_ptr<MResource> MResourceRef::operator=(std::shared_ptr<MResource> pResource)
{
	m_funcReloadCallback = nullptr;
	SetResource(pResource);

	return pResource;
}

flatbuffers::Offset<void> MResourceRef::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	if (!m_pResource)
	{
		return {};
	}

	auto fbPath = fbb.CreateString(m_pResource->GetResourcePath());

	mfbs::MResourceRefBuilder builder(fbb);
	
	builder.add_path(fbPath);

	return builder.Finish().Union();
}

void MResourceRef::Deserialize(MResourceSystem* pResourceSystem, const void* pBufferPointer)
{
	const mfbs::MResourceRef* fbData = reinterpret_cast<const mfbs::MResourceRef*>(pBufferPointer);
	if(!fbData)
	{
		return;
	}

	std::shared_ptr<MResource> pResource = nullptr;
	if (fbData->path())
	{
		MString strPath = fbData->path()->str();
		pResource = pResourceSystem->LoadResource(strPath);
	}

	SetResource(pResource);
}
