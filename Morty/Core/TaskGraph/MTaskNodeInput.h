/**
 * @File         MTaskNodeInput
 * 
 * @Created      2021-07-08 11:50:37
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Type/MType.h"
#include "Utility/MStringId.h"

namespace morty
{

class MTaskNode;
class MTaskNodeOutput;
class MORTY_API MTaskNodeInput : public MTypeClass
{
    MORTY_CLASS(MTaskNodeInput)
public:
    MTaskNodeInput();

    void                           LinkTo(MTaskNodeOutput* pOutput);

    void                           UnLink();


    [[nodiscard]] size_t           GetIndex() const { return m_unIndex; }
    [[nodiscard]] MString          GetStringID() const;

    void                           SetName(const MStringId& strName) { m_strName = strName; }
    [[nodiscard]] const MStringId& GetName() const { return m_strName; }

    [[nodiscard]] MTaskNode*       GetTaskNode() const { return pGraphNode; }
    [[nodiscard]] MTaskNode*       GetLinkedNode() const;
    [[nodiscard]] MTaskNodeOutput* GetLinkedOutput() const { return pLinkedOutput; }

private:
    friend class MTaskNode;
    friend class MTaskNodeOutput;

private:
    size_t           m_unIndex;
    MStringId        m_strName;
    MTaskNode*       pGraphNode;
    MTaskNodeOutput* pLinkedOutput;
};

}// namespace morty