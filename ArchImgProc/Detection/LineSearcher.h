#pragma once

#include <limits> 
#include <opencv2/core/matx.hpp>

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

		struct LineSegmentStartEndWithIndices
		{
			LineSegmentStartEnd startEnd;
			std::vector<size_t> indices;
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

		static std::vector<LineSegmentStartEndWithIndices> CreateLineSegmentsStartStopIndex(
			const ArchImgProc::Point<tFlt>& lineToProjectP1,
			const ArchImgProc::Point<tFlt>& lineToProjectP2,
			std::function<bool(ArchImgProc::Point<tFlt>& p1, ArchImgProc::Point<tFlt>& p2, size_t& index)> getPoints)
		{
			std::vector<LineSegmentStartEndWithIndices> result;
			for (;;)
			{
				ArchImgProc::Point<tFlt> p1, p2; size_t index;
				bool b = getPoints(p1, p2, index);
				if (b == false)
				{
					break;
				}

				auto l1 = CUtils::ProjectPointOntoLine(p1.x, p1.y, lineToProjectP1.x, lineToProjectP1.y, lineToProjectP2.x, lineToProjectP2.y);
				auto l2 = CUtils::ProjectPointOntoLine(p2.x, p2.y, lineToProjectP1.x, lineToProjectP1.y, lineToProjectP2.x, lineToProjectP2.y);

				LineSegmentStartEndWithIndices lsStartEndIndex;
				lsStartEndIndex.indices.push_back(index);
				if (l1 < l2)
				{
					lsStartEndIndex.startEnd.start = l1;
					lsStartEndIndex.startEnd.end = l2;
				}
				else
				{
					lsStartEndIndex.startEnd.start = l2;
					lsStartEndIndex.startEnd.end = l1;
				}

				result.push_back(lsStartEndIndex);
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

		static void Sort(std::vector<LineSegmentStartEndWithIndices>& v)
		{
			std::sort(v.begin(), v.end(),
				[](const LineSegmentStartEndWithIndices & a, const LineSegmentStartEndWithIndices & b) -> bool
			{
				return a.startEnd.start < b.startEnd.start;
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

		static std::vector<LineSegmentStartEndWithIndices> LinkOverlapping(const std::vector<LineSegmentStartEndWithIndices>& v)
		{
			if (v.size() < 2)
			{
				// create a copy and return it
				return std::vector<LineSegmentStartEndWithIndices>(v);
			}

			std::vector<LineSegmentStartEndWithIndices> result;
			typename std::vector<LineSegmentStartEndWithIndices>::const_iterator it = v.begin();
			LineSegmentStartEndWithIndices cur = *it;
			++it;
			for (; it != v.end(); ++it)
			{
				if (cur.startEnd.end >= it->startEnd.start)
				{
					cur.startEnd.end = (std::max)(cur.startEnd.end, it->startEnd.end);
					std::copy(it->indices.cbegin(), it->indices.cend(), std::back_inserter(cur.indices));
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

		static std::vector<LineSegmentStartEndWithIndices> LinkSmallGaps(const std::vector<LineSegmentStartEndWithIndices>& v, tFlt maxGap)
		{
			if (v.size() < 2)
			{
				// create a copy and return it
				return std::vector<LineSegmentStartEndWithIndices>(v);
			}

			std::vector<LineSegmentStartEndWithIndices> result;
			typename std::vector<LineSegmentStartEndWithIndices>::const_iterator it = v.begin();
			LineSegmentStartEndWithIndices cur = *it;
			++it;
			for (; it != v.end(); ++it)
			{
				if ((cur.startEnd.end + maxGap) >= it->startEnd.start)
				{
					cur.startEnd.end = (std::max)(cur.startEnd.end, it->startEnd.end);
					std::copy(it->indices.cbegin(), it->indices.cend(), std::back_inserter(cur.indices));
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

	class GetPointsFromCvVec4f
	{
	private:
		const cv::Vec4f& vec4;
	public:
		explicit GetPointsFromCvVec4f(const cv::Vec4f& vec4) :vec4(vec4) {}
		GetPointsFromCvVec4f() = delete;
		cv::Vec4f::value_type x1() const { return this->vec4.val[0]; }
		cv::Vec4f::value_type y1() const { return this->vec4.val[1]; }
		cv::Vec4f::value_type x2() const { return this->vec4.val[2]; }
		cv::Vec4f::value_type y2() const { return this->vec4.val[3]; }
	};

	template <typename tFlt, typename tLineSegment, typename tGetPointsFromLineSegment = GetPointsFromCvVec4f>
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
		std::vector<size_t> indices;


		/// <summary>
		/// For each iteration we store how many elements have been present in the "indices"-vector before the next iteration 
		/// was run.
		/// </summary>
		std::vector<size_t> indicesPerIteration;

		ArchImgProc::Point<float> lineSegmentsStartpoint;
		ArchImgProc::Vector2<float> lineSegmentsDirectionVector;
		std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEndWithIndices> lineSegments;

		int width, height;
		Parameters parameters;
	public:
		class Result
		{
			tFlt x1, y1, x2, y2;
		};

		struct OriginalLineSegment
		{
			size_t index;
			int iteration;
			ArchImgProc::Point<float> p1, p2;
		};

		struct ResultLineSegment
		{
			ArchImgProc::Point<float> p1, p2;
			std::vector<size_t> origIndices;
		};
	public:
		size_t GetOriginalLineSegmentsCount() const { return this->indices.size(); }
		bool GetOriginalLineSegment(size_t index, OriginalLineSegment* pOut) const
		{
			if (index >= this->GetOriginalLineSegmentsCount())
			{
				return false;
			}

			if (pOut != nullptr)
			{
				pOut->index = this->indices[index];
				pOut->iteration = 0;
				for (auto it = this->indicesPerIteration.cbegin(); it != this->indicesPerIteration.cend(); ++it)
				{
					if (*it < index)
					{
						++(pOut->iteration);
					}
					else
						break;
				}

				tGetPointsFromLineSegment getVal(this->lines[pOut->index]);
				pOut->p1.x = getVal.x1();
				pOut->p1.y = getVal.y1();
				pOut->p2.x = getVal.x2();
				pOut->p2.y = getVal.y2();
				//pOut.p1.x = this->lines[pOut->index].val[0];
				//pOut.p1.y = this->lines[pOut->index].val[1];
				//pOut.p2.x = this->lines[pOut->index].val[2];
				//pOut.p2.y = this->lines[pOut->index].val[3];
			}

			return true;
		}

		size_t GetResultLineSegmentsCount() const
		{
			return this->lineSegments.size();
		}

		bool GetResultLineSegment(size_t index, ResultLineSegment* pOut)const
		{
			if (index >= this->GetResultLineSegmentsCount())
			{
				return false;
			}

			if (pOut != nullptr)
			{
				auto ptrLs = this->lineSegments.data() + index;
				pOut->p1.x = this->lineSegmentsStartpoint.x + ptrLs->startEnd.start * this->lineSegmentsDirectionVector.x;
				pOut->p2.x = this->lineSegmentsStartpoint.x + ptrLs->startEnd.end * this->lineSegmentsDirectionVector.x;
				pOut->p1.y = this->lineSegmentsStartpoint.y + ptrLs->startEnd.start * this->lineSegmentsDirectionVector.y;
				pOut->p2.y = this->lineSegmentsStartpoint.y + ptrLs->startEnd.end * this->lineSegmentsDirectionVector.y;
			}

			return true;
		}

		void EnumOriginalLineSegments(std::function<bool(size_t index, int iteration, tFlt x1, tFlt y1, tFlt x2, tFlt y2)> func) const
		{
			size_t iterInd = 0;
			for (size_t i = 0; i < this->indices.size(); ++i)
			{
				if (iterInd < this->indicesPerIteration.size())
				{
					if (i >= this->indicesPerIteration[iterInd])
					{
						++iterInd;
					}
				}

				size_t ind = this->indices[i];
				tGetPointsFromLineSegment getVal{ this->lines[ind] };
				bool b = func(ind, (int)iterInd, getVal.x1(), getVal.y1(), getVal.x2(), getVal.y2());
				if (b != true)
				{
					break;
				}
			}
		}

		void EnumOriginalLineSegments(std::function<bool(tFlt x1, tFlt y1, tFlt x2, tFlt y2)> func)
		{
			for (size_t i = 0; i < this->lineSegments.size(); ++i)
			{
				tFlt x1 = this->lineSegmentsStartpoint.x + this->lineSegments[i].start * this->lineSegmentsDirectionVector.x;
				tFlt x2 = this->lineSegmentsStartpoint.x + this->lineSegments[i].end * this->lineSegmentsDirectionVector.x;
				tFlt y1 = this->lineSegmentsStartpoint.y + this->lineSegments[i].start * this->lineSegmentsDirectionVector.y;
				tFlt y2 = this->lineSegmentsStartpoint.y + this->lineSegments[i].end * this->lineSegmentsDirectionVector.y;

				bool b = func(x1, y1, x2, y2);
				if (b != true)
				{
					break;
				}
			}
		}

	public:
		CHoughLineRefiner(const std::vector<tLineSegment>& lines, std::vector<size_t>& indices, int width, int height)
			:lines(lines), indices(indices), width(width), height(height)
		{
			this->parameters.SetDefaults();
		}

		void Refine()
		{
			for (;;)
			{
				ArchImgProc::Point<float> p; ArchImgProc::Vector2<float> dir;
				this->CalcPointAndDirectionVectorByAveragingLineSegments(p, dir);

				// create normalized
				auto linkedSegments = this->CreateLinkedSegments(p, dir);

				std::vector<size_t> additionalLineSegments;
				// line segments in "linkedSegments": p + linkedSegments[i].start * dir , p + linkedSegments[i].end * dir
				this->SearchLineSegmentsWithAngleInRangeAndDistanceLess(p, dir, linkedSegments, this->parameters.maxAngleDiff, this->parameters.maxDistance,
					[&](size_t i)->void {additionalLineSegments.push_back(i); });

				if (additionalLineSegments.empty())
				{
					this->lineSegmentsStartpoint = p;
					this->lineSegmentsDirectionVector = dir;
					this->lineSegments = std::move(linkedSegments);
					break;
				}

				this->indicesPerIteration.push_back(this->indices.size());
				std::copy(additionalLineSegments.cbegin(), additionalLineSegments.cend(), std::back_inserter(this->indices));
			}
		}

	private:
		void SearchLineSegmentsWithAngleInRangeAndDistanceLess(ArchImgProc::Point<float>& p, ArchImgProc::Vector2<float> dir, const std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEnd>& lineStartStop, float maxAngleDiff, float maxDistance, std::function<void(size_t)> addLineSegment)
		{
			tFlt angle = atan(dir.y / dir.x);
			std::vector<int> candidates;
			for (typename std::vector<tLineSegment>::const_iterator it = this->lines.cbegin(); it != this->lines.cend(); ++it)
			{
				auto index = it - this->lines.cbegin();
				if (std::find(this->indices.cbegin(), this->indices.cend(), index) != this->indices.cend())
				{
					continue;
				}

				tGetPointsFromLineSegment getVal(*it);
				//tFlt angleOfSegment = atan((it->val[1] - it->val[3]) / (it->val[0] - it->val[2]));
				tFlt angleOfSegment = atan((getVal.y1() - getVal.y2()) / (getVal.x1() - getVal.x2()));
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

				//tFlt minDistance = this->CalcMinDistance(p, dir, lineStartStop, it->val[0], it->val[1], it->val[2], it->val[3]);
				tFlt minDistance = this->CalcMinDistance(p, dir, lineStartStop, getVal.x1(), getVal.y1(), getVal.x2(), getVal.y2());
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
				tFlt distance = std::sqrt(CsgUtils::SegmentSegmentDistanceSquared(p1.x, p1.y, p2.x, p2.y, x1, y1, x2, y2, dummyPt.x, dummyPt.y));
				if (distance < minDist)
				{
					minDist = distance;
				}
			}

			return minDist;
		}

		void SearchLineSegmentsWithAngleInRangeAndDistanceLess(ArchImgProc::Point<float>& p, ArchImgProc::Vector2<float> dir, const std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEndWithIndices>& lineStartStop, float maxAngleDiff, float maxDistance, std::function<void(size_t)> addLineSegment)
		{
			tFlt angle = atan(dir.y / dir.x);
			std::vector<int> candidates;
			for (typename std::vector<tLineSegment>::const_iterator it = this->lines.cbegin(); it != this->lines.cend(); ++it)
			{
				auto index = it - this->lines.cbegin();
				if (std::find(this->indices.cbegin(), this->indices.cend(), index) != this->indices.cend())
				{
					continue;
				}

				tGetPointsFromLineSegment getVal(*it);
				//tFlt angleOfSegment = atan((it->val[1] - it->val[3]) / (it->val[0] - it->val[2]));
				tFlt angleOfSegment = atan((getVal.y1() - getVal.y2()) / (getVal.x1() - getVal.x2()));
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

				//tFlt minDistance = this->CalcMinDistance(p, dir, lineStartStop, it->val[0], it->val[1], it->val[2], it->val[3]);
				tFlt minDistance = this->CalcMinDistance(p, dir, lineStartStop, getVal.x1(), getVal.y1(), getVal.x2(), getVal.y2());
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

		tFlt CalcMinDistance(const ArchImgProc::Point<float>& p, const  ArchImgProc::Vector2<float> dir, const std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEndWithIndices>& lineStartStop,
			tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			tFlt minDist = (std::numeric_limits<tFlt>::max)();
			for (const auto lsStartEnd : lineStartStop)
			{
				ArchImgProc::Point<tFlt> p1{ p.x + dir.x*lsStartEnd.startEnd.start,p.y + dir.y*lsStartEnd.startEnd.start };
				ArchImgProc::Point<tFlt> p2{ p.x + dir.x*lsStartEnd.startEnd.end,p.y + dir.y*lsStartEnd.startEnd.end };

				ArchImgProc::Point<float> dummyPt;
				tFlt distance = std::sqrt(CsgUtils::SegmentSegmentDistanceSquared(p1.x, p1.y, p2.x, p2.y, x1, y1, x2, y2, dummyPt.x, dummyPt.y));
				if (distance < minDist)
				{
					minDist = distance;
				}
			}

			return minDist;
		}

		std::vector<ArchImgProc::CLineSearcher<float>::LineSegmentStartEndWithIndices> CreateLinkedSegments(const ArchImgProc::Point<float>& point, const ArchImgProc::Vector2<float>& normalizedDir)
		{
			auto itIndices = this->indices.cbegin();
			auto lsStartStop = CLineSearcher<float>::CreateLineSegmentsStartStopIndex(point, ArchImgProc::Point<float> { point.x + normalizedDir.x, point.y + normalizedDir.y },
				[&](ArchImgProc::Point<float>& p1Ls, ArchImgProc::Point<float>& p2LS, size_t& index)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				tGetPointsFromLineSegment getVal(this->lines[*itIndices]);
				p1Ls = ArchImgProc::Point<float>{ getVal.x1(), getVal.y1() };
				p2LS = ArchImgProc::Point<float>{ getVal.x2(), getVal.y2() };
				index = *itIndices;// std::distance(this->indices.cbegin(), itIndices);
				/*p1Ls = ArchImgProc::Point<float>{ this->lines[*itIndices].val[0], this->lines[*itIndices].val[1] };
				p2LS = ArchImgProc::Point<float>{ this->lines[*itIndices].val[2] , this->lines[*itIndices].val[3] };*/
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
			//this->CalcTwoPointsOnLineByAveragingLineSegments(p1, p2);

			//this->CalcTwoPointsByLineFitting(p1, p2);
			//this->CalcTwoPointsByLineFittingWeighted(p1, p2);

			this->CalcTwoPointsByLineFitting2Weighted(p1, p2);

			// get a unit-vector in direction from p1 to p2
			dir.x = p2.x - p1.x;
			dir.y = p2.y - p1.y;
			auto lengthDir = std::sqrt(dir.x*dir.x + dir.y*dir.y);
			dir.x /= lengthDir; dir.y /= lengthDir;
			p = p1;
		}

		///*void CalcTwoPointsByLineFitting(ArchImgProc::Point<float>& p1, ArchImgProc::Point<float>& p2)
		//{
		//	float a, b;
		//	CUtils::LineFit<float>([&](size_t n, float& x, float& y)->bool
		//	{
		//		size_t idx = n / 2;
		//		if (idx >= this->indices.size())
		//		{
		//			return false;
		//		}

		//		if (n & 1)
		//		{
		//			x = this->lines[this->indices[idx]].val[2];
		//			y = this->lines[this->indices[idx]].val[3];
		//		}
		//		else
		//		{
		//			x = this->lines[this->indices[idx]].val[0];
		//			y = this->lines[this->indices[idx]].val[1];
		//		}

		//		return true;
		//	}, &a, &b);

		//	p1.x = 0;
		//	p1.y = a;
		//	p2.x = 10;
		//	p2.y = a + b * 10;
		//}*/

		/*void CalcTwoPointsByLineFittingWeighted(ArchImgProc::Point<float>& p1, ArchImgProc::Point<float>& p2)
		{
			float a, b;
			CUtils::LineFitWeighted<float>([&](size_t n, float& x, float& y, float& weight)->bool
			{
				size_t idx = n / 2;
				if (idx >= this->indices.size())
				{
					return false;
				}

				float dx = this->lines[this->indices[idx]].val[0] - this->lines[this->indices[idx]].val[2];
				float dy = this->lines[this->indices[idx]].val[1] - this->lines[this->indices[idx]].val[3];
				weight = std::sqrt(dx*dx + dy*dy);
				if (n & 1)
				{
					x = this->lines[this->indices[idx]].val[2];
					y = this->lines[this->indices[idx]].val[3];
				}
				else
				{
					x = this->lines[this->indices[idx]].val[0];
					y = this->lines[this->indices[idx]].val[1];
				}

				return true;
			}, &a, &b);

			p1.x = 0;
			p1.y = a;
			p2.x = 10;
			p2.y = a + b * 10;
		}*/

		void CalcTwoPointsByLineFitting2Weighted(ArchImgProc::Point<float>& p1, ArchImgProc::Point<float>& p2)
		{
			float a, b, c;
			float curWeight;
			CUtils::LineFit2Weighted<float>([&](size_t n, float& x, float& y, float& weight)->bool
			{
				size_t idx = n / 2;
				if (idx >= this->indices.size())
				{
					return false;
				}

				size_t idxIntoLines = this->indices[idx];
				tGetPointsFromLineSegment getVal(this->lines[idxIntoLines]);
				if (n & 1)
				{
					//x = this->lines[idxIntoLines].val[2];
					x = getVal.x2();
					//y = this->lines[idxIntoLines].val[3];
					y = getVal.y2();
					weight = curWeight;
				}
				else
				{
					auto dx = getVal.x1() - getVal.x2();// this->lines[idxIntoLines].val[0] - this->lines[idxIntoLines].val[2];
					auto dy = getVal.y1() - getVal.y2();//this->lines[idxIntoLines].val[1] - this->lines[idxIntoLines].val[3];
					weight = std::sqrt(dx*dx + dy*dy);

					// Since we use the same weight on both points of a linesegment, we store the value (with the first point of 
					// a line segment) so that we do not have to re-calculate for the second point.
					// Note that we require that the counter ("n") starts with 0 and is consecutive.
					curWeight = weight;
					x = getVal.x1();// this->lines[idxIntoLines].val[0];
					y = getVal.y1(); //this->lines[idxIntoLines].val[1];
				}

				return true;
			}, a, b, c);

			if (std::fabs(b) > std::fabs(a))
			{
				// use the formula y =  (-a / b) * x + (-c / b)
				float bprime = -a / b;
				float aprime = -c / b;

				p1.x = 0;
				p1.y = aprime;
				p2.x = 10;
				p2.y = aprime + bprime * 10;
			}
			else
			{
				// use the formula x = (-b / a) * y + (-c / a)
				float bprime = -b / a;
				float aprime = -c / a;

				p1.x = aprime;
				p1.y = 0;
				p2.x = aprime + bprime * 10;
				p2.y = 10;
			}
		}


		/*void CalcTwoPointsOnLineByAveragingLineSegments(ArchImgProc::Point<float>& p1, ArchImgProc::Point<float>& p2)
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
			avgAngle = CUtils::CalculateWeightedAverage<float>(
				[&](float& v, float& weight)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				float angle;
				CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, &angle, (float*)nullptr);
				float dx = (lines[*itIndices].val[0]) - (lines[*itIndices].val[2]);
				float dy = (lines[*itIndices].val[1]) - (lines[*itIndices].val[3]);
				weight = std::sqrt(dx*dx + dy*dy);
				++itIndices;
				v = angle;
			});

			itIndices = this->indices.cbegin();
			avgDistance = CUtils::CalculateWeightedAverage<float>(
				[&](float& v, float& weight)->bool
			{
				if (itIndices == this->indices.cend())
				{
					return false;
				}

				float distance;
				CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, (float*)nullptr, &distance);

				float dx = (lines[*itIndices].val[0]) - (lines[*itIndices].val[2]);
				float dy = (lines[*itIndices].val[1]) - (lines[*itIndices].val[3]);
				weight = std::sqrt(dx*dx + dy*dy);

				++itIndices;
				v = distance;
			});
		}*/
	};
}
