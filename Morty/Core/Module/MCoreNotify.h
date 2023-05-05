/**
 * @File         MCoreNotify
 *
 * @Created      2021-08-03 14:54:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCORE_NOTIFY_H_
#define _M_MCORE_NOTIFY_H_
#include "Utility/MGlobal.h"

class MORTY_API MCoreNotify
{
public:
    static constexpr char* NOTIFY_VISIBLE_CHANGED = "Visible Changed";
    static constexpr char* NOTIFY_TRANSFORM_CHANGED = "Transform Changed";
};

#endif
