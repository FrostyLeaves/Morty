/**
 * @File         MCoreNotify
 *
 * @Created      2021-08-03 14:54:21
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

class MORTY_API MCoreNotify
{
public:
    static inline const char* NOTIFY_VISIBLE_CHANGED = "Visible Changed";
    static inline const char* NOTIFY_TRANSFORM_CHANGED = "Transform Changed";
    static inline const char* NOTIFY_PARENT_CHANGED = "Parent Changed";
};

MORTY_SPACE_END