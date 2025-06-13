#pragma once

struct Vector2i
{
	int x;
	int y;

	Vector2i() : x(0), y(0) {}
	Vector2i(int _x, int _y) : x(_x), y(_y) {}

	bool operator==(const Vector2i& v2) const
	{
		return x == v2.x && y == v2.y;
	}

	bool operator!=(const Vector2i& v2) const
	{
		return !(x == v2.x && y == v2.y);
	}

	raylib::Vector2 Vector2iToVector2() const
	{
		return raylib::Vector2(static_cast<float>(x), static_cast<float>(y));
	}

	int DistanceTo(const Vector2i& v2) const
	{
		int dx = x - v2.x;
		int dy = y - v2.y;
		return static_cast<int>(sqrt(dx * dx + dy * dy));
	}
};