/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"

namespace morty
{

class MResourceLoader;
class MResourceSystem;
class MResourceRef;
class MEngine;
class MObject;

class MORTY_API MResourceData
{
public:
                               MResourceData() = default;

    virtual ~                  MResourceData() = default;

    virtual void               LoadBuffer(const std::vector<MByte>& buffer) = 0;

    virtual std::vector<MByte> SaveBuffer() const = 0;
};

class MORTY_API MFbResourceData : public MResourceData
{
public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const = 0;

    virtual void                      Deserialize(const void* pBufferPointer) = 0;

    void                              LoadBuffer(const std::vector<MByte>& buffer) override;

    [[nodiscard]] std::vector<MByte>  SaveBuffer() const override;
};

class MORTY_API MYamlResourceData : public MResourceData
{
public:
    virtual YAML::Node               Serialize() const                   = 0;
    virtual void                     Deserialize(const YAML::Node& node) = 0;

    void                             LoadBuffer(const std::vector<MByte>& buffer) override;
    [[nodiscard]] std::vector<MByte> SaveBuffer() const override;
};

class MORTY_API MTextResourceData : public MResourceData
{
public:
    virtual void                     Serialize(std::vector<MByte>& output) const   = 0;
    virtual void                     Deserialize(const std::vector<MByte>& buffer) = 0;

    void                             LoadBuffer(const std::vector<MByte>& buffer) override;
    [[nodiscard]] std::vector<MByte> SaveBuffer() const override;
};

class MORTY_API MResource : public MTypeClass
{
    MORTY_INTERFACE(MResource)
public:
                               MResource();

    virtual ~                  MResource();

    static MString             GetSuffix(const MString& strPath);

    static MString             GetFolder(const MString& strPath);

    static MString             GetFileName(const MString& strPath);

    MEngine*                   GetEngine() const { return m_engine; }

    MResourceSystem*           GetResourceSystem();

    MResourceID                GetResourceID() const { return m_unResourceID; }

    const MString&             GetResourcePath() const { return m_strResourcePath; }

    std::shared_ptr<MResource> GetShared() const;

#if MORTY_DEBUG

    const char* GetDebugName() const;

#endif

public:
    virtual void OnCreated() {}

    virtual void OnDelete() {}

    virtual bool Load(std::unique_ptr<MResourceData>&& pResourceData)
    {
        MORTY_UNUSED(pResourceData);
        return false;
    };

    virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData)
    {
        MORTY_UNUSED(pResourceData);
        return false;
    }

    void OnPreLoad();
    void OnPostLoad();

protected:
    friend class MResourceSystem;

    friend class MResourceLoader;

    friend class MResourceRef;

    MString                    m_strResourcePath;
    MResourceID                m_unResourceID;
    MEngine*                   m_engine;

    std::vector<MResourceRef*> m_owner;

    std::weak_ptr<MResource>   m_self;
};

class MORTY_API MResourceRef final
{
public:
    typedef std::function<void()> ResourceLoadCallback;

public:
                                             MResourceRef() = default;

    explicit                                 MResourceRef(std::shared_ptr<MResource> pResource);

                                             MResourceRef(const MResourceRef& cHolder);

    virtual ~                                MResourceRef();

    [[nodiscard]] MString                    GetResourcePath() const;

    void                                     SetResource(std::shared_ptr<MResource> pResource);

    [[nodiscard]] std::shared_ptr<MResource> GetResource() const { return m_resource; }

    [[nodiscard]] MHashCode                  GetHashCode() const;

    MResourceRef&                            operator=(const MResourceRef& keeper);

    bool                                     operator==(const MResourceRef& other) const;

    template<class T> std::shared_ptr<T>     GetResource() const;

    void SetPreLoadCallback(const ResourceLoadCallback& function) { m_funcPreFunction = function; }
    void SetPostLoadCallback(const ResourceLoadCallback& function) { m_funcPostLoadFunction = function; }

public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                      Deserialize(MResourceSystem* resourceSystem, const void* pBufferPointer);

private:
    friend class MResource;

    ResourceLoadCallback       m_funcPostLoadFunction = nullptr;
    ResourceLoadCallback       m_funcPreFunction      = nullptr;
    std::shared_ptr<MResource> m_resource             = nullptr;
};

template<class T> std::shared_ptr<T> MResourceRef::GetResource() const
{
    return m_resource ? std::dynamic_pointer_cast<T>(m_resource) : nullptr;
}

}// namespace morty