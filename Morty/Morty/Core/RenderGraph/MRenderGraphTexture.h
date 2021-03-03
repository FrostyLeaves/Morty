/**
 * @File         MRenderGraph
 * 
 * @Created      2021-01-27 16:21:44
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERGRAPH_TEXTURE_H_
#define _M_MRENDERGRAPH_TEXTURE_H_
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


#endif
