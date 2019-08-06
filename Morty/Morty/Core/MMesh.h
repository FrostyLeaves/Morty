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
    
    friend class MModelLoader;
    
    std::vector<Vertex> m_vVertices;
    std::vector<unsigned int> m_vIndices;

};


#endif
