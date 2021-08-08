#pragma once

#include <stdint.h>

#define ImDrawIdx uint32_t

#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT \
struct ImDrawVert{ImVec2 pos;ImVec2 uv;ImColor col;};
