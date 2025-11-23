#pragma once

#include <functional>
#include <map>
#include <queue>
#include <stdint.h>

#include "Tools/MModelImporter.h"
#include "Utility/MString.h"


#include "Main/BaseWidget.h"

namespace morty
{

class BaseWidget;
class ModelImportView : public BaseWidget
{
public:
    ModelImportView();

    ~ModelImportView() = default;

public:
    void Render() override;

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;

    void Convert(std::queue<MModelConvertInfo> queue);

private:
    std::queue<MModelConvertInfo> m_convertQueue;

    std::string                   m_strSourcePath;
    std::string                   m_strOutputDir;
    std::string                   m_strOutputName;
    size_t                        m_materialTypeEnum = 0;
    bool                          m_bImportCamera    = false;
    bool                          m_bImportLights    = true;

    // Status feedback
    std::string                   m_strStatusMessage;
    bool                          m_bIsConverting = false;
};

}// namespace morty