#pragma once
#include "Main/BaseWidget.h"

MORTY_SPACE_BEGIN

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

MORTY_SPACE_END