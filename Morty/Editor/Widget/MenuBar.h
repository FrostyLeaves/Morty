#pragma once

#include "Main/BaseWidget.h"
#include <map>
#include <string>

namespace morty
{

class MScene;
class MResource;

class MenuBar : public BaseWidget
{
public:
    MenuBar();
    virtual ~MenuBar() = default;

    void Render() override;

    void Initialize(MainEditor* pMainEditor) override;
    void Release() override;

protected:
    void RenderFileMenu();
    void RenderViewMenu();
    void RenderWindowMenu();
    void RenderEditMenu();
    void RenderToolMenu();
    void RenderFileDialogs();

    void HandleOpenFile(const std::map<std::string, std::string>& files);
    void HandleSaveFile(const std::string& filePath, const std::string& fileName);

    bool LoadEntityFile(const std::string& filePath);
};

}// namespace morty
