/**
 * @File         MEditorModule
 * 
 * @Created      2021-08-20 11:38:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

class MEngine;
class MORTY_API MEditorModule
{
public:
    static bool Register(MEngine* pEngine);
};

}// namespace morty