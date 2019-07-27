#ifndef _MATRIX_H_
#define _MATRIX_H_

#ifndef MATH3D_EXPORTS
#define MATH_IOE_DLL _declspec(dllimport)
#else
#define MATH_IOE_DLL _declspec(dllexport)
#endif


#include "Vector.h"

class Matrix4;
class MATH_IOE_DLL Matrix3
{
public:
	Matrix3(const Matrix4& mat4, const int&i, const int&j);

	float Determinant() const;

public:
	float m[3][3];
};

class MATH_IOE_DLL Matrix4
{
public:
	Matrix4();
	Matrix4(float value[4][4]);
	Matrix4(const float& m_00, const float& m_01, const float& m_02, const float& m_03,
		const float& m_10, const float& m_11, const float& m_12, const float& m_13,
		const float& m_20, const float& m_21, const float& m_22, const float& m_23,
		const float& m_30, const float& m_31, const float& m_32, const float& m_33);

	Matrix4 Transposed() const;

	float Determinant() const;

	Matrix4 CofactorMatrix() const;
	Matrix4 AdjointMatrix() const;
	Matrix4 Inverse() const;

	bool IsOrthogonal() const;

	Matrix4 operator* (const Matrix4& mat) const;
	Matrix4 operator * (const float& value) const;
	Matrix4 operator / (const float& value) const;

	bool operator == (const Matrix4& mat) const;

	void Translation(const float& x, const float& y, const float& z);

public:
	float m[4][4];


private:
	
	float AlgebraicCofactor(const int& i, const int& j) const;
};


const Matrix4 IdentityMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);



#endif