/**
 * @File         MCoreNotify
 *
 * @Created      2021-08-03 14:54:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

class MORTY_API MRenderNotify
{
public:

	static inline const char* NOTIFY_MATERIAL_CHANGED = "Material Changed";
	static inline const char* NOTIFY_MESH_CHANGED = "Mesh Changed";
	static inline const char* NOTIFY_SKYBOX_TEX_CHANGED = "SkyBox Tex Changed";
	static inline const char* NOTIFY_DIFFUSE_ENV_TEX_CHANGED = "Diffuse Environment Text Changed";
	static inline const char* NOTIFY_SPECULAR_ENV_TEX_CHANGED = "Specular Environment Text Changed";
	static inline const char* NOTIFY_GENERATE_SHADOW_CHANGED = "Generate Shadow Changed";
	static inline const char* NOTIFY_ANIMATION_POSE_CHANGED = "Animation Pose Changed";
	static inline const char* NOTIFY_ATTACHED_SKELETON_CHANGED = "Attached Skeleton Changed";
};
