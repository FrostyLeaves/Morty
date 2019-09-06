#ifndef _MAIN_EDITOR_H_
#define _MAIN_EDITOR_H_

#include "MWindowsRenderView.h"


class MMeshInstance;
class MainEditor : public MWindowsRenderView
{
public:

	MainEditor();
	virtual ~MainEditor();


	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;

	virtual void OnRenderEnd() override;




protected:


	void DrawGUI(class ImDrawData* pDrawData);


	void InitImGUI();
	bool InitImGUIDX11();



	MMeshInstance* m_pMeshInstance;
};


#endif