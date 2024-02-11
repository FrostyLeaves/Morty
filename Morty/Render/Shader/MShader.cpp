﻿#include "MShader.h"
#include "Render/MIDevice.h"
#include "Render/MVertex.h"

using namespace morty;

bool MShader::CompileShader(MIDevice* pDevice)
{
	if (false == pDevice->CompileShader(this))
		return false;

	return true;
}

void MShader::CleanShader(MIDevice* pDevice)
{
	pDevice->CleanShader(this);
}
