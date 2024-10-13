#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Material/MMaterial.h"
#include "Render/MRenderInfo.h"
#include "Variant/MVariant.h"

namespace morty
{

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;


class MORTY_API MVoxelMapUtil
{
public:
    static MVoxelClipmap GetClipMap(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx);

    static MBoundsAABB   GetClipMapBounding(const MVoxelClipmap& clipmap);
};

}// namespace morty