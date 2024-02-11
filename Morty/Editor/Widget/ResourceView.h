#pragma once

#include "Main/BaseWidget.h"

MORTY_SPACE_BEGIN

class ResourceView : public BaseWidget
{
public:
	ResourceView();
    ~ResourceView() = default;

	void Render() override;
	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;
	
};

MORTY_SPACE_END