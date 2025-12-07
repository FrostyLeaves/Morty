#include "Utility/MRect.h"
#include <algorithm>

using namespace morty;

// MRect implementation
const MRect MRect::Zero = MRect(0.0f, 0.0f, 0.0f, 0.0f);
const MRect MRect::Unit = MRect(0.0f, 0.0f, 1.0f, 1.0f);

MRect::MRect()
    : x(0.0f)
    , y(0.0f)
    , width(0.0f)
    , height(0.0f)
{}

MRect::MRect(const float& _x, const float& _y, const float& _width, const float& _height)
    : x(_x)
    , y(_y)
    , width(_width)
    , height(_height)
{}

MRect::MRect(const Vector2& position, const Vector2& size)
    : x(position.x)
    , y(position.y)
    , width(size.x)
    , height(size.y)
{}

void MRect::SetPosition(const Vector2& position)
{
    x = position.x;
    y = position.y;
}

void MRect::SetPosition(const float& _x, const float& _y)
{
    x = _x;
    y = _y;
}

void MRect::SetSize(const Vector2& size)
{
    width  = size.x;
    height = size.y;
}

void MRect::SetSize(const float& _width, const float& _height)
{
    width  = _width;
    height = _height;
}

void MRect::SetLeft(const float& left)
{
    width += (x - left);
    x = left;
}

void MRect::SetTop(const float& top)
{
    height += (y - top);
    y = top;
}

void MRect::SetRight(const float& right) { width = right - x; }

void MRect::SetBottom(const float& bottom) { height = bottom - y; }

bool MRect::Contains(const Vector2& point) const { return Contains(point.x, point.y); }

bool MRect::Contains(const float& px, const float& py) const
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

bool MRect::Intersects(const MRect& other) const
{
    return !(
            other.GetLeft() > GetRight() || other.GetRight() < GetLeft() || other.GetTop() > GetBottom() ||
            other.GetBottom() < GetTop()
    );
}

MRect MRect::Intersection(const MRect& other) const
{
    if (!Intersects(other)) { return MRect::Zero; }

    float left   = std::max(GetLeft(), other.GetLeft());
    float top    = std::max(GetTop(), other.GetTop());
    float right  = std::min(GetRight(), other.GetRight());
    float bottom = std::min(GetBottom(), other.GetBottom());

    return MRect(left, top, right - left, bottom - top);
}

MRect MRect::Union(const MRect& other) const
{
    float left   = std::min(GetLeft(), other.GetLeft());
    float top    = std::min(GetTop(), other.GetTop());
    float right  = std::max(GetRight(), other.GetRight());
    float bottom = std::max(GetBottom(), other.GetBottom());

    return MRect(left, top, right - left, bottom - top);
}

bool MRect::operator==(const MRect& other) const
{
    return x == other.x && y == other.y && width == other.width && height == other.height;
}

bool         MRect::operator!=(const MRect& other) const { return !(*this == other); }

// MRecti implementation
const MRecti MRecti::Zero = MRecti(0, 0, 0, 0);
const MRecti MRecti::Unit = MRecti(0, 0, 1, 1);

MRecti::MRecti()
    : x(0)
    , y(0)
    , width(0)
    , height(0)
{}

MRecti::MRecti(const int& _x, const int& _y, const int& _width, const int& _height)
    : x(_x)
    , y(_y)
    , width(_width)
    , height(_height)
{}

MRecti::MRecti(const Vector2i& position, const Vector2i& size)
    : x(position.x)
    , y(position.y)
    , width(size.x)
    , height(size.y)
{}

void MRecti::SetPosition(const Vector2i& position)
{
    x = position.x;
    y = position.y;
}

void MRecti::SetPosition(const int& _x, const int& _y)
{
    x = _x;
    y = _y;
}

void MRecti::SetSize(const Vector2i& size)
{
    width  = size.x;
    height = size.y;
}

void MRecti::SetSize(const int& _width, const int& _height)
{
    width  = _width;
    height = _height;
}

void MRecti::SetLeft(const int& left)
{
    width += (x - left);
    x = left;
}

void MRecti::SetTop(const int& top)
{
    height += (y - top);
    y = top;
}

void MRecti::SetRight(const int& right) { width = right - x; }

void MRecti::SetBottom(const int& bottom) { height = bottom - y; }

bool MRecti::Contains(const Vector2i& point) const { return Contains(point.x, point.y); }

bool MRecti::Contains(const int& px, const int& py) const
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

bool MRecti::Intersects(const MRecti& other) const
{
    return !(
            other.GetLeft() > GetRight() || other.GetRight() < GetLeft() || other.GetTop() > GetBottom() ||
            other.GetBottom() < GetTop()
    );
}

MRecti MRecti::Intersection(const MRecti& other) const
{
    if (!Intersects(other)) { return MRecti::Zero; }

    int left   = std::max(GetLeft(), other.GetLeft());
    int top    = std::max(GetTop(), other.GetTop());
    int right  = std::min(GetRight(), other.GetRight());
    int bottom = std::min(GetBottom(), other.GetBottom());

    return MRecti(left, top, right - left, bottom - top);
}

MRecti MRecti::Union(const MRecti& other) const
{
    int left   = std::min(GetLeft(), other.GetLeft());
    int top    = std::min(GetTop(), other.GetTop());
    int right  = std::max(GetRight(), other.GetRight());
    int bottom = std::max(GetBottom(), other.GetBottom());

    return MRecti(left, top, right - left, bottom - top);
}

bool MRecti::operator==(const MRecti& other) const
{
    return x == other.x && y == other.y && width == other.width && height == other.height;
}

bool MRecti::operator!=(const MRecti& other) const { return !(*this == other); }
