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
	Vector2 GetSize() const { m_v2Size; }

    void AddRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);
    void RemoveRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);

	MIRenderTexture* GetRenderTexture(const size_t& nIdx);

	void Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);



private:

	friend class MRenderGraph;

	MString m_strTextureName;
	METextureUsage m_eUsage;
	METextureLayout m_eLayout;
	Vector2 m_v2Size;

	std::array<MRenderTexture*, M_BUFFER_NUM> m_aTextures;
    std::vector<MRenderGraphNodeOutput*> m_vOutputs;
	class MRenderGraph* m_pGraph;
};

class MORTY_API MRenderGraphNode
{
public:

    MRenderGraphNode();
    virtual ~MRenderGraphNode() {}

    MRenderGraphNodeInput* AppendInput();
    MRenderGraphNodeOutput* AppendOutput();

    MRenderGraphNodeInput* GetInput(const size_t& nInputIdx);
    MRenderGraphNodeOutput* GetOutput(const size_t& nOutputIdx);

    MRenderGraphTexture* GetInputTexture(const size_t& nInputIdx);

    MRenderPass* GetRenderPass() { return m_pRenderPass; }

    void SetFinalNode(const bool& bFinal) { m_bFinalNode = bFinal; }
    bool GetFinalNode()const { return m_bFinalNode; }

    void Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);

protected:

    friend class MRenderGraph;

    MString m_strNodeName;

    std::vector<MRenderGraphNodeInput*> m_vInputTextures;
    std::vector<MRenderGraphNodeOutput*> m_vOutputTextures;

    int m_nCommandLevel;
    bool m_bFinalNode;

    class MRenderGraph* m_pGraph;
    MRenderPass* m_pRenderPass;
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

    bool GetCompiled() const { return m_bCompiled; }
	void CompileDirty();
	bool Compile(MIDevice* pDevice);

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);

protected:

    virtual MRenderGraphNode* NewRenderGraphNode() { return new MRenderGraphNode(); }

protected:
    std::map<MString, MRenderGraphNode*> m_tGraphNodeMap;
    std::map<MString, MRenderGraphTexture*> m_tGraphTextureMap;

    std::vector<MRenderGraphNode*> m_vSortedNodes;

    bool m_bCompiled;
    MEngine* m_pEngine;
};

template<typename T>
class MRenderGraphNodeTemplate : public MRenderGraphNode
{
public:
	void BindRenderFunction(std::function<void(MRenderGraphNode*, T&)> func) { m_funcRender = func; }

	std::function<void(MRenderGraphNode*, T&)> m_funcRender; // render 
};

template<typename T>
class MORTY_API MRenderGraphTemplate : public MRenderGraph
{
public:

    MRenderGraphTemplate() :MRenderGraph() {}
    MRenderGraphTemplate(MEngine* pEngine) : MRenderGraph(pEngine) {}

	virtual MRenderGraphNode* NewRenderGraphNode() override { return new MRenderGraphNodeTemplate<T>(); }

public:

    

	void Render(T& renderInfo)
	{
        if (!m_bCompiled)
            return;

		for (MRenderGraphNode* pNode : m_vSortedNodes)
		{
            MRenderGraphNodeTemplate<T>*  pTypedNode = static_cast<MRenderGraphNodeTemplate<T>*>(pNode);
			if (pTypedNode->m_funcRender)
			{
                pTypedNode->m_funcRender(pTypedNode, renderInfo);
			}
		}
	}
};


#endif
