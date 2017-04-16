#pragma once

#include <limits> 

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
	public:
		struct Parameters
		{
			/// <summary>The maximum angle difference (in radians).</summary>
			tFlt	maxAngleDiff;
			tFlt	maxDistance;

			void SetDefaults()
			{
				this->maxDistance = 8;
				this->maxAngleDiff = CUtils::DegToRad(9.f);
			}
		};
	private:
		const std::vector<tLineSegment>& lines;
		std::vector<size_t>& indices;
		int width, height;
		Parameters parameters;
	public:
		class Result
		{
			tFlt x1, y1, x2, y2;
		};
	public:
		CHoughLineRefiner(const std::vector<tLineSegment>& lines, std::vector<size_t>& indices, int width, int height)
			:lines(lines), indices(indices), width(width), height(height)
		{
			this->parameters.SetDefaults();
		}

		std::vector<size_t> Refine()
		{
			ArchImgProc::Point<float> p; ArchImgProc::Vector2<float> dir;
			this->CalcPointAndDirectionVectorByAveragingLineSegments(p, dir);

			// create normalized
			auto linkedSegments = this->CreateLinkedSegments(p, dir);

			std::vector<size_t> additionalLineSegments;
			// line segments in "linkedSegments": p + linkedSegments[i].start * dir , p + linkedSegments[i].end * dir
			this->SearchLineSegmentsWithAngleInRangeAndDistanceLess(p, dir, linkedSegments, this->parameters.maxAngleDiff, this->parameters.maxDistance,
				[&](size_t i)->void {additionalLineSegments.push_back(i); });

			return additionalLineSegments;
		}

	private:
		void SearchLineSegmentsWithAngleInRangeAndDistanceLess(ArchImgProc::Point<float>& p, ArchImgProc::Vector2<float> dir, const std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEnd>& lineStartStop, float maxAngleDiff, float maxDistance, std::function<void(size_t)> addLineSegment)
		{
			tFlt angle = atan(dir.y / dir.x);
			std::vector<int> candidates;
			for (std::vector<tLineSegment>::const_iterator it = this->lines.cbegin(); it != this->lines.cend(); ++it)
			{
				auto index = it - this->lines.cbegin();
				if (std::find(this->indices.cbegin(), this->indices.cend(), index) != this->indices.cend())
				{
					continue;
				}

				tFlt angleOfSegment = atan((it->val[1] - it->val[3]) / (it->val[0] - it->val[2]));
				if (abs(angleOfSegment - angle) > this->parameters.maxAngleDiff)
				{
					if (angleOfSegment > angle)
					{
						angleOfSegment -= (tFlt)3.14159265358979323846;
					}
					else
					{
						angle -= (tFlt)3.14159265358979323846;
					}

					if (abs(angleOfSegment - angle) > this->parameters.maxAngleDiff)
					{
						continue;
					}
				}

				tFlt minDistance = this->CalcMinDistance(p,dir, lineStartStop, it->val[0], it->val[1], it->val[2], it->val[3]);
				////ArchImgProc::Point<float> dummyPt;
				////tFlt distance = CsgUtils::SegmentSegmentDistanceSquared(
				////	p.x, p.y, p.x + dir.x, p.y + dir.y,
				////	it->val[0], it->val[1], it->val[2], it->val[3], dummyPt.x, dummyPt.y);
				////distance = sqrt(distance);
				if (minDistance > this->parameters.maxDistance)
				{
					continue;
				}

				addLineSegment(index);
			}
		}

		tFlt CalcMinDistance(const ArchImgProc::Point<float>& p, const  ArchImgProc::Vector2<float> dir, const std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEnd>& lineStartStop,
			tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			tFlt minDist = (std::numeric_limits<tFlt>::max)();
			for (const auto lsStartEnd : lineStartStop)
			{
				ArchImgProc::Point<tFlt> p1{ p.x + dir.x*lsStartEnd.start,p.y + dir.y*lsStartEnd.start };
				ArchImgProc::Point<tFlt> p2{ p.x + dir.x*lsStartEnd.end,p.y + dir.y*lsStartEnd.end };

				ArchImgProc::Point<float> dummyPt;
				tFlt distance = sqrt(CsgUtils::SegmentSegmentDistanceSquared(p1.x,p1.y,p2.x,p2.y,x1,y1,x2,y2, dummyPt.x, dummyPt.y));
				if (distance < minDist)
				{
					minDist = distance;
				}
			}

			return minDist;
		}


		std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEnd> CreateLinkedSegments(const ArchImgProc::Point<float>& point, const ArchImgProc::Vector2<float>& normalizedDir)
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
			return linked2;
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