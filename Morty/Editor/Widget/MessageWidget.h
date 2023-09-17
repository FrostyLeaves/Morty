#pragma once
#include "Main/BaseWidget.h"


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