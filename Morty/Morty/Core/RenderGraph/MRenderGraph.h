/**
 * @File         MRenderGraph
 * 
 * @Created      2021-01-27 16:21:44
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERGRAPH_H_
#define _M_MRENDERGRAPH_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MTexture.h"

#include <array>
#include <vector>
#include <functional>

class MRenderPass;
class MRenderGraph;
class MRenderGraphNode;
class MRenderGraphTexture;
class MRenderGraphNodeInput;
class MRenderGraphNodeOutput;


class MORTY_API MRenderGraph
{
public:
    MRenderGraph();
    MRenderGraph(MEngine* pEngine);
    virtual ~MRenderGraph();

public:

    template<typename T>
    T* AddRenderGraphNode(const MString& strNodeName);

    MRenderGraphNode* AddRenderGraphNode(const MString& strNodeName);

    MRenderGraphTexture* AddRenderGraphTexture(const MString& strTextureName);

    MRenderGraphTexture* FindRenderGraphTexture(const MString& strTextureName) const;

    MRenderGraphNode* FindRenderGraphNode(const MString& strNodeName) const;

    MEngine* GetEngine() { return m_pEngine; }

	void SetFinalOutput(MRenderGraphNodeOutput* pFinalOutput);
    MRenderGraphNodeOutput* GetFinalOutput() const;
    MRenderGraphTexture* GetFinalOutputTexture() const;


    void AddRelationTexture(MRenderGraphTexture* pTexture);
    void RemoveRelationTexture(MRenderGraphTexture* pTexture);
    void UpdateTextureSize(MRenderGraphTexture* pTexture);

    void SetOutputSize(const Vector2& v2Size);

    bool GetCompiled() const { return m_bCompiled; }
	void CompileDirty();
	bool Compile(MIDevice* pDevice);

    void Release();

    const std::vector<MRenderGraphNode*>& GetAllNodes() const { return m_vSortedNodes; }

	void Render();

protected:

	bool AddRenderGraphNode(MRenderGraphNode* pGraphNode, const MString& strNodeName);

protected:
    std::map<MString, MRenderGraphNode*> m_tGraphNodeMap;
    std::map<MString, MRenderGraphTexture*> m_tGraphTextureMap;

    
    std::vector<MRenderGraphNode*> m_vSortedNodes;
    std::vector<MRenderGraphTexture*> m_vRelationTextures;

    Vector2 m_v2OutputSize;

    bool m_bCompiled;
    MEngine* m_pEngine;

    MRenderGraphNodeOutput* m_pFinalOutput;
};

template<typename T>
T* MRenderGraph::AddRenderGraphNode(const MString& strNodeName)
{
    if (!MTypedClass::IsType<T, MRenderGraphNode>())
    {
        return nullptr;
    }

    T* pNode = new T();
    if (!AddRenderGraphNode(pNode, strNodeName))
	{
		delete pNode;
		pNode = nullptr;
    }
    
    return pNode;
}

#endif
