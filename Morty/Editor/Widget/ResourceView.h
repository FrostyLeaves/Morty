#pragma once

#include "Main/BaseWidget.h"


class ResourceView : public BaseWidget
{
public:
	ResourceView();
    ~ResourceView() = default;

	void Render() override;
	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;
	
};

