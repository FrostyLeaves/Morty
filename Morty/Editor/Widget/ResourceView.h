#ifndef _RESOURCE_VIEW_H_
#define _RESOURCE_VIEW_H_


#include "Main/IBaseView.h"


class ResourceView : public IBaseView
{
public:
	ResourceView();
	virtual ~ResourceView();

	virtual void Render() override;
		;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;




private:
	MEngine* m_pEngine;
};





#endif