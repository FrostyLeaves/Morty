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


    size_t GetIndex() const { return m_unIndex; }
    MString GetStringID() const;

    MRenderGraphNode* GetRenderGraphNode() const { return pGraphNode; }
    MRenderGraphNode* GetLinkedNode() const;
    MRenderGraphTexture* GetLinkedTexture() const;
    MRenderGraphNodeOutput* GetLinkedOutput() const { return pLinkedOutput; }

private:
	friend class MRenderGraphNode;
	friend class MRenderGraphNodeOutput;

private:
    size_t m_unIndex;
	MRenderGraphNode* pGraphNode;
    MRenderGraphNodeOutput* pLinkedOutput;
};

class MORTY_API MRenderGraphNodeOutput
{
public:
    MRenderGraphNodeOutput();

	size_t GetIndex() const { return m_unIndex; }
	MString GetStringID() const;

    void SetClear(const bool& b) { bClear = b; }
    bool GetClear() const { return bClear; }

    void SetClearColor(const MColor& color) { mClearColor = color; }
    MColor GetClearColor() const { return mClearColor; }

    void SetRenderTexture(MRenderGraphTexture* pTexture);
    MRenderGraphTexture* GetRenderTexture() const { return pGraphTexture; }

    MRenderGraphNode* GetRenderGraphNode() const { return pGraphNode; }

    const std::vector<MRenderGraphNodeInput*>& GetLinkedInputs() const { return vLinkedInput; }

    void LinkTo(MRenderGraphNodeInput* pInput);
    void UnLink(MRenderGraphNodeInput* pInput);

private:

	friend class MRenderGraphNode;

    size_t m_unIndex;
    bool bClear;
    MColor mClearColor;

    MRenderGraphNode* pGraphNode;
    MRenderGraphTexture* pGraphTexture;

    std::vector<MRenderGraphNodeInput*> vLinkedInput;
};

class MORTY_API MRenderGraphTexture
{
public:
    enum class ESizePolicy
    {
        ERelative,
        EAbsolute
    };
public:
	MRenderGraphTexture();

    void SetSizePolicy(const ESizePolicy& ePolicy);
    ESizePolicy GetSizePolicy() const { return m_eSizePolicy; }

	MRenderGraph* GetRenderGraph() const { return m_pGraph; }
    MString GetTextureName() const { return m_strTextureName; }

	void SetUsage(const METextureUsage& eUsage);
	METextureUsage GetUsage() const { return m_eUsage; }

	void SetLayout(const METextureLayout& eLayout);
	METextureLayout GetLayout() const { return m_eLayout; }

	void SetSize(const Vector2& size);
	Vector2 GetSize() const { return m_v2Size; }

    Vector2 GetOutputSize() const;

public:

    void SetDirty();

    void AddRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);
    void RemoveRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput);

    MRenderGraphNodeOutput* GetFinalNodeOutput() const;

	MIRenderTexture* GetRenderTexture();

	void UpdateBuffer(MIDevice* pDevice);
    void DestroyBuffer(MIDevice* pDevice);

private:

	friend class MRenderGraph;

	MString m_strTextureName;
	METextureUsage m_eUsage;
	METextureLayout m_eLayout;
    ESizePolicy m_eSizePolicy;
	Vector2 m_v2Size;
    bool m_bDirty;

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
    int GetLevel() const { return m_nCommandLevel; }

    MRenderGraphNodeInput* AppendInput();
    MRenderGraphNodeOutput* AppendOutput();

    size_t GetInputSize() const { return m_vInputTextures.size(); }
    size_t GetOutputSize() const { return m_vOutputTextures.size(); }

    MRenderGraphNodeInput* GetInput(const size_t& nInputIdx);
    MRenderGraphNodeOutput* GetOutput(const size_t& nOutputIdx);

    MRenderGraphTexture* GetInputTexture(const size_t& nInputIdx);

    MRenderPass* GetRenderPass() { return m_pRenderPass; }

    void SetDirty() { m_bDirty = true; }
    bool GetDirty() { return m_bDirty; }

    void UpdateBuffer(MIDevice* pDevice);
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

    bool m_bDirty;

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

    virtual MRenderGraphNode* NewRenderGraphNode() { return new MRenderGraphNode(); }

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


#endif
