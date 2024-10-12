#include "MVoxelMapUtil.h"
#include "Math/MMath.h"

using namespace morty;

MVoxelClipmap MVoxelMapUtil::GetClipMap(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx)
{
    const uint32_t nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;
    const float    fBasicVoxelSize = MRenderGlobal::VOXEL_BASIC_VOXEL_SIZE;

    const float    fVoxelSize      = fBasicVoxelSize * std::pow(2.0f, nClipmapIdx);
    Vector3        f3Origin        = f3CameraPosition - nVoxelTableSize * fVoxelSize * 0.5f;
    const Vector3i n3FloorPosition = MMath::Floor(f3Origin / fVoxelSize);
    f3Origin                       = Vector3(n3FloorPosition.x, n3FloorPosition.y, n3FloorPosition.z) * fVoxelSize;


    MVoxelClipmap result;
    result.f3VoxelOrigin = f3Origin;
    result.fVoxelSize    = fVoxelSize;

    return result;
}

MBoundsAABB MVoxelMapUtil::GetClipMapBounding(const MVoxelClipmap& clipmap)
{
    const uint32_t nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;

    return MBoundsAABB(clipmap.f3VoxelOrigin, clipmap.f3VoxelOrigin + nVoxelTableSize * clipmap.fVoxelSize);
}
