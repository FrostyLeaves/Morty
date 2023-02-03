#include "Resource/MResource.h"

#include <MResourceRef_generated.h>

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

MORTY_INTERFACE_IMPLEMENT(MResource, MTypeClass)

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
		return pSystem->DynamicCast<MResourceSystem>();
	}

	return nullptr;
}

std::shared_ptr<MResource> MResource::GetShared()
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
	
	if (m_pResource = pResource)
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
	flatbuffers::Offset<flatbuffers::String> fbPath;
	if (m_pResource)
	{
		fbPath = fbb.CreateString(m_pResource->GetResourcePath());
	}

	mfbs::MResourceRefBuilder builder(fbb);

	if (m_pResource)
	{
		builder.add_path(fbPath);
	}

	return builder.Finish().Union();
}

void MResourceRef::Deserialize(MEngine* pEngine, const void* pBufferPointer)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	const mfbs::MResourceRef* fbData = reinterpret_cast<const mfbs::MResourceRef*>(pBufferPointer);

	std::shared_ptr<MResource> pResource = nullptr;
	if (fbData->path())
	{
		pResource = pResourceSystem->LoadResource(fbData->path()->c_str());
	}

	SetResource(pResource);
}
