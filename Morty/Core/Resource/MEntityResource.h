/**
 * @File         MEntityResource
 * 
 * @Created      2021-07-20 10:48:24
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"

#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"

struct MORTY_API MEntityResourceData : public MResourceData
{
public:
	//RawData
    std::vector<MByte> aEntityData;

    void LoadBuffer(const std::vector<MByte>& buffer) override;
    std::vector<MByte> SaveBuffer() const override;
};

class MORTY_API MEntityResource : public MResource
{
    MORTY_CLASS(MEntityResource)
public:
    MEntityResource();
    virtual ~MEntityResource();

    const MByte* GetData() const;
    size_t GetSize() const;
    
protected:

    bool Load(std::unique_ptr<MResourceData>&& pResourceData) override;
    bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:

    friend class MEntitySystem;

    std::unique_ptr<MResourceData> m_pResourceData = nullptr;
};

class MORTY_API MEntityResourceLoader : public MResourceLoader
{
public:
    MEntityResourceLoader() = default;
    virtual ~MEntityResourceLoader() = default;

public:

    static MString GetResourceTypeName() { return "Entity"; }
    static std::vector<MString> GetSuffixList() { return { "entity" }; }

    const MType* ResourceType() const override;

    std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath) override;

};
