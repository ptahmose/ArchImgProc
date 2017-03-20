#pragma once

#include "basictypes.h"

namespace ArchImgProc
{
	class CsgUtils
	{
	public:
		template<typename tFlt>
		static void HesseNormalFormToTwoPoints(tFlt angle, tFlt distance, Point<tFlt>& p1, Point<tFlt>& p2)
		{
			tFlt angleDeg = 180 * angle / ((tFlt)3.14159265358979323846);
			Point<tFlt> p;
			tFlt cosOfAngle = cos(angle);
			tFlt sinOfAngle = sin(angle);
			p.x = cosOfAngle * distance;
			p.y = sinOfAngle * distance;
			tFlt dirX = sinOfAngle;
			tFlt dirY = -cosOfAngle;

			p1 = p2 = p;
			p2.x += dirX;
			p2.y += dirY;
		}


		enum class ContactResult
		{
			None,
			OnePoint,
			TwoPoints
		};

		template<typename tFlt>
		struct IntersectionRectLineResult
		{
			ContactResult result;
			Point<tFlt> pt1, pt2;
		};

		/// Calculates the intersection points of an infinite line (through P1 and P2) and
		/// the rectangle rect.
		/// \param p1   The first point (on the infinite line).
		/// \param p2   The second point (on the infinite line).
		/// \param rect The rectangle.
		/// \return The calculated intersection result.
		template<typename tFlt>
		static IntersectionRectLineResult<tFlt> CalcIntersectionPoints(const Point<tFlt>& p1, const Point<tFlt>& p2, const IntRect& rect)
		{
			IntersectionRectLineResult<tFlt> result;
			Point<tFlt>* rsltPts[2] = { &result.pt1,&result.pt2 };
			int resultPtsFnd = 0;

			//         +---------------------------
			//         |               3  
			//         |       +--------------+
			//         |       |              |
			//         |     1 |             2|
			//         |       |       4      |
			//         |       +--------------+
			//         |
			Point<tFlt> l1, l2;
			l1 = l2 = { (tFlt)rect.x,(tFlt)rect.y };
			l2.y += rect.h;
			Point<tFlt> p;
			LineLineIntersectionType llit = CalcIntersection(l1, l2, p1, p2, p);
			if (llit == LineLineIntersectionType::Point)
			{
				*rsltPts[resultPtsFnd++] = p;
			}

			l1.x = l2.x = rect.x + rect.w;
			llit = CalcIntersection(l1, l2, p1, p2, p);
			if (llit == LineLineIntersectionType::Point)
			{
				*rsltPts[resultPtsFnd++] = p;
				if (resultPtsFnd == 2)
				{
					result.result = ContactResult::TwoPoints;
					return result;
				}
			}

			l1 = l2 = { (tFlt)rect.x,(tFlt)rect.y };
			l2.x += rect.w;
			llit = CalcIntersection(l1, l2, p1, p2, p);
			if (llit == LineLineIntersectionType::Point)
			{
				*rsltPts[resultPtsFnd++] = p;
				if (resultPtsFnd == 2)
				{
					result.result = ContactResult::TwoPoints;
					return result;
				}
			}

			l1.y = l2.y = (tFlt)rect.x + rect.h;
			llit = CalcIntersection(l1, l2, p1, p2, p);
			if (llit == LineLineIntersectionType::Point)
			{
				*rsltPts[resultPtsFnd++] = p;
				if (resultPtsFnd == 2)
				{
					result.result = ContactResult::TwoPoints;
					return result;
				}
			}

			result.result = ContactResult::None;
			return result;
		}

		enum class LineLineIntersectionType
		{
			None,
			Point,
			Line
		};

		/// Calculates the intersection between the infinite line through Q1 and Q2 and the line segment from P1 and P2.
		/// \param p1					 The point P1.
		/// \param p2					 The point P2.
		/// \param q1					 The point Q1.
		/// \param q2					 The point Q2.
		/// \param [in,out] point of intersection (if it exists and the return value is "Point".
		/// \return The calculated type of intersection.
		template<typename tFlt>
		static LineLineIntersectionType CalcIntersection(const Point<tFlt>& p1, const Point<tFlt>& p2, const Point<tFlt>& q1, const Point<tFlt>& q2, Point<tFlt>& intersection)
		{
			// solve the equation Q1 + s (Q2 - Q1) = P1 + t (P2 - P1)
			//
			// if 0<t<1 then we have a solution, otherwise not

			tFlt t = (-q1.y*q2.x + p1.y*(-q1.x + q2.x) + p1.x*(q1.y - q2.y) + q1.x*q2.y) /
				(-(p1.y - p2.y)*(q1.x - q2.x) + (p1.x - p2.x)*(q1.y - q2.y));
			if (t < 0 || t > 1)
			{
				return LineLineIntersectionType::None;
			}

			if (std::isnan(t))
			{
				return LineLineIntersectionType::Line;
			}

			intersection.x = p1.x + t*(p2.x - p1.x);
			intersection.y = p1.y + t*(p2.y - p1.y);

			return LineLineIntersectionType::Point;
		}
	};
}