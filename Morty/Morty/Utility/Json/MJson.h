/**
 * @File         MJson
 * 
 * @Created      2019-10-25 00:02:48
 *
 * @Author       Pobrecito
**/

#ifndef _M_MJSON_H_
#define _M_MJSON_H_
#include "MGlobal.h"
#include "MVariant.h"

class GenericValue;
class MORTY_CLASS MJson
{
public:
    MJson();
    virtual ~MJson();

	static MVariant JsonToMVariant(const MString& strJson);

public:

	

private:

};


#endif
