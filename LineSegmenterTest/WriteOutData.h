#pragma once

#include <fstream>
#include <tuple>
#include <functional>

using namespace std;

class CWriteOutData
{
public:
	template<typename tFlt>
	static void WriteLineSegmentsAsSvg(std::function<bool(tFlt& x1, tFlt& y1, tFlt& x2, tFlt& y2, tFlt* ptrstrokeWidth, std::string& color)> getLine, int xsize, int ysize, ostream& ost)
	{
		if (xsize <= 0 || ysize <= 0)
		{
			throw std::logic_error("Error: invalid image size in write_svg.");
		}

		ost << "<?xml version=\"1.0\" standalone=\"no\"?>" << std::endl;
		ost << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"" << std::endl;
		ost << " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << std::endl;
		ost << "<svg width=\"" << xsize << "px\" height=\"" << ysize << "px\" ";
		ost << "version=\"1.1\"\n xmlns=\"http://www.w3.org/2000/svg\" ";
		ost << "xmlns:xlink=\"http://www.w3.org/1999/xlink\">" << std::endl;

		/* write line segments */
		for (;;)
		{
			std::string color = "black";
			tFlt x1, x2, y1, y2, strokeWidth;
			strokeWidth = 1;
			if (!getLine(x1, y1, x2, y2, &strokeWidth, color))
			{
				break;
			}

			ost << "  <line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke-width=\"" << strokeWidth << "\" stroke=\"" << color.c_str() << "\" />" << std::endl;
		}

		ost << "</svg>" << std::endl;
	}

	template<typename tFlt>
	static void WriteLineSegmentsAsSvg(std::function<bool(tFlt& x1, tFlt& y1, tFlt& x2, tFlt& y2, tFlt* ptrstrokeWidth, std::string& color)> getNextLine, int xsize, int ysize, const wchar_t* szwFilename)
	{
		std::ofstream ofile(szwFilename, ios::out);
		WriteLineSegmentsAsSvg<tFlt>(getNextLine, xsize, ysize, ofile);
	}

	template<typename ForwardIterator>
	static std::tuple<int, int> DetermineSize(ForwardIterator begin, ForwardIterator end)
	{
		decltype(begin->x1) xmin, xmax, ymin, ymax;
		xmin = ymin = (std::numeric_limits<decltype(begin->x1)>::max)();
		xmax = ymax = (std::numeric_limits<decltype(begin->x1)>::min)();
		for (; begin != end; ++begin)
		{
			auto xminmax = std::minmax(begin->x1, begin->x2);
			auto yminmax = std::minmax(begin->y1, begin->y2);
			if (std::get<0>(xminmax) < xmin)
			{
				xmin = std::get<0>(xminmax);
			}

			if (std::get<1>(xminmax) > xmax)
			{
				xmax = std::get<1>(xminmax);
			}

			if (std::get<0>(yminmax) < ymin)
			{
				ymin = std::get<0>(yminmax);
			}

			if (std::get<1>(yminmax) > ymax)
			{
				ymax = std::get<1>(yminmax);
			}
		}

		return std::make_pair((int)std::ceil(xmax - xmin), (int)std::ceil(ymax - ymin));
	}

	template<typename tFlt, typename ForwardIterator>
	static void WriteLineSegmentsAsSvg(ForwardIterator begin, ForwardIterator end, const wchar_t* szwFilename)
	{
		std::ofstream ofile(szwFilename, ios::out);
		auto size = DetermineSize(begin, end);
		auto it = begin;
		WriteLineSegmentsAsSvg<tFlt>(
			[&](tFlt& x1, tFlt& y1, tFlt& x2, tFlt& y2, tFlt* pStrokewidth, std::string& color)->bool
		{
			if (it == end)
			{
				return false;
			}

			x1 = it->x1;
			y1 = it->y1;
			x2 = it->x2;
			y2 = it->y2;
			++it;
			return true;
		},
			get<0>(size),
			get<1>(size),
			ofile);
	}


	template <typename tFlt>
	static void WriteLineSegmentAsSvg(const std::vector<ArchImgProc::LineSegment<tFlt>>& lineSegments, const wchar_t* szwFilename)
	{
		std::ofstream ofile(szwFilename, ios::out);
		auto size = DetermineSize(lineSegments.cbegin(), lineSegments.cend());

		auto it = lineSegments.cbegin();
		WriteLineSegmentsAsSvg(
			[&](tFlt& x1, tFlt& y1, tFlt& x2, tFlt& y2, tFlt* pStrokewidth, std::string& color)->bool
		{
			if (it == lineSegments.cend())
			{
				return false;
			}

			x1 = it->x1;
			y1 = it->y1;
			x2 = it->x2;
			y2 = it->y2;
			return true;
		},
			get<0>(size),
			get<1>(size),
			ofile);

	}
};
