/**
 * @File         MGuid
 * 
 * @Created      2022-06-30 11:00:00
 *
 * @Author       DoubleYe
**/

#pragma once
#include "crossguid/guid.hpp"

#include "Utility/MGlobal.h"

namespace morty
{

class MORTY_API MGuid
{
public:
    MGuid();

    static MGuid generate();


public:
    const MGuid& operator=(const MGuid& other);
    bool         operator==(const MGuid& other) const;
    bool         operator<(const MGuid& other) const;

public:
    static MGuid invalid;

    xg::Guid     m_id;
};

}// namespace morty