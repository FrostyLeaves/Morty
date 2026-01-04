#include "Resource/MResource.h"

#include <Flatbuffer/MResourceRef_generated.h>

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"
#include "Utility/MUtils.h"

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

void MYamlResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
    if (buffer.empty()) return;

    // Parse YAML from buffer
    std::string yamlStr(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    YAML::Node  root;

    try
    {
        root = YAML::Load(yamlStr);
    } catch (const YAML::Exception& e)
    {
        // Handle parse error
        return;
    }

    Deserialize(root);
}

std::vector<MByte> MYamlResourceData::SaveBuffer() const
{
    auto          root = Serialize();

    // Emit to string
    YAML::Emitter emitter;
    emitter << root;

    // Copy to output buffer
    std::vector<MByte> output;
    const std::string  yamlStr = emitter.c_str();
    output.resize(yamlStr.size());
    std::memcpy(output.data(), yamlStr.data(), yamlStr.size());

    return output;
}

void               MTextResourceData::LoadBuffer(const std::vector<MByte>& buffer) { Deserialize(buffer); }

std::vector<MByte> MTextResourceData::SaveBuffer() const
{
    std::vector<MByte> output;
    Serialize(output);

    return output;
}

MResource::MResource()
    : m_unResourceID(0)
    , m_engine(nullptr)
{}

MResource::~MResource()
{
    for (MResourceRef* owner: m_owner) { owner->m_resource = nullptr; }
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
    for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
    {
        if (*iter == '\\' || *iter == '/') { return MString(strRegularPath.begin(), iter.base() - 1); }
    }

    return MString();
}

MString MResource::GetFileName(const MString& strPath)
{
    MString strRegularPath = strPath;
    for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
    {
        if (*iter == '\\' || *iter == '/') { return MString(iter.base(), strRegularPath.end()); }
    }

    return strPath;
}

MResourceSystem* MResource::GetResourceSystem()
{
    if (nullptr == m_engine) return nullptr;

    if (MISystem* pSystem = m_engine->GetSystem(MResourceSystem::GetClassType()))
    {
        return pSystem->template DynamicCast<MResourceSystem>();
    }

    return nullptr;
}

std::shared_ptr<MResource> MResource::GetShared() const { return m_self.lock(); }

#if MORTY_DEBUG

const char* MResource::GetDebugName() const { return GetResourcePath().c_str(); }

#endif

void MResource::OnPreLoad()
{
    for (MResourceRef* owner: m_owner)
    {
        if (owner->m_funcPreFunction) owner->m_funcPreFunction();
    }
}


void MResource::OnPostLoad()
{
    for (MResourceRef* owner: m_owner)
    {
        if (owner->m_funcPostLoadFunction) owner->m_funcPostLoadFunction();
    }
}

MResourceRef:: MResourceRef(std::shared_ptr<MResource> pResource) { SetResource(pResource); }

MResourceRef:: MResourceRef(const MResourceRef& other) { SetResource(other.m_resource); }

MResourceRef::~MResourceRef() { SetResource(nullptr); }

MString        MResourceRef::GetResourcePath() const { return m_resource ? m_resource->GetResourcePath() : ""; }

void           MResourceRef::SetResource(std::shared_ptr<MResource> pResource)
{
    std::shared_ptr<MResource> pOldResource = m_resource;
    if (m_resource)
    {
        std::vector<MResourceRef*>::iterator iter =
                std::find(m_resource->m_owner.begin(), m_resource->m_owner.end(), this);
        if (m_resource->m_owner.end() != iter) { m_resource->m_owner.erase(iter); }
    }

    m_resource = pResource;
    if (m_resource) { m_resource->m_owner.push_back(this); }
}

MHashCode     MResourceRef::GetHashCode() const { return MUtils::Hash(GetResourcePath()); }

MResourceRef& MResourceRef::operator=(const MResourceRef& keeper)
{
    SetResource(keeper.m_resource);
    return *this;
}

bool MResourceRef::operator==(const MResourceRef& other) const { return GetResource() == other.GetResource(); }

flatbuffers::Offset<void> MResourceRef::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    if (!m_resource) { return {}; }

    auto                     fbPath = fbb.CreateString(m_resource->GetResourcePath());

    fbs::MResourceRefBuilder builder(fbb);

    builder.add_path(fbPath);

    return builder.Finish().Union();
}

void MResourceRef::Deserialize(MResourceSystem* resourceSystem, const void* pBufferPointer)
{
    const fbs::MResourceRef* fbData = reinterpret_cast<const fbs::MResourceRef*>(pBufferPointer);
    if (!fbData) { return; }

    std::shared_ptr<MResource> pResource = nullptr;
    if (fbData->path())
    {
        MString strPath = fbData->path()->str();
        pResource       = resourceSystem->LoadResource(strPath);
    }

    SetResource(pResource);
}
