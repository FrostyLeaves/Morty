/**
 * @File         MMeshUtil
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"

namespace morty
{

class MIDevice;
class MBuffer;
struct MVertex;

class MMaterial;
class MIMesh;
enum class MEMeshVertexType
{
    Normal = 0,
    Skeleton,
};

class MMeshUtil
{
public:
    static std::unique_ptr<MIMesh> CreateMeshFromType(MEMeshVertexType eType);

    static std::unique_ptr<MIMesh> CreatePlane(MEMeshVertexType eType);

    static std::unique_ptr<MIMesh> CreateCube(MEMeshVertexType eType);

    static std::unique_ptr<MIMesh> CreateSphere(MEMeshVertexType eType);
};

}// namespace morty