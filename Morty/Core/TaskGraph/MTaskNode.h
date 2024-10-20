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
    ~MTaskNode() override;

public:
    [[nodiscard]] MStringId                         GetNodeName() const { return m_strNodeName; }

    template<typename TYPE = MTaskNodeInput> TYPE*  AppendInput();

    template<typename TYPE = MTaskNodeOutput> TYPE* AppendOutput();

    void                                            SetThreadType(const METhreadType& eType) { m_threadType = eType; }
    [[nodiscard]] size_t                            GetNodeID() const { return m_id; }
    [[nodiscard]] METhreadType                      GetThreadType() const { return m_threadType; }
    [[nodiscard]] size_t                            GetInputSize() const { return m_input.size(); }
    [[nodiscard]] size_t                            GetOutputSize() const { return m_output.size(); }
    [[nodiscard]] MTaskNodeInput*                   GetInput(const size_t& nInputIdx) const;
    [[nodiscard]] MTaskNodeOutput*                  GetOutput(const size_t& nOutputIdx) const;
    [[nodiscard]] MTaskGraph*                       GetGraph() const { return m_graph; }

    void                                            ConnectTo(MTaskNode* pNextNode);
    void                                            DisconnectTo(MTaskNode* pNextNode);
    void                                            DisconnectAll();

    bool                                            IsStartNode();
    bool                                            IsFinalNode();

    void                                            Run();

    void         BindTaskFunction(const std::function<void(MTaskNode*)>& func) { m_funcTaskFunction = func; };

    virtual void OnCreated() {}
    virtual void OnCompile() {}
    virtual void OnDelete() {}

    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb);
    virtual void                      Deserialize(const void* flatbuffer);

    static const size_t               InvalidSlotId;

private:
    void AppendInput(MTaskNodeInput* pInput);
    void AppendOutput(MTaskNodeOutput* pOutput);

protected:
    friend class MTaskGraph;

    MStringId                       m_strNodeName;
    MTaskGraph*                     m_graph         = nullptr;
    size_t                          m_priorityLevel = 0;
    size_t                          m_id            = 0;
    METhreadType                    m_threadType    = METhreadType::EAny;
    std::vector<MTaskNodeInput*>    m_input;
    std::vector<MTaskNodeOutput*>   m_output;
    std::function<void(MTaskNode*)> m_funcTaskFunction = nullptr;

#if MORTY_DEBUG
public:
    long long m_debugTime = 0;
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