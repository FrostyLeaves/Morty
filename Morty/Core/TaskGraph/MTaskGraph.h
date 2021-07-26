/**
 * @File         MTaskGraph
 * 
 * @Created      2021-07-08 11:26:02
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTASKGRAPH_H_
#define _M_MTASKGRAPH_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MTaskNode.h"
#include "MTaskNodeInput.h"
#include "MTaskNodeOutput.h"

class MTaskNode;
class MORTY_API MTaskGraph : public MObject
{
    MORTY_CLASS(MTaskGraph)

public:
    MTaskGraph();
    virtual ~MTaskGraph();

public:


    template<typename TYPE>
    TYPE* AddNode(const MString& strNodeName);

    MTaskNode* FindNode(const MString& strNodeName) const;

	bool IsValid() const { return m_bValid; }

    virtual bool Compile();

	void RequireCompile() { m_bRequireCompile = true; }
	bool NeedCompile() const { return m_bRequireCompile; }

	void Run();

public:

	const std::vector<MTaskNode*>& GetStartNodes() { return m_vStartTaskNode; }
	const std::vector<MTaskNode*>& GetFinalNodes() { return m_vFinalTaskNode; }
	const std::vector<MTaskNode*> GetAllNodes() { return m_vTaskNode; }
protected:

	bool AddNode(const MString& strNodeName, MTaskNode* pGraphNode);


protected:

	std::map<MString, MTaskNode*> m_tTaskNode;
    std::vector<MTaskNode*> m_vTaskNode;

	std::vector<MTaskNode*> m_vStartTaskNode;
	std::vector<MTaskNode*> m_vFinalTaskNode;

	bool m_bRequireCompile;
	bool m_bValid;
};

template<typename TYPE>
TYPE* MTaskGraph::AddNode(const MString& strNodeName)
{
	if (!MTypeClass::IsType<TYPE, MTaskNode>())
	{
		return nullptr;
	}

	TYPE* pNode = new TYPE();
	if (!AddNode(strNodeName, pNode))
	{
		delete pNode;
		pNode = nullptr;
	}

	return pNode;
}


#endif
