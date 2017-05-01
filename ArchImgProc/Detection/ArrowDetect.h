#pragma once

#include <cassert>
#include "csgutils.h"
#include "utils.h"
#include "LineSearcher.h"

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

		bool GetFromSorted(tIdx i, int& x, int& y, tCnt* count) const
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
		struct BinResult
		{
			tFlt length;
			tFlt angleMin;
			tFlt angleMax; 
			tFlt distanceMin;
			tFlt distanceMax;
		};
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
			if (std::fabs(x1-x2)<0.001 && std::fabs(y1 - y2)<0.001)
			{
				return;
			}

			tFlt angle, distance;
			CalcAngleAndDistance(x1, y1, x2, y2, &angle, &distance);

			// normalize angle
			if (angle< (tFlt)-3.14159265358979323846)
			{
				angle += (tFlt)(2 * 3.14159265358979323846);
			}
			else if (angle>(tFlt)3.14159265358979323846)
			{
				angle -= (tFlt)(2 * 3.14159265358979323846);
			}

			//printf("Angle: %f\n", CUtils::RadToDeg(angle));

			// angle and distance are the parameters of the Hesseian normal form
			int binIdxX = GetBinIdxFromAngle<float>(angle);
			int binIdxY = GetBinIdxFromDistance(distance);
			int val = (int)ceil(CUtils::CalcDistance(x1, y1, x2, y2));
			this->accumulator.Add(binIdxX, binIdxY, val);
		}

		void Sort()
		{
			this->accumulator.Sort();
		}

		bool GetAngleAndDistanceMaxMinSortedByCount(int i, BinResult* result)
		{
			if (result!=nullptr)
			{
				return this->GetAngleAndDistanceMaxMinSortedByCount(i, &result->length, &result->angleMin, &result->angleMax, &result->distanceMin, &result->distanceMax);
			}
			else
			{
				return this->GetAngleAndDistanceMaxMinSortedByCount(i, nullptr, nullptr, nullptr, nullptr, nullptr);
			}
		}

		bool GetAngleAndDistanceMaxMinSortedByCount(int i, float* length, float* angleMin, float* angleMax, float* distanceMin, float* distanceMax)
		{
			//TestAngle();

			int x, y;
			bool b = this->accumulator.GetFromSorted(i, x, y, length);
			if (b == false)
			{
				return false;
			}

			if (angleMin != nullptr)
			{
				//float a = (float(x) / this->angleBinsCount/*NumberOfBinsForAngle*/)*1.5*3.14159265358979323846 - 3.14159265358979323846 / 2;
				float a = (float(x) / this->angleBinsCount/*NumberOfBinsForAngle*/)*2*3.14159265358979323846 - 3.14159265358979323846 ;
				*angleMin = a;
			}

			if (angleMax != nullptr)
			{
				//float a = (float(x + 1) / this->angleBinsCount/*NumberOfBinsForAngle*/)*1.5*3.14159265358979323846 - 3.14159265358979323846 / 2;
				float a = (float(x + 1) / this->angleBinsCount/*NumberOfBinsForAngle*/)*2*3.14159265358979323846 - 3.14159265358979323846 ;
				*angleMax = a;
			}

			if (distanceMin != nullptr)
			{
				float d = (float(y) / this->distanceBinsCount/* NumberOfBinsForDistance*/)*this->maxDistance;
				*distanceMin = d;
			}

			if (distanceMax != nullptr)
			{
				float d = (float(y + 1) / this->distanceBinsCount /*NumberOfBinsForDistance*/)*this->maxDistance;
				*distanceMax = d;
			}

			return true;
		}

		static void FindItemsInRange(std::function<bool(tLsIdx&, tFlt&, tFlt&, tFlt&, tFlt&)> func, tFlt  angleMin, tFlt angleMax, tFlt distMin, tFlt distMax, std::vector<tLsIdx>& indices)
		{
			indices.clear();
			tFlt x1, x2, y1, y2;
			tLsIdx idx;
			for (;;)
			{
				if (!func(idx, x1, y1, x2, y2))
				{
					return;
				}

				tFlt angle, distance;
				CalcAngleAndDistance(x1, y1, x2, y2, &angle, &distance);

				/*if (angle - angleMin > (tFlt)(3.14159265358979323846))
				{
					angle -= (tFlt)3.14159265358979323846;
				}
				else if (angle - angleMin < -(tFlt)(3.14159265358979323846))
				{
					angle += (tFlt)3.14159265358979323846;
				}*/

				if ((angleMin <= angle&&angleMax >= angle) && (distMin <= distance&&distMax >= distance))
				{
					indices.push_back(idx);
				}
			}
		}

	private:
		int GetBinIdxFromDistance(tFlt dist)
		{
			tFlt f = dist / this->maxDistance;
			if (f > 1)
			{
				f = 1;
			}

			int idx = (int)floor(f * this->distanceBinsCount);
			return idx;
		}

		template <typename tFlt>
		int GetBinIdxFromAngle(tFlt angle)
		{
			typename tFlt v = angle + (tFlt)(3.14159265358979323846 );
			v = v / (tFlt)(3.14159265358979323846 * 2);
			// v is now in the range 0...1
			
			assert(v >= 0 && v <= 1);

			int idx = (int)(v * this->angleBinsCount);
			if (idx < 0)
			{
				idx = 0;
			}
			else if (idx >= this->angleBinsCount)
			{
				idx = this->angleBinsCount;
			}

			return idx;
		}

		static void CalcAngleAndDistance(tFlt x1, tFlt y1, tFlt x2, tFlt y2, tFlt* angle, tFlt* distance)
		{
			CUtils::ConvertToHessianNormalForm(x1, y1, x2, y2, angle, distance);
			/*tFlt nx = y1 - y2;
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
			}*/
		}
	};




}