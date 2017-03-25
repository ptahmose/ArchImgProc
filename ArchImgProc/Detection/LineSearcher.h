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
				if ((cur.end+maxGap) >= it->start)
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
}