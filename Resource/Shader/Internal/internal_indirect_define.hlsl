#ifndef _M_INTERNAL_INDIRECT_DEFINE_HLSL_
#define _M_INTERNAL_INDIRECT_DEFINE_HLSL_


// Same layout as VkDrawIndexedIndirectCommand
struct IndexedIndirectCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};



#endif