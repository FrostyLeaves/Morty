#ifndef _M_VOXEL_STRUCT_DEFINE_HLSL_
#define _M_VOXEL_STRUCT_DEFINE_HLSL_


struct VoxelizerOutput
{
    uint nVoxelCount;
};


struct VoxelMapSetting
{
    float3 f3VoxelOrigin;       //voxel map origin position in world space.
    float fResolution;          //voxel table resolution.
    float fVoxelStep;           //how much distance does a voxel.
};


int VoxelUVWToInstanceId(VoxelMapSetting setting, uint3 uvw)
{
    int instanceId = int(uvw.z * (setting.fResolution * setting.fResolution) +
                    uvw.y * setting.fResolution +
                    uvw.x);
    
    return instanceId;
}

uint3 InstanceIdToVoxelUVW(VoxelMapSetting setting,int instanceId)
{
    int num = instanceId;

    uint3 uvw;
    uvw.z = num / (setting.fResolution * setting.fResolution);
    num = num - uvw.z * (setting.fResolution * setting.fResolution);

    uvw.y = num / setting.fResolution;
    num = num - uvw.y * setting.fResolution;

    uvw.x = num;
    
    return uvw;
}

uint3 WorldPositionToVoxelUVW(VoxelMapSetting setting, float3 position)
{
    uint3 uvw = uint3( (position - setting.f3VoxelOrigin) / fVoxelStep );

    return uvw;
}

float3 VoxelUVWToWorldPosition(VoxelMapSetting setting, uint3 uvw)
{
    float3 position = float3(uvw) * setting.fVoxelStep + setting.f3VoxelOrigin;

    return position;
}


#endif