#pragma once

namespace ArchImgProc
{
	template <typename tFlt>
	class CLineSearcher
	{
	public:
		struct LineSegmentStartEnd
		{
			tFlt start, end;
		};

		static std::vector<LineSegmentStartEnd> CreateLineSegmentsStartStop(
			const ArchImgProc::Point<tFlt>& lineToProjectP1,
			const ArchImgProc::Point<tFlt>& lineToProjectP2,
			std::function<bool(ArchImgProc::Point<tFlt>& p1, ArchImgProc::Point<tFlt>& p2)> getPoints)
		{
			std::vector<LineSegmentStartEnd> result;
			for (;;)
			{
				ArchImgProc::Point<tFlt> p1, p2;
				bool b = getPoints(p1, p2);
				if (b == false)
				{
					break;
				}

				auto l1 = CUtils::ProjectPointOntoLine(p1.x, p1.y, lineToProjectP1.x, lineToProjectP1.y, lineToProjectP2.x, lineToProjectP2.y);
				auto l2 = CUtils::ProjectPointOntoLine(p2.x, p2.y, lineToProjectP1.x, lineToProjectP1.y, lineToProjectP2.x, lineToProjectP2.y);

				LineSegmentStartEnd lsStartEnd;
				if (l1 < l2)
				{
					lsStartEnd.start = l1;
					lsStartEnd.end = l2;
				}
				else
				{
					lsStartEnd.start = l2;
					lsStartEnd.end = l1;
				}

				result.push_back(lsStartEnd);
			}

			return result;
		}

		static void Sort(std::vector<LineSegmentStartEnd>& v)
		{
			std::sort(v.begin(), v.end(),
				[](const LineSegmentStartEnd & a, const LineSegmentStartEnd & b) -> bool
			{
				return a.start < b.start;
			});
		}

		static std::vector<LineSegmentStartEnd> LinkOverlapping(const std::vector<LineSegmentStartEnd>& v)
		{
			if (v.size() < 2)
			{
				// create a copy and return it
				return std::vector<LineSegmentStartEnd>(v);
			}

			std::vector<LineSegmentStartEnd> result;
			typename std::vector<LineSegmentStartEnd>::const_iterator it = v.begin();
			LineSegmentStartEnd cur = *it;
			++it;
			for (; it != v.end(); ++it)
			{
				if (cur.end >= it->start)
				{
					cur.end = (std::max)(cur.end, it->end);
				}
				else
				{
					result.push_back(cur);
					cur = *it;
				}
			}

			result.push_back(cur);
			return result;
		}

		static std::vector<LineSegmentStartEnd> LinkSmallGaps(const std::vector<LineSegmentStartEnd>& v, tFlt maxGap)
		{
			if (v.size() < 2)
			{
				// create a copy and return it
				return std::vector<LineSegmentStartEnd>(v);
			}

			std::vector<LineSegmentStartEnd> result;
			typename std::vector<LineSegmentStartEnd>::const_iterator it = v.begin();
			LineSegmentStartEnd cur = *it;
			++it;
			for (; it != v.end(); ++it)
			{
				if ((cur.end + maxGap) >= it->start)
				{
					cur.end = (std::max)(cur.end, it->end);
				}
				else
				{
					result.push_back(cur);
					cur = *it;
				}
			}

			result.push_back(cur);
			return result;

		}
	};

	template <typename tFlt, typename tLineSegment>
	class CHoughLineRefiner
	{
	private:
		const std::vector<tLineSegment>& lines;
		std::vector<size_t>& indices;
		int width, height;
	public:
		class Result
		{
			tFlt x1, y1, x2, y2;
		};
	public:
		CHoughLineRefiner(const std::vector<tLineSegment>& lines, std::vector<size_t>& indices, int width, int height)
			:lines(lines), indices(indices), width(width), height(height)
		{}

		void Refine()
		{
			ArchImgProc::Point<float> p; ArchImgProc::Vector2<float> dir;
			this->CalcPointAndDirectionVectorByAveragingLineSegments(p, dir);

			// create normalized
			this->CreateLinkedSegments(p, dir);
		}

	private:
		void CreateLinkedSegments(const ArchImgProc::Point<float>& point, const ArchImgProc::Vector2<float>& normalizedDir)
		{
			auto itIndices = this->indices.cbegin();
			auto lsStartStop = CLineSearcher<float>::CreateLineSegmentsStartStop(point, ArchImgProc::Point<float> { point.x + normalizedDir.x, point.y + normalizedDir.y },
				[&](ArchImgProc::Point<float>& p1Ls, ArchImgProc::Point<float>& p2LS)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				p1Ls = ArchImgProc::Point<float>{ this->lines[*itIndices].val[0], this->lines[*itIndices].val[1] };
				p2LS = ArchImgProc::Point<float>{ this->lines[*itIndices].val[2] , this->lines[*itIndices].val[3] };
				++itIndices;
				return true;
			});

			CLineSearcher<float>::Sort(lsStartStop);
			auto linked = CLineSearcher<float>::LinkOverlapping(lsStartStop);
			auto linked2 = CLineSearcher<float>::LinkSmallGaps(linked, 10);
		}

		void CalcPointAndDirectionVectorByAveragingLineSegments(ArchImgProc::Point<float>& p, ArchImgProc::Vector2<float>& dir)
		{
			ArchImgProc::Point<float> p1, p2;
			this->CalcTwoPointsOnLineByAveragingLineSegments(p1, p2);
			// get a unit-vector in direction from p1 to p2
			dir.x = p2.x - p1.x;
			dir.y = p2.y - p1.y;
			auto lengthDir = std::sqrt(dir.x*dir.x + dir.y*dir.y);
			dir.x /= lengthDir; dir.y /= lengthDir;
			p = p1;
		}

		void CalcTwoPointsOnLineByAveragingLineSegments(ArchImgProc::Point<float>& p1, ArchImgProc::Point<float>& p2)
		{
			float avgAngle, avgDistance;
			this->CalcAverageAngleAndDistance(avgAngle, avgDistance);
			CsgUtils::HesseNormalFormToTwoPoints(avgAngle, avgDistance, p1, p2);
			float centerX = width / 2.f;
			float centerY = height / 2.f;
			p1.x += centerX; p2.x += centerX;
			p1.y += centerY; p2.y += centerY;
		}

		void CalcAverageAngleAndDistance(float& avgAngle, float&  avgDistance)
		{
			float centerX = width / 2.f;
			float centerY = height / 2.f;

			std::vector<size_t>::const_iterator itIndices = this->indices.cbegin();
			avgAngle = CUtils::CalculateAverage<float>(
				[&](float& v)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				float angle;
				CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, &angle, (float*)nullptr);
				++itIndices;
				v = angle;
			});

			itIndices = this->indices.cbegin();
			avgDistance = CUtils::CalculateAverage<float>(
				[&](float& v)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				float distance;
				CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, (float*)nullptr, &distance);
				++itIndices;
				v = distance;
			});
		}
	};
}