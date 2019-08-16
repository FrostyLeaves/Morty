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


	void CreateVertices(const unsigned int& unSize);
	void CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize);

private:
    
    friend class MModelResource;
    
    Vertex* m_vVertices;
    unsigned int* m_vIndices;

	MVertexBuffer* pVertexBuffer;
};


#endif
