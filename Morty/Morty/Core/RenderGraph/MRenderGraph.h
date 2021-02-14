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


class MORTY_API MRenderGraphNodeInput
{
public:

    void LinkTo(MRenderGraphNodeOutput* pOutput);

    void UnLink();

    MRenderGraphNode* GetLinkedNode() const;
    MRenderGraphTexture* GetLinkedTexture() const;
    MRenderGraphNodeOutput* GetLinkedOutput() const { return pLinkedOutput; }

private:
	friend class MRenderGraphNodeOutput;

private:
    MRenderGraphNodeOutput* pLinkedOutput;
};

class MORTY_API MRenderGraphNodeOutput
{
public:
    MRenderGraphNodeOutput();

    void SetClear(const bool& b) { bClear = b; }
    bool GetClear() const { return bClear; }

    void SetClearColor(const MColor& color) { mClearColor = color; }
    MColor GetClearColor() const { return mClearColor; }

    void SetRenderTexture(MRenderGraphTexture* pTexture);
    MRenderGraphTexture* GetRenderTexture() const { return pGraphTexture; }

    MRenderGraphNode* GetRenderGraphNode() { return pGraphNode; }

    void LinkTo(MRenderGraphNodeInput* pInput);
    void UnLink(MRenderGraphNodeInput* pInput);

private:

	friend class MRenderGraphNode;

    bool bClear;
    MColor mClearColor;

    MRenderGraphNode* pGraphNode;
    MRenderGraphTexture* pGraphTexture;

    std::vector<MRenderGraphNodeInput*> vLinkedInput;
};

class MORTY_API MRenderGraphTexture
{
public:
	MRenderGraphTexture();


	MRenderGraph* GetRenderGraph() const { return m_pGraph; }

	void SetUsage(const METextureUsage& eUsage);
	METextureUsage GetUsage() const { return m_eUsage; }

	void SetLayout(const METextureLayout& eLayout);
	METextureLayout GetLayout() const { return m_eLayout; }

	void SetSize(const Vector2& size);
	Vector2 GetSize() const { return m_v2Size; }

    void AddRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);
    void RemoveRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);

	MIRenderTexture* GetRenderTexture();

	void Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);



private:

	friend class MRenderGraph;

	MString m_strTextureName;
	METextureUsage m_eUsage;
	METextureLayout m_eLayout;
	Vector2 m_v2Size;

	MRenderTexture* m_pTexture;
    std::vector<MRenderGraphNodeOutput*> m_vOutputs;
	class MRenderGraph* m_pGraph;
};

class MORTY_API MRenderGraphNode
{
public:

    MRenderGraphNode();
    virtual ~MRenderGraphNode() {}

    MString GetNodeName() const { return m_strNodeName; }

    MRenderGraphNodeInput* AppendInput();
    MRenderGraphNodeOutput* AppendOutput();

    MRenderGraphNodeInput* GetInput(const size_t& nInputIdx);
    MRenderGraphNodeOutput* GetOutput(const size_t& nOutputIdx);

    MRenderGraphTexture* GetInputTexture(const size_t& nInputIdx);

    MRenderPass* GetRenderPass() { return m_pRenderPass; }

    void Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);

    void BindRenderFunction(const std::function<void(MRenderGraphNode*)>& func) { m_funcRender = func; }

protected:

    friend class MRenderGraph;

    MString m_strNodeName;

    std::vector<MRenderGraphNodeInput*> m_vInputTextures;
    std::vector<MRenderGraphNodeOutput*> m_vOutputTextures;

    int m_nCommandLevel;

    class MRenderGraph* m_pGraph;
    MRenderPass* m_pRenderPass;

    std::function<void(MRenderGraphNode*)> m_funcRender;
};

class MORTY_API MRenderGraph
{
public:
    MRenderGraph();
    MRenderGraph(MEngine* pEngine);
    virtual ~MRenderGraph();

public:
	MRenderGraphNode* AddRenderGraphNode(const MString& strNodeName);

    MRenderGraphTexture* AddRenderGraphTexture(const MString& strTextureName);

    MRenderGraphTexture* FindRenderGraphTexture(const MString& strTextureName) const;

    MRenderGraphNode* FindRenderGraphNode(const MString& strNodeName) const;

    MEngine* GetEngine() { return m_pEngine; }

    void SetFinalOutputTexture(MRenderGraphTexture* pGraphTexture);
    MRenderGraphTexture* GetFinalOutputTexture() const { return m_pFinalOutputTexture; }

    void SetFinalNode(MRenderGraphNode* pGraphNode);
    MRenderGraphNode* GetFinalNode() const { return m_pFinalNode; }

    bool GetCompiled() const { return m_bCompiled; }
	void CompileDirty();
	bool Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);

	void Render()
	{
		if (!m_bCompiled)
			return;

		for (MRenderGraphNode* pNode : m_vSortedNodes)
		{
			if (pNode->m_funcRender)
			{
                pNode->m_funcRender(pNode);
			}
		}
	}

protected:

    virtual MRenderGraphNode* NewRenderGraphNode() { return new MRenderGraphNode(); }

protected:
    std::map<MString, MRenderGraphNode*> m_tGraphNodeMap;
    std::map<MString, MRenderGraphTexture*> m_tGraphTextureMap;

    std::vector<MRenderGraphNode*> m_vSortedNodes;

    bool m_bCompiled;
    MEngine* m_pEngine;

    MRenderGraphNode* m_pFinalNode;
    MRenderGraphTexture* m_pFinalOutputTexture;
};


#endif
