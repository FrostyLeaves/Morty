#pragma once

#include "Main/BaseWidget.h"

namespace morty
{

class MessageWidget : public BaseWidget
{
public:
    MessageWidget();

    ~MessageWidget() = default;

public:
    void Render() override;

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;
};

}// namespace morty