/**
 * @File         MRenderGraph
 * 
 * @Created      2021-01-27 16:21:44
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERGRAPH_NODE_H_
#define _M_MRENDERGRAPH_NODE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MTexture.h"
#include "MTypedClass.h"

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

class MORTY_API MRenderGraphNode : public MTypedClass
{
public:
    MTypedClassSign(MRenderGraphNode)
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
    void ResetDirty() { m_bDirty = false; }
    bool GetDirty() { return m_bDirty; }

    virtual void UpdateBuffer(MIDevice* pDevice);
    virtual void DestroyBuffer(MIDevice* pDevice);

    void BindUpdateFunction(const std::function<void(MRenderGraphNode*)>& func) { m_funcUpdate = func; };

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

	std::function<void(MRenderGraphNode*)> m_funcUpdate;
    std::function<void(MRenderGraphNode*)> m_funcRender;
};


#endif
