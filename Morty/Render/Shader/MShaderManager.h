#pragma once

#include "Object/MObject.h"
#include "Render/MMesh.h"
#include "Render/MBuffer.h"
#include "Utility/MGlobal.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

MORTY_SPACE_BEGIN

class MShaderProgram;
class MTaskNode;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MMeshBufferAdapter;
class MRenderMeshComponent;

class MShaderProgramKey
{
};

class MShaderManager : public MObject
{
	MORTY_CLASS(MShaderManager)
public:
	explicit  MShaderManager();

	void OnCreated() override;
	void OnDelete() override;


	std::shared_ptr<MShaderProgram> FindOrCreateShaderProgram(MShaderProgramKey key);

public:


	std::map<MShaderProgramKey, std::shared_ptr<MShaderProgram>> m_tShaderProgramTable;
};

MORTY_SPACE_END