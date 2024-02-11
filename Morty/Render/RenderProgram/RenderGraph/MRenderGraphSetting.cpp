#include "MRenderGraphSetting.h"

#include "Utility/MFunction.h"

using namespace morty;

void MRenderGraphSetting::MarkDirty(const MStringId& name)
{
    m_tDirty.insert(name);
}

bool MRenderGraphSetting::IsDirty(const MStringId& name) const
{
    return m_tDirty.find(name) != m_tDirty.end();
}

void MRenderGraphSetting::FlushDirty()
{
    m_tDirty.clear();
}

void MRenderGraphSetting::RegisterPropertyVariant(const MStringId& name, const MVariant& value)
{
    m_tSettings[name] = value;
    MarkDirty(name);
}

MVariant MRenderGraphSetting::GetPropertyVariant(const MStringId& name) const
{
    auto findResult = m_tSettings.find(name);
    if (findResult == m_tSettings.end())
    {
        return MVariant();
    }

    return findResult->second;
}
