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
    MMesh();
    virtual ~MMesh();

public:
	MVertexBuffer* GetBuffer() { return m_pVertexBuffer; }
	unsigned int GetVerticesLength() { return m_unVerticesLength; }
	unsigned int GetIndicesLength() { return m_unIndicesLength; }
	MVertex* GetVertices(){ return m_vVertices; }
	unsigned int* GetIndices(){ return m_vIndices; }

	void GenerateBuffer(MIRenderer* pRenderer);

public:

// 	void SetMaterial(MMaterial* pMaterial);
// 	MMaterial* GetMaterial(){ return m_pMaterial; }

private:
    
    friend class MModelResource;

	void CreateVertices(const unsigned int& unSize);
	void CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize);

    MVertex* m_vVertices;
    unsigned int* m_vIndices;
	unsigned int m_unVerticesLength;
	unsigned int m_unIndicesLength;

	MVertexBuffer* m_pVertexBuffer;

	MMaterial* m_pMaterial;

};


#endif
