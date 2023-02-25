#ifndef _MESSAGE_VIEW_H_
#define _MESSAGE_VIEW_H_

#include "Main/IBaseView.h"


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

	void SetDrawCallCount(size_t nCount) { m_nDrawCallCount = nCount; }

private:

	MEngine* m_pEngine;

	size_t m_nDrawCallCount = 0;
};

#endif