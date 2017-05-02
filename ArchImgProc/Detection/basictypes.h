#pragma once

namespace ArchImgProc
{
	template<typename tFlt>
	struct Point
	{
		tFlt	x, y;

		tFlt DistanceTo(const Point<tFlt>& p) const
		{
			return std::sqrt((p.x - this->x)*(p.x - this->x) + (p.y - this->y)*(p.y - this->y));
		}
	};

	template<typename tFlt>
	struct Vector2
	{
		tFlt	x, y;

		Vector2<tFlt> GetNormalized() const
		{
			tFlt f = std::sqrt(this->x*this->x + this->y*this->y);
			return Vector2<tFlt>{this->x / f, this->y / f};
		}

		tFlt DotProduct(const Vector2<tFlt>& other) const
		{
			return other.x*this->x + other.y*this->y;
		}
	};

	struct IntRect
	{
		int x, y, w, h;
	};
}