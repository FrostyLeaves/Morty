/**
 * @File         MString
 * 
 * @Created      2019-05-20 00:20:18
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MGlobal.h"

class MStringId
{
public:

    explicit MStringId() = default; 
    explicit MStringId(std::string_view strview);

    bool operator ==(const MStringId& other) const;
    bool operator <(const MStringId& other) const;

    size_t Hash() const { return m_hash; }

    const std::string& ToString() const { return *m_string; }

    size_t GetClashIndex() const;

private:

    //read only
    std::shared_ptr<std::string> m_string;
    size_t m_hash = 0;
    std::shared_ptr<std::array<size_t, MGlobal::M_MAX_THREAD_NUM>> m_clashIndex;
};

template <>
struct std::hash<MStringId>
{
    std::size_t operator()(const MStringId& strid) const
    {
        return strid.Hash();
    }
};
