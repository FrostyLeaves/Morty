#include "MStorageVariant.h"

using namespace morty;

void MStorageVariant::ResizeMemory(MIDevice* pDevice, size_t nSize)
{
    if (buffer.GetSize() == nSize) { return; }

    data.resize(nSize);

    buffer.DestroyBuffer(pDevice);
    buffer.ReallocMemory(nSize);
    buffer.GenerateBuffer(pDevice, data.data(), nSize);
}

void MStorageVariant::UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* pData, const size_t& nSize)
{
    memcpy(data.data() + nBeginOffset, pData, nSize);
    buffer.UploadBuffer(pDevice, nBeginOffset, pData, nSize);
}
