#pragma once

namespace ArchImgProc
{
	class CUtils
	{
	public:
		template<typename tFlt>
		static tFlt DegToRad(tFlt x)
		{
			return (x / 180)*((tFlt)3.14159265358979323846);
		}

		template<typename tFlt>
		static tFlt RadToDeg(tFlt x)
		{
			return (x / ((tFlt)3.14159265358979323846)) * 180;
		}

		template<typename tFlt>
		static tFlt CalcDistance(tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			return std::sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
		}

		template<typename tFlt>
		static tFlt CalcDistanceSquared(tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			return ((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
		}

		template<typename tFlt>
		static void ConvertToHessianNormalForm(tFlt x1, tFlt y1, tFlt x2, tFlt y2, tFlt* angle, tFlt* distance)
		{
			tFlt nx = y1 - y2;
			tFlt ny = -(x1 - x2);

			// let p_vec = (lsd.x1,lsd.y1)
			tFlt p_times_n = x1*nx + y1*ny;
			int factor = p_times_n >= 0 ? 1 : -1;
			tFlt abs_n = sqrt(nx*nx + ny*ny);
			tFlt n0x = factor*nx / abs_n;
			tFlt n0y = factor*ny / abs_n;

			tFlt _dist = x1*n0x + y1*n0y;

			// _angle must be >= -90 degree and <= 180 degree (in our case, where all coordinates are positive)
			//assert(_angle >= -M_PI / 2 && _angle <= M_PI);

			if (distance != nullptr)
			{
				*distance = _dist;
			}

			if (angle != nullptr)
			{
				*angle = atan2(n0y, n0x);
			}
		}

		template<typename tFlt>
		static void ConvertFromHessianToPointAndVector(tFlt angle, tFlt distance, tFlt* pX, tFlt* pY, tFlt* pVecX, tFlt* pVecY)
		{
			tFlt tX = cos(angle)*distance;
			tFlt tY = sin(angle)*distance;
			if (pX != nullptr)
			{
				*pX = tX;
			}

			if (pY != nullptr)
			{
				*pY = tY;
			}

			if (pVecX!=nullptr)
			{
				*pVecX = -tX;
			}

			if (pVecY != nullptr)
			{
				*pVecY = tY;
			}
		}

		template<typename tSum, typename ForwardIterator>
		static typename ForwardIterator::value_type CalculateAverage(ForwardIterator first, ForwardIterator last)
		{
			tSum sum = 0;
			size_t count = 0;
			while (first != last)
			{
				++count;
				sum += *first;
				++first;
			}

			return (ForwardIterator::value_type)(sum / count);
		}

		template<typename tFlt>
		static tFlt CalculateAverage(std::function<bool(tFlt&)> getValue)
		{
			tFlt sum = 0;
			size_t count = 0;
			tFlt v;
			for (;;)
			{
				if (getValue(v) == false)
				{
					return sum / count;
				}

				++count;
				sum += v;
			}
		}

		/// <summary>
		/// Project point onto the line define by (pt1OnLineX,pt1OnLineY) and (pt2OnLineX,pt1OnLineY). We return a float that gives the factor
		/// to be multiplied by the vector (pt2OnLine - pt1OnLine) to give the projected point on the line - so
		/// projected point = pt1OnLine + value * (pt2OnLine - pt1OnLine).
		/// If the returned value is between 0 and 1, then the projected point is between the two points.
		/// </summary>
		/// <param name="px">		 X-coordinate of the point to project onto the line.</param>
		/// <param name="py">		 y-coordinate of the point to project onto the line.</param>
		/// <param name="pt1OnLineX">X-coordinate of the 1st point on the line.</param>
		/// <param name="pt1OnLineY">Y-coordinate of the 1st point on the line.</param>
		/// <param name="pt2OnLineX">X-coordinate of the 2nd point on the line.</param>
		/// <param name="pt2OnLineY">Y-coordinate of the 2nd point on the line.</param>
		/// <returns>The value so that "projected point" = pt1OnLine + value * (pt2OnLine - pt1OnLine).</returns>
		template <typename tFlt>
		static tFlt ProjectPointOntoLine(tFlt px, tFlt py, tFlt pt1OnLineX, tFlt pt1OnLineY, tFlt pt2OnLineX, tFlt pt2OnLineY)
		{
			tFlt v1x = pt2OnLineX - pt1OnLineX;
			tFlt v1y = pt2OnLineY - pt1OnLineY;

			tFlt v2x = px - pt1OnLineX;
			tFlt v2y = py - pt1OnLineY;

			tFlt n = v1x*v2x + v1y*v2y;
			tFlt d = v1x*v1x + v1y*v1y;
			tFlt r = n / d;
			return r;
		}

	};
}