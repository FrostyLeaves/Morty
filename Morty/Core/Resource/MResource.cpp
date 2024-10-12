#include "Resource/MResource.h"

#include <Flatbuffer/MResourceRef_generated.h>

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MResource, MTypeClass)

void MFbResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushBytes((const uint8_t*) buffer.data(), buffer.size());
    Deserialize(fbb.GetCurrentBufferPointer());
}

std::vector<MByte> MFbResourceData::SaveBuffer() const
{
    flatbuffers::FlatBufferBuilder fbb;
    auto                           fbData = Serialize(fbb);
    fbb.Finish(fbData);

    std::vector<MByte> data(fbb.GetSize());
    memcpy(data.data(), (MByte*) fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

    return data;
}


MResource::MResource()
    : m_unResourceID(0)
    , m_engine(nullptr)
{}

MResource::~MResource()
{
    for (MResourceRef* pKeeper: m_keeper) { pKeeper->m_resource = nullptr; }
}

MString MResource::GetSuffix(const MString& strPath)
{
    size_t index = strPath.find_last_of('.');
    if (index >= strPath.size()) return "";

    MString suffix = strPath.substr(index + 1, strPath.size());
    for (char& c: suffix)
    {
        if ('A' <= c && c <= 'Z') c += ('a' - 'A');
    }

    return suffix;
}

MString MResource::GetFolder(const MString& strPath)
{
    MString strRegularPath = strPath;
    for (MString::reverse_iterator iter = strRegularPath.rbegin();
         iter != strRegularPath.rend();
         ++iter)
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
    for (MString::reverse_iterator iter = strRegularPath.rbegin();
         iter != strRegularPath.rend();
         ++iter)
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
    if (nullptr == m_engine) return nullptr;

    if (MISystem* pSystem = m_engine->FindSystem(MResourceSystem::GetClassType()))
    {
        return pSystem->template DynamicCast<MResourceSystem>();
    }

    return nullptr;
}

std::shared_ptr<MResource> MResource::GetShared() const { return m_self.lock(); }

void                       MResource::ReplaceFrom(std::shared_ptr<MResource> pResource)
{
    if (pResource->GetType() != GetType()) return;

    std::vector<MResourceRef*> keeps = m_keeper;
    m_keeper.clear();

    for (MResourceRef* pKeeper: keeps)
    {
        //pKeeper->SetResource(pResource);

        pKeeper->m_resource = pResource;
        pResource->m_keeper.push_back(pKeeper);

        if (pKeeper->m_funcReloadCallback) { pKeeper->m_funcReloadCallback(); }
    }
}

#if MORTY_DEBUG

const char* MResource::GetDebugName() const { return GetResourcePath().c_str(); }

#endif

void MResource::OnReload()
{
    for (MResourceRef* pKeeper: m_keeper)
    {
        if (pKeeper->m_funcReloadCallback) pKeeper->m_funcReloadCallback();
    }
}

MResourceRef::MResourceRef()
    : m_funcReloadCallback(nullptr)
    , m_resource(nullptr)
{}

MResourceRef::MResourceRef(std::shared_ptr<MResource> pResource)
    : m_funcReloadCallback(nullptr)
    , m_resource(nullptr)
{
    SetResource(pResource);
}

MResourceRef::MResourceRef(const MResourceRef& cHolder)
    : m_funcReloadCallback(cHolder.m_funcReloadCallback)
    , m_resource(nullptr)
{
    SetResource(cHolder.m_resource);
}

MResourceRef::~MResourceRef() { SetResource(nullptr); }

void MResourceRef::SetResource(std::shared_ptr<MResource> pResource)
{
    std::shared_ptr<MResource> pOldResource = m_resource;
    if (m_resource)
    {
        std::vector<MResourceRef*>::iterator iter =
                std::find(m_resource->m_keeper.begin(), m_resource->m_keeper.end(), this);
        if (m_resource->m_keeper.end() != iter) { m_resource->m_keeper.erase(iter); }
    }

    m_resource = pResource;
    if (m_resource) { m_resource->m_keeper.push_back(this); }
}

const MResourceRef& MResourceRef::operator=(const MResourceRef& keeper)
{
    m_funcReloadCallback = keeper.m_funcReloadCallback;
    SetResource(keeper.m_resource);

    return keeper;
}

std::shared_ptr<MResource> MResourceRef::operator=(std::shared_ptr<MResource> pResource)
{
    m_funcReloadCallback = nullptr;
    SetResource(pResource);

    return pResource;
}

flatbuffers::Offset<void> MResourceRef::Serialize(flatbuffers::FlatBufferBuilder& fbb
) const
{
    if (!m_resource) { return {}; }

    auto                     fbPath = fbb.CreateString(m_resource->GetResourcePath());

    fbs::MResourceRefBuilder builder(fbb);

    builder.add_path(fbPath);

    return builder.Finish().Union();
}

void MResourceRef::Deserialize(
        MResourceSystem* pResourceSystem,
        const void*      pBufferPointer
)
{
    const fbs::MResourceRef* fbData =
            reinterpret_cast<const fbs::MResourceRef*>(pBufferPointer);
    if (!fbData) { return; }

    std::shared_ptr<MResource> pResource = nullptr;
    if (fbData->path())
    {
        MString strPath = fbData->path()->str();
        pResource       = pResourceSystem->LoadResource(strPath);
    }

    SetResource(pResource);
}
