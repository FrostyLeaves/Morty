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
class MORTY_CLASS MMesh
{
public:
    MMesh();
    virtual ~MMesh();

public:
    
    struct Vertex{
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector3 tangent;
        Vector3 bitangent;
    };


private:
    
    friend class MModelResource;

	void CreateVertices(const unsigned int& unSize);
	void CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize);

	MVertexBuffer* GetBuffer() { return m_pVertexBuffer; }
	void GenerateBuffer(MIRenderer* pRenderer);

    Vertex* m_vVertices;
    unsigned int* m_vIndices;

	MVertexBuffer* m_pVertexBuffer;
};


#endif
