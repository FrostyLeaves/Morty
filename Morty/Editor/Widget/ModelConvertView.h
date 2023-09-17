#pragma once

#include <map>
#include <queue>
#include <functional>
#include <stdint.h>

#include "Utility/MString.h"
#include "Model/MModelConverter.h"


#include "Main/BaseWidget.h"

class BaseWidget;
class ModelConvertView : public BaseWidget
{
public:
	ModelConvertView();
    ~ModelConvertView() = default;

public:
	
	void Render() override;

	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;
	
	void Convert(std::queue<MModelConvertInfo> queue);

private:

	std::queue<MModelConvertInfo> m_convertQueue;

	std::string m_strSourcePath;
	std::string m_strOutputDir;
	std::string m_strOutputName;
	uint32_t m_nMaterialTypeEnum = 0;
};

