/**
 * @File         MLogManager
 * 
 * @Created      2019-05-13 00:38:35
 *
 * @Author       Morty
**/

#ifndef _M_MLOGMANAGER_H_
#define _M_MLOGMANAGER_H_
#include "MGlobal.h"
#include "MSingleInstance.h"

class MORTY_CLASS MLogManager : public MSingleInstance<MLogManager>
{
public:
	MLogManager(){};
	virtual ~MLogManager() {};

public:

	void Error(const char* svMessage, ...);
	void Information(const char* svMessage, ...);
	void Log(const char* svMessage, ...);

private:

};


#endif
