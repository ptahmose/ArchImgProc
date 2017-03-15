#pragma once

#include "utils.h"

namespace ArchImgProc
{
	template<typename tCnt, typename tIdx>
	class CHoughAccumulator
	{
	private:
		int binCntX, binCntY;
		std::vector<tCnt> acc;
		std::vector<tIdx> sortedIdx;
	public:
		CHoughAccumulator(int numberOfBinsX, int numberOfBinsY) : binCntX(numberOfBinsX), binCntY(numberOfBinsY)
		{
			this->acc.resize(numberOfBinsX*numberOfBinsY);
			this->sortedIdx.resize(numberOfBinsX*numberOfBinsY);
			for (tIdx i = 0; i < numberOfBinsX*numberOfBinsY; ++i)
			{
				this->sortedIdx[i] = i;
			}
		}

		void Add(int binNoX, int binNoY, tCnt val)
		{
			this->acc[binNoX + binNoY*this->binCntX] += val;
		}

		void Sort()
		{
			std::sort(this->sortedIdx.begin(), this->sortedIdx.end(),
				[&](int a, int  b)->bool
			{
				return this->acc[a] > this->acc[b];
			});
		}

		bool GetFromSorted(tIdx i, int& x, int& y, int* count) const
		{
			if (i >= this->sortedIdx.size())return false;
			int idx = this->sortedIdx[i];
			x = idx % this->binCntX;
			y = idx / this->binCntX;
			if (count != nullptr)
			{
				*count = this->acc[this->sortedIdx[i]];
			}
			return true;
		}
	};

	template <typename tFlt, typename tLsIdx>
	class CHoughOnLineSegments
	{
	private:
		int distanceBinsCount;
		int angleBinsCount;
		tFlt maxDistance;
		CHoughAccumulator<float, tLsIdx> accumulator;
	public:
		CHoughOnLineSegments(tFlt maxDistance, int distanceBins, int angleBins)
			: distanceBinsCount(distanceBins),
			angleBinsCount(angleBins),
			accumulator(angleBins, distanceBins),
			maxDistance(maxDistance)
		{
		}

		void Add(tLsIdx id, tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			tFlt angle, distance;
			CalcAngleAndDistance(x1, y2, x2, y2, &angle, &distance);

			// angle and distance are the parameters of the Hesseian normal form
			int binIdxX = GetBinIdxFromAngle<float>(angle);
			int binIdxY = GetBinIdxFromDistance(distance);
			int val = (int)ceil(CUtils::CalcDistance(x1, y1, x2, y2));
			this->__super::Add(binIdxX, binIdxY, val);
		}

	private:
		int GetBinIdxFromDistance(tFlt dist)
		{
			tFlt f = dist / this->maxDistance;
			if (f > 1)
			{
				f = 1;
			}

			int idx = (int)floor(f * NumberOfBinsForDistance);
			return idx;
		}

		template <typename tFlt>
		static int GetBinIdxFromAngle(tFlt angle)
		{
			typename tFlt v = angle + (tFlt)(M_PI);
			v = v / (M_PI * 2);
			// v is now in the range 0...1
			int idx = (int)(v * NumberOfBinsForAngle);
			if (idx < 0)
			{
				idx = 0;
			}
			else if (idx >= NumberOfBinsForAngle)
			{
				idx = NumberOfBinsForAngle;
			}

			return idx;
		}

		static void CalcAngleAndDistance(tFlt x1, tFlt y1, tFlt x2, tFlt y2, tFlt* angle, tFlt* distance)
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