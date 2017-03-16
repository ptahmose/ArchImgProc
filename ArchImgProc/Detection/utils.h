#pragma once

namespace ArchImgProc
{
	class CUtils
	{
	public:
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
			tFlt _angle;
			_angle = atan2(n0y, n0x);

			// _angle must be >= -90 degree and <= 180 degree (in our case, where all coordinates are positive)
			//assert(_angle >= -M_PI / 2 && _angle <= M_PI);

			if (distance != nullptr)
			{
				*distance = _dist;
			}

			if (angle != nullptr)
			{
				*angle = _angle;
			}
		}

	};
}