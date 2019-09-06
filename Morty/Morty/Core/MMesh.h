/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       Morty
**/

#ifndef _M_MMESH_H_
#define _M_MMESH_H_
#include "MGlobal.h"
#include "Vector.h"
#include <vector>

class MIRenderer;
class MVertexBuffer;
class MVertex;
class MMaterial;

class MORTY_CLASS MMesh
{
public:
    MMesh(const bool& bModifiable = false);
    virtual ~MMesh();

public:
	MVertexBuffer* GetBuffer() { return m_pVertexBuffer; }
	unsigned int GetVerticesLength() { return m_unVerticesLength; }
	unsigned int GetIndicesLength() { return m_unIndicesLength; }
	MVertex* GetVertices(){ return m_vVertices; }
	unsigned int* GetIndices(){ return m_vIndices; }

	bool GetNeedGenerate() { return nullptr == m_pVertexBuffer || m_bNeedGenerate; }
	bool GetNeedUpload(){ return m_bNeedUpload; }
	void SetNeedUpload(){ m_bNeedUpload = true; }

	void GenerateBuffer(MIRenderer* pRenderer);
	void UploadBuffer(MIRenderer* pRenderer);

public:
    
	void CreateVertices(const unsigned int& unSize);
	void CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize);

private:

    MVertex* m_vVertices;
    unsigned int* m_vIndices;
	unsigned int m_unVerticesLength;
	unsigned int m_unIndicesLength;

	unsigned int m_unVerticesArraySize;
	unsigned int m_unIndicesArraySize;

	MVertexBuffer* m_pVertexBuffer;

	bool m_bNeedGenerate;
	bool m_bNeedUpload;
	bool m_bModifiable;
};

#endif
