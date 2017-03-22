#pragma once

namespace ArchImgProc
{
	template<typename tFlt>
	struct Point
	{
		tFlt	x, y;

		float DistanceTo(const Point<tFlt>& p) const
		{
			return sqrt((p.x - this->x)*(p.x - this->x) + (p.y - this->y)*(p.y - this->y));
		}
	};

	template<typename tFlt>
	struct Vector2
	{
		tFlt	x, y;
	};

	struct IntRect
	{
		int x, y, w, h;
	};

}