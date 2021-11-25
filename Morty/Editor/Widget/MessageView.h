#ifndef _MESSAGE_VIEW_H_
#define _MESSAGE_VIEW_H_

#include "IBaseView.h"


class MessageView : public IBaseView
{
public:
	MessageView();
	virtual ~MessageView();

public:
	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;
	virtual void Input(MInputEvent* pEvent) override;

private:

	MEngine* m_pEngine;
};

#endif