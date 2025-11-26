#include "Utility/MLogger.h"

using namespace morty;

MLogger* MLogger::GetInstance()
{
    static MLogger logger;
    return &logger;
}