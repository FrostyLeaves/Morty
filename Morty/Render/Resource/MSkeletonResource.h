/**
 * @File         MSkeletonResource
 * 
 * @Created      2020-06-02 01:50:44
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Model/MSkeleton.h"
#include "Resource/MResourceLoader.h"


struct MORTY_API MSkeletonResourceData : public MFbResourceData
{
public:
	MSkeleton skeleton;

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;
	void Deserialize(const void* pBufferPointer) override;
};

class MORTY_API MSkeletonResource : public MResource
{
public:
	MORTY_CLASS(MSkeletonResource)
	MSkeletonResource() = default;
	virtual ~MSkeletonResource() = default;

	MSkeleton* GetSkeleton() const;

	bool Load(std::unique_ptr<MResourceData>&& pResourceData) override;
	bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:

	std::unique_ptr<MResourceData> m_pResourceData = nullptr;
};

class MORTY_API MSkeletonResourceLoader : public MResourceLoaderTemplate<MSkeletonResource, MSkeletonResourceData>
{
public:

	static MString GetResourceTypeName() { return "Skeleton"; };
	static std::vector<MString> GetSuffixList() { return { "ske" }; };
};
