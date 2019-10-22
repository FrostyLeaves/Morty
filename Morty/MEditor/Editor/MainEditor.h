#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MWindowsRenderView.h"

class MNode;
class MIScene;
class MMeshInstance;
class MainEditor : public MWindowsRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();


	void SetEditorNode(MNode* pNode);


	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	virtual void OnResize(const int& nWidth, const int& nHeight) override;

	virtual void OnRenderEnd() override;

public:
	virtual LRESULT CALLBACK ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

protected:

	MIScene* m_pScene;

};


#endif