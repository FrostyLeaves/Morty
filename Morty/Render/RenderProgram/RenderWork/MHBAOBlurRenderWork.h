/**
 * @File         MHBAOBlurRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBlurRenderWork.h"

namespace morty
{

class MORTY_API MHBAOBlurRenderWorkV : public MBlurRenderWork
{
public:
    static MStringId                   BlurOutput;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

class MORTY_API MHBAOBlurRenderWorkH : public MBlurRenderWork
{
public:
    static MStringId                   BlurOutput;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty