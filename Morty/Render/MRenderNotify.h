/**
 * @File         MCoreNotify
 *
 * @Created      2021-08-03 14:54:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDER_NOTIFY_H_
#define _M_MRENDER_NOTIFY_H_
#include "Utility/MGlobal.h"

class MORTY_API MRenderNotify
{
public:

	static constexpr char* NOTIFY_MATERIAL_CHANGED = "Material Changed";
	static constexpr char* NOTIFY_MESH_CHANGED = "Mesh Changed";
	static constexpr char* NOTIFY_SKYBOX_TEX_CHANGED = "SkyBox Tex Changed";
	static constexpr char* NOTIFY_DIFFUSE_ENV_TEX_CHANGED = "Diffuse Environment Text Changed";
	static constexpr char* NOTIFY_SPECULAR_ENV_TEX_CHANGED = "Specular Environment Text Changed";
	static constexpr char* NOTIFY_GENERATE_SHADOW_CHANGED = "Generate Shadow Changed";
};

#endif
