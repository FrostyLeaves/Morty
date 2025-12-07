/**
 * @File         MRect
 * 
 * @Created      2025-12-05
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"

namespace morty
{

class MORTY_API MRect
{
public:
    MRect();

    MRect(const float& x, const float& y, const float& width, const float& height);

    MRect(const Vector2& position, const Vector2& size);

    MRect(const MRect& other) = default;

    ~MRect() = default;

    float   GetLeft() const { return x; }

    float   GetTop() const { return y; }

    float   GetRight() const { return x + width; }

    float   GetBottom() const { return y + height; }

    float   GetWidth() const { return width; }

    float   GetHeight() const { return height; }

    Vector2 GetPosition() const { return Vector2(x, y); }

    Vector2 GetSize() const { return Vector2(width, height); }

    Vector2 GetCenter() const { return Vector2(x + width * 0.5f, y + height * 0.5f); }

    void    SetPosition(const Vector2& position);

    void    SetPosition(const float& x, const float& y);

    void    SetSize(const Vector2& size);

    void    SetSize(const float& width, const float& height);

    void    SetLeft(const float& left);

    void    SetTop(const float& top);

    void    SetRight(const float& right);

    void    SetBottom(const float& bottom);

    bool    Contains(const Vector2& point) const;

    bool    Contains(const float& px, const float& py) const;

    bool    Intersects(const MRect& other) const;

    MRect   Intersection(const MRect& other) const;

    MRect   Union(const MRect& other) const;

    bool    operator==(const MRect& other) const;

    bool    operator!=(const MRect& other) const;

    MRect&  operator=(const MRect& other) = default;

public:
    union
    {
        struct {
            float x;
            float y;
            float width;
            float height;
        };
        float m[4];
    };

    static const MRect Zero;
    static const MRect Unit;
};

class MORTY_API MRecti
{
public:
    MRecti();

    MRecti(const int& x, const int& y, const int& width, const int& height);

    MRecti(const Vector2i& position, const Vector2i& size);

    MRecti(const MRecti& other) = default;

    ~MRecti() = default;

    int      GetLeft() const { return x; }

    int      GetTop() const { return y; }

    int      GetRight() const { return x + width; }

    int      GetBottom() const { return y + height; }

    int      GetWidth() const { return width; }

    int      GetHeight() const { return height; }

    Vector2i GetPosition() const { return Vector2i(x, y); }

    Vector2i GetSize() const { return Vector2i(width, height); }

    Vector2  GetCenter() const { return Vector2(x + width * 0.5f, y + height * 0.5f); }

    void     SetPosition(const Vector2i& position);

    void     SetPosition(const int& x, const int& y);

    void     SetSize(const Vector2i& size);

    void     SetSize(const int& width, const int& height);

    void     SetLeft(const int& left);

    void     SetTop(const int& top);

    void     SetRight(const int& right);

    void     SetBottom(const int& bottom);

    bool     Contains(const Vector2i& point) const;

    bool     Contains(const int& px, const int& py) const;

    bool     Intersects(const MRecti& other) const;

    MRecti   Intersection(const MRecti& other) const;

    MRecti   Union(const MRecti& other) const;

    bool     operator==(const MRecti& other) const;

    bool     operator!=(const MRecti& other) const;

    MRecti&  operator=(const MRecti& other) = default;

public:
    union
    {
        struct {
            int x;
            int y;
            int width;
            int height;
        };
        int m[4];
    };

    static const MRecti Zero;
    static const MRecti Unit;
};

}// namespace morty
