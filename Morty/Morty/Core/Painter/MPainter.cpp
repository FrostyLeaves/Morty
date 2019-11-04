#include "MPainter.h"

struct Line
{
	MColor color;
	float lineWidth;
	Vector3 begin;
	Vector3 end;
};

struct Rect
{
	MColor color;
	float lineWidth;
	Vector3 center;
	Vector3 normal;
	float width;
	float height;
};

struct Circle
{
	MColor color;
	float lineWidth;
	Vector3 center;
	Vector3 normal;
	float radius;
	float angle;
};

enum MEShape
{
	ELINE = 1,
	ERECT = 2,
	ECIRCLE = 3,
};

struct Shape
{
	MEShape eShape;
	union
	{
		Line line;
		Rect rect;
		Circle circle;
	};
};

MPainter::MPainter()
{

}

MPainter::~MPainter()
{

}
