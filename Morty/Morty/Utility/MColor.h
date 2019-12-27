/**
 * @File         MString
 * 
 * @Created      2019-11-04 23:01:32
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOLOR_H_
#define _M_MCOLOR_H_

class MColor
{
public:
	MColor();
	~MColor();



public:

	union
	{
		struct 
		{
			char r;
			char g;
			char b;
			char a;
		};

		char m[4];
	};

};


#endif
