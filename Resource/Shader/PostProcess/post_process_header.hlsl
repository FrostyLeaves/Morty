#ifndef _POST_PROCESS_HEADER_H_
#define _POST_PROCESS_HEADER_H_


[[vk::binding(0,1)]]cbuffer cbPostProcess
{
    float2 u_f2ScreenSize;
};

[[vk::binding(1,1)]]sampler LinearSampler;


[[vk::binding(0,0)]]Texture2D u_texInputTexture;


#endif