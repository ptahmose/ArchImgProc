#pragma once

#include "basictypes.h"

namespace ArchImgProc
{
	class CsgUtils
	{
	private:
		// minimum distance (squared) between vertices, i.e. minimum segment length (squared)
		static constexpr double EPSILON_MIN_VERTEX_DISTANCE_SQUARED() { return 0.00000001; }

		// An arbitrary tiny epsilon.  If you use float instead of double, you'll probably want to change this to something like 1E-7f
		static constexpr double EPSILON_TINY() { return 1.0E-14; }

		// Arbitrary general epsilon.  Useful for places where you need more "slop" than EPSILON_TINY (which is most places).
		// If you use float instead of double, you'll likely want to change this to something like 1.192092896E-04
		static constexpr double EPSILON_GENERAL() { return 1.192092896E-07; }

		template <typename tFlt>
		static 	bool AreValuesEqual(tFlt val1, tFlt val2, tFlt tolerance)
		{
			if (val1 >= (val2 - tolerance) && val1 <= (val2 + tolerance))
			{
				return true;
			}

			return false;
		}
		template <typename tFlt>
		static tFlt PointToPointDistanceSquared(tFlt p1x, tFlt p1y, tFlt p2x, tFlt p2y)
		{
			tFlt dx = p2x - p1x;
			tFlt dy = p2y - p1y;
			return (dx * dx) + (dy * dy);
		}

		template <typename tFlt>
		tFlt PointSegmentDistanceSquared(tFlt px, tFlt py,
			tFlt p1x, tFlt p1y,
			tFlt p2x, tFlt p2y,
			tFlt& t,
			tFlt& qx, tFlt& qy)
		{
			tFlt dx = p2x - p1x;
			tFlt dy = p2y - p1y;
			tFlt dp1x = px - p1x;
			tFlt dp1y = py - p1y;
			const tFlt segLenSquared = (dx * dx) + (dy * dy);
			if (AreValuesEqual(segLenSquared, 0.0, (tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED))
			{
				// segment is a point.
				qx = p1x;
				qy = p1y;
				t = 0.0;
				return ((dp1x * dp1x) + (dp1y * dp1y));
			}
			else
			{
				t = ((dp1x * dx) + (dp1y * dy)) / segLenSquared;
				if (t <= EPSILON_TINY())
				{
					// intersects at or to the "left" of first segment vertex (p1x, p1y).  If t is approximately 0.0, then
					// intersection is at p1.  If t is less than that, then there is no intersection (i.e. p is not within
					// the 'bounds' of the segment)
					if (t >= -(tFlt)EPSILON_TINY())
					{
						// intersects at 1st segment vertex
						t = 0;
					}
					// set our 'intersection' point to p1.
					qx = p1x;
					qy = p1y;
					// Note: If you wanted the ACTUAL intersection point of where the projected lines would intersect if
					// we were doing PointLineDistanceSquared, then qx would be (p1x + (t * dx)) and qy would be (p1y + (t * dy)).
				}
				else if (t >= (1 - (tFlt)EPSILON_TINY()))
				{
					// intersects at or to the "right" of second segment vertex (p2x, p2y).  If t is approximately 1.0, then
					// intersection is at p2.  If t is greater than that, then there is no intersection (i.e. p is not within
					// the 'bounds' of the segment)
					if (t <= (1.0 + (tFlt)EPSILON_TINY()))
					{
						// intersects at 2nd segment vertex
						t = 1;
					}
					qx = p2x;
					qy = p2y;
					// Note: If you wanted the ACTUAL intersection point of where the projected lines would intersect if
					// we were doing PointLineDistanceSquared, then qx would be (p1x + (t * dx)) and qy would be (p1y + (t * dy)).
				}
				else
				{
					// The projection of the point to the point on the segment that is perpendicular succeeded and the point
					// is 'within' the bounds of the segment.  Set the intersection point as that projected point.
					qx = ((1 - t) * p1x) + (t * p2x);
					qy = ((1 - t) * p1y) + (t * p2y);
					// for debugging
					//ASSERT(AreValuesEqual(qx, p1x + (t * dx), EPSILON_TINY));
					//ASSERT(AreValuesEqual(qy, p1y + (t * dy), EPSILON_TINY));
				}

				// return the squared distance from p to the intersection point.
				tFlt dpqx = px - qx;
				tFlt dpqy = py - qy;
				return ((dpqx * dpqx) + (dpqy * dpqy));
			}
		}


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

		template<typename tFlt>
		static tFlt SegmentSegmentDistanceSquared(
			tFlt p1x, tFlt p1y,
			tFlt p2x, tFlt p2y,
			tFlt p3x, tFlt p3y,
			tFlt p4x, tFlt p4y,
			tFlt& qx, tFlt& qy)
		{
			// http://stackoverflow.com/questions/541150/connect-two-line-segments/11427699#11427699

			// check to make sure both segments are long enough (i.e. verts are farther apart than minimum allowed vert distance).
			// If 1 or both segments are shorter than this min length, treat them as a single point.
			tFlt segLen12Squared = PointToPointDistanceSquared<tFlt>(p1x, p1y, p2x, p2y);
			tFlt segLen34Squared = PointToPointDistanceSquared<tFlt>(p3x, p3y, p4x, p4y);
			tFlt t = 0;
			tFlt minDist2 = (tFlt)1E+38;

			if (segLen12Squared <= (tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED())
			{
				qx = p1x;
				qy = p1y;
				if (segLen34Squared <= (tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED())
				{
					// point to point
					minDist2 = PointToPointDistanceSquared(p1x, p1y, p3x, p3y);
				}
				else
				{
					// point - seg
					minDist2 = PointSegmentDistanceSquared(p1x, p1y, p3x, p3y, p4x, p4y);
				}
				return minDist2;
			}
			else if (segLen34Squared <= (tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED())
			{
				// seg - point
				minDist2 = PointSegmentDistanceSquared(p3x, p3y, p1x, p1y, p2x, p2y, t, qx, qy);
				return minDist2;
			}

			// if you have a point class and/or methods to do cross products, you can use those here.
			// This is what we're actually doing:
			// Point2D delta43(p4x - p3x, p4y - p3y);    // dir of p3 -> p4
			// Point2D delta12(p1x - p2x, p1y - p2y);    // dir of p2 -> p1
			// double d = delta12.Cross2D(delta43);
			tFlt d = ((p4y - p3y) * (p1x - p2x)) - ((p1y - p2y) * (p4x - p3x));
			bool bParallel = AreValuesEqual(d, 0, (tFlt)EPSILON_GENERAL());

			if (!bParallel)
			{
				// segments are not parallel.  Check for intersection.
				// Point2D delta42(p4x - p2x, p4y - p2y);    // dir of p2 -> p4
				// t = 1.0 - (delta42.Cross2D(delta43) / d);
				t = 1.0 - ((((p4y - p3y) * (p4x - p2x)) - ((p4y - p2y) * (p4x - p3x))) / d);
				tFlt seg12TEps = sqrt((tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED() / segLen12Squared);
				if (t >= -seg12TEps && t <= (1 + seg12TEps))
				{
					// inside [p1,p2].   Segments may intersect.
					// double s = 1.0 - (delta12.Cross2D(delta42) / d);
					tFlt s = 1 - ((((p4y - p2y) * (p1x - p2x)) - ((p1y - p2y) * (p4x - p2x))) / d);
					tFlt seg34TEps = sqrt((tFlt)EPSILON_MIN_VERTEX_DISTANCE_SQUARED() / segLen34Squared);
					if (s >= -seg34TEps && s <= (1 + seg34TEps))
					{
						// segments intersect!
						minDist2 = 0;
						qx = ((1 - t) * p1x) + (t * p2x);
						qy = ((1 - t) * p1y) + (t * p2y);
						// for debugging
						//double qsx = ((1.0 - s) * p3x) + (s * p4x);
						//double qsy = ((1.0 - s) * p3y) + (s * p4y);
						//ASSERT(AreValuesEqual(qx, qsx, EPSILON_MIN_VERTEX_DISTANCE_SQUARED));
						//ASSERT(AreValuesEqual(qy, qsy, EPSILON_MIN_VERTEX_DISTANCE_SQUARED));
						return minDist2;
					}
				}
			}

			// Segments do not intersect.   Find closest point and return dist.   No other way at this
			// point except to just brute-force check each segment end-point vs opposite segment.  The
			// minimum distance of those 4 tests is the closest point.
			tFlt tmpQx, tmpQy, tmpD2;
			minDist2 = PointSegmentDistanceSquared(p3x, p3y, p1x, p1y, p2x, p2y, t, qx, qy);
			tmpD2 = PointSegmentDistanceSquared(p4x, p4y, p1x, p1y, p2x, p2y, t, tmpQx, tmpQy);
			if (tmpD2 < minDist2)
			{
				qx = tmpQx;
				qy = tmpQy;
				minDist2 = tmpD2;
			}
			tmpD2 = PointSegmentDistanceSquared(p1x, p1y, p3x, p3y, p4x, p4y, t, tmpQx, tmpQy);
			if (tmpD2 < minDist2)
			{
				qx = p1x;
				qy = p1y;
				minDist2 = tmpD2;
			}
			tmpD2 = PointSegmentDistanceSquared(p2x, p2y, p3x, p3y, p4x, p4y, t, tmpQx, tmpQy);
			if (tmpD2 < minDist2)
			{
				qx = p2x;
				qy = p2y;
				minDist2 = tmpD2;
			}

			return minDist2;
		}
	};
}