﻿/**
 * @File         MResourceLoader
 * 
 * @Created      2019-08-06 17:59:45
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"
#include "Utility/MString.h"

namespace morty
{

class MResourceSystem;
class MORTY_API MResourceLoader
{
public:
    MResourceLoader() = default;

    virtual ~MResourceLoader() = default;

    virtual const MType*                   ResourceType() const = 0;

    virtual std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath) = 0;

    MString                                strResourcePath;
    MString                                strResourceFullPath;

    std::shared_ptr<MResource>             pResource     = nullptr;
    std::unique_ptr<MResourceData>         pResourceData = nullptr;
};

template<typename RESOURCE_TYPE, typename RESOURCE_DATA_TYPE> class MORTY_API MResourceLoaderTemplate
    : public MResourceLoader
{
public:
    const MType*                   ResourceType() const override { return RESOURCE_TYPE::GetClassType(); }

    std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath) override
    {
        std::unique_ptr<RESOURCE_DATA_TYPE> pResourceData = std::make_unique<RESOURCE_DATA_TYPE>();

        std::vector<MByte>                  data;
        MFileHelper::ReadData(svFullPath, data);

        pResourceData->LoadBuffer(data);
        return pResourceData;
    }
};

}// namespace morty