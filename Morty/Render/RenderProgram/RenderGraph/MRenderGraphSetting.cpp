#include "MRenderGraphSetting.h"

#include "Utility/MFunction.h"

using namespace morty;

void MRenderGraphSetting::MarkDirty(const MStringId& name) { m_dirty.insert(name); }

bool MRenderGraphSetting::IsDirty(const MStringId& name) const { return m_dirty.find(name) != m_dirty.end(); }

void MRenderGraphSetting::FlushDirty() { m_dirty.clear(); }

void MRenderGraphSetting::RegisterPropertyVariant(const MStringId& name, const MVariant& value)
{
    m_settings[name] = value;
    MarkDirty(name);
}

MVariant MRenderGraphSetting::GetPropertyVariant(const MStringId& name) const
{
    auto findResult = m_settings.find(name);
    if (findResult == m_settings.end()) { return MVariant(); }

    return findResult->second;
}
