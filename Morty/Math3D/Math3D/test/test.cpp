// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"

int main(int argc, _TCHAR* argv[])
{
// 	Matrix4 mt1(1, 1, 1, 1,
// 				2, 2, 2, 2,
// 				3, 3, 3, 3,
// 				4, 4, 4, 4);
// 
// 	Matrix4 mt2(5, 6, 7, 8,
// 				5, 6, 7, 8,
// 				5, 6, 7, 8,
// 				5, 6, 7, 8);
// 
// 	Matrix4 mt3 = mt1 * mt2;
// 

	Quaternion a(Vector3(1, 1, 0), 45);

	auto b = a.GetAxis();


	return 0;
}

