/**
 * @File         MTaskNode
 * 
 * @Created      2021-07-08 11:41:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "Thread/MThreadWork.h"
#include "Type/MType.h"
#include "Utility/MStringId.h"

namespace morty
{

class MTaskGraph;
class MTaskNodeInput;
class MTaskNodeOutput;
class MORTY_API MTaskNode : public MTypeClass
{
    MORTY_CLASS(MTaskNode)
public:
    MTaskNode() = default;

    virtual ~MTaskNode();

public:
    MStringId                                       GetNodeName() const { return m_strNodeName; }

    template<typename TYPE = MTaskNodeInput> TYPE*  AppendInput();

    template<typename TYPE = MTaskNodeOutput> TYPE* AppendOutput();

    void                                            SetThreadType(const METhreadType& eType) { m_threadType = eType; }

    METhreadType                                    GetThreadType() const { return m_threadType; }

    size_t                                          GetInputSize() const { return m_input.size(); }

    size_t                                          GetOutputSize() const { return m_output.size(); }

    MTaskNodeInput*                                 GetInput(const size_t& nInputIdx);

    MTaskNodeOutput*                                GetOutput(const size_t& nOutputIdx);

    void                                            ConnectTo(MTaskNode* pNextNode);

    void                                            DisconnectTo(MTaskNode* pNextNode);

    void                                            DisconnectAll();

    MTaskGraph*                                     GetGraph() const { return m_graph; }

    bool                                            IsStartNode();

    bool                                            IsFinalNode();


    void                                            Run();

public:
    virtual void OnCreated();

    virtual void OnCompile();

    virtual void OnDelete();

public:
    void BindTaskFunction(const std::function<void(MTaskNode*)>& func) { m_funcTaskFunction = func; };

private:
    void AppendInput(MTaskNodeInput* pInput);

    void AppendOutput(MTaskNodeOutput* pOutput);

protected:
    friend class MTaskGraph;

    MStringId                       m_strNodeName;
    MTaskGraph*                     m_graph         = nullptr;
    size_t                          m_priorityLevel = 0;
    METhreadType                    m_threadType    = METhreadType::EAny;

    std::vector<MTaskNodeInput*>    m_input;
    std::vector<MTaskNodeOutput*>   m_output;

    std::function<void(MTaskNode*)> m_funcTaskFunction = nullptr;

#if MORTY_DEBUG
public:
    long long m_debugTime;
#endif
};

template<typename TYPE> inline TYPE* MTaskNode::AppendInput()
{
    if (!MTypeClass::IsType<TYPE, MTaskNodeInput>()) return nullptr;

    TYPE* pInput = new TYPE();
    AppendInput(pInput);
    return pInput;
}

template<typename TYPE /*= MTaskNodeOutput*/> TYPE* MTaskNode::AppendOutput()
{
    if (!MTypeClass::IsType<TYPE, MTaskNodeOutput>()) return nullptr;

    TYPE* pOutput = new TYPE();
    AppendOutput(pOutput);
    return pOutput;
}

}// namespace morty