/**
 * @File         MJson
 * 
 * @Created      2019-10-25 00:02:48
 *
 * @Author       DoubleYe
**/

#ifndef _M_MJSON_H_
#define _M_MJSON_H_
#include "Utility/MGlobal.h"
#include "Utility/MVariant.h"

class GenericValue;
class MORTY_API MJson
{
public:
    MJson();
    virtual ~MJson();

	static bool JsonToMVariant(const MString& strJson, MVariant& variant);
	static void MVariantToJson(const MVariant& var, MString& strJson);

public:

	

private:

};


#endif
