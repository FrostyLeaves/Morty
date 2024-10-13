/**
 * @File         MTaskNodeOutput
 * 
 * @Created      2021-07-08 11:48:17
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"
#include "Variant/MVariant.h"

namespace morty
{

class MTaskNode;
class MTaskNodeInput;
class MORTY_API MTaskNodeOutput : public MTypeClass
{
    MORTY_CLASS(MTaskNodeOutput)
public:
    MTaskNodeOutput();

    [[nodiscard]] size_t                              GetIndex() const { return m_unIndex; }
    [[nodiscard]] MString                             GetStringID() const;

    [[nodiscard]] MTaskNode*                          GetTaskNode() const { return pGraphNode; }

    [[nodiscard]] const std::vector<MTaskNodeInput*>& GetLinkedInputs() const { return vLinkedInput; }

    void                                              LinkTo(MTaskNodeInput* pInput);
    void                                              UnLink(MTaskNodeInput* pInput);


    void                                              SetName(const MStringId& strName) { m_strName = strName; }
    [[nodiscard]] const MStringId&                    GetName() const { return m_strName; }

private:
    friend class MTaskNode;

    size_t                       m_unIndex;
    MStringId                    m_strName;

    MTaskNode*                   pGraphNode;

    std::vector<MTaskNodeInput*> vLinkedInput;
};

}// namespace morty