/**
 * @File         MRenderGraph
 * 
 * @Created      2021-01-27 16:21:44
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERGRAPH_H_
#define _M_MRENDERGRAPH_H_
#include "MGlobal.h"
#include "MTexture.h"


#include <vector>
#include <functional>

class MRenderPass;
class MTextureRenderTarget;
class MORTY_API MRenderGraphNode
{
public:

    MRenderGraphNode();
    MRenderGraphNode(const MString& strNodeName
        , const std::vector<size_t>& vDependNodes
        , const std::vector<size_t>& vInputTextures
        , const std::vector<size_t>& vOutputTextures);


public:

    MString m_strNodeName;

    std::vector<size_t> m_vDependNodeNames;

    std::vector<size_t> m_vInputTextureNames;
    std::vector<size_t> m_vOutputTextureNames;


    int m_nCommandLevel;

    std::function<void()> m_funcRender; // render 

 //   MRenderPass* m_pRenderPass;
 //   MTextureRenderTarget* m_pRenderTarget;
};

class MORTY_API MRenderGraphTexture
{

};

class MORTY_API MRenderGraph
{
public:

    MRenderGraph();
    virtual ~MRenderGraph();

public:

    void SetFinalNode(const size_t& nNodeIdx);

    size_t AddRenderGraphNode(MRenderGraphNode* pNode);

    size_t AddRenderGraphTexture(const MString& strTextureName);

    bool Compile();

protected:

    MRenderGraphNode* GetFinalNode();
    MRenderGraphNode* GetNode(const size_t& nIdx);

private:


    std::vector<MRenderGraphNode*> m_vGraphNodes;
    std::vector<MRenderGraphTexture*> m_vGraphTextures;

    std::vector<MRenderGraphNode*> m_vSortedNodes;

    size_t m_nFinalNodeIdx;
};


#endif
