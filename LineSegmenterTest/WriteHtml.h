#pragma once

#include <functional>
//#include "lsd.h"
//#include "EllipseFit.h"
#include "inc_EllipseUtils.h"

class CResultAsHtmlOutput
{
public:
	class EllipseOptions
	{
	private:
		std::string color;
		bool drawLineMajorAxis;
		bool drawLineMinorAxis;
	public:
		EllipseOptions() : drawLineMajorAxis(false), drawLineMinorAxis(false) {}
		const std::string& GetColor() const { return this->color; }
		bool GetDrawLineMajorAxis() const { return this->drawLineMajorAxis; }
		bool GetDrawLineMinorAxis() const { return this->drawLineMajorAxis; }

		void SetColor(const char* sz) { this->color = sz; }
		void SetDrawLineMajorAxis(bool b) { this->drawLineMajorAxis = b; }
		void SetDrawLineMinorAxis(bool b) { this->drawLineMinorAxis = b; }
	};

	struct Attributes
	{
		std::string className;
		std::string idText;
		std::string color;

		void Clear() { this->className.clear(); this->idText.clear(); this->color.clear(); }
	};

	struct LineData
	{
		float x0;
		float y0;
		float x1;
		float y1;
	};

	struct SegmentData
	{
		float x0;
		float y0;
		float x1;
		float y1;
		float width;

		void SetDefault()
		{
			this->x0 = this->y0 = this->x1 = this->y1 = std::numeric_limits<float>::quiet_NaN();
			this->width = 1;
		}
	};


	typedef EllipseUtils::EllipseParameters<float> EllipseParams;
private:
	struct HideShowModifierInfo
	{
		std::string text;
		std::string className;
	};
	static const char* szHTML;

	std::function<bool(int, SegmentData&, Attributes&)> getSegments;
	std::function<bool(int, EllipseParams&, CResultAsHtmlOutput::EllipseOptions&)> getEllipse;
	std::function<bool(int, float& x, float&y, std::string& color)> getPoint;
	std::function<bool(int, LineData& lineData, Attributes& attribs)> getLine;
	std::function<bool(int, float& x, float& y)> getPolygonPoints;
	std::vector<HideShowModifierInfo> hideShowModifierInfo;
	std::wstring filenameImage;
	int widthsvg, heightsvg;
	int widthImage, heightImage;
	FILE* fp;
	std::wstring customText;
	float a, b, c, d, e, f;
public:
	CResultAsHtmlOutput(const wchar_t* filename);
	CResultAsHtmlOutput(const std::wstring& filename) : CResultAsHtmlOutput(filename.c_str()) {}
	~CResultAsHtmlOutput();

	void SetImageUrl(const wchar_t* szFilename) { this->filenameImage = szFilename; }
	void SetWidthHeight(int width, int height) { this->SetWidthHeightSvg(width, height); this->SetWidthHeightImage(width, height); }
	void SetWidthHeightSvg(int width, int height) { this->widthsvg = width; this->heightsvg = height; }
	void SetWidthHeightImage(int width, int height) { this->widthImage = width; this->heightImage = height; }
	void SetGetSegments(std::function<bool(int, SegmentData&, Attributes&)> getSegments) { this->getSegments = getSegments; }
	void SetGetEllipses(std::function<bool(int, EllipseParams&, EllipseOptions& option)> getEllipse) { this->getEllipse = getEllipse; }
	void SetGetPoints(std::function<bool(int, float& x, float&y, std::string& color)> getPoint) { this->getPoint = getPoint; }
	void SetGetLines(std::function<bool(int, LineData& lineData, Attributes& attribs)> getLine) { this->getLine = getLine; }
	void SetGetPolygonPoints(std::function<bool(int, float& x, float& y)> getPolygonPoints) { this->getPolygonPoints = getPolygonPoints; }
	void SetImageTransformationMatrix(float a, float b, float c, float d, float e, float f) { this->a = a; this->b = b; this->c = c; this->d = d; this->e = e; this->f = f; }

	void AddCustomTextLine(const char* sz);
	void AddCustomTextLine(const wchar_t* sz);

	void AddShowHideModifierForClass(const char* text, const char* className) { this->hideShowModifierInfo.push_back(HideShowModifierInfo{ text, className }); }
	void AddShowHideModifierForClass(const std::string& text, const std::string& className) { this->hideShowModifierInfo.push_back(HideShowModifierInfo{ text, className }); }

	void Generate();

private:
	void WriteUTF16(const wchar_t* str);

	std::string GenerateSegmentsSvg();
	std::string GenerateEllipsesSvg();
	std::string GeneratePointsSvg();
	std::string GenerateLinesSvg();
	std::string GeneratePolygonSvg();
	void GenerateSegmentsSvg(std::ostream& stream);
	void GenerateEllipsesSvg(std::ostream& stream);
	void GeneratePointsSvg(std::ostream& stream);
	void GenerateLinesSvg(std::ostream& stream);
	void GeneratePolygonSvg(std::ostream& stream);

	std::wstring  Process(const std::string& str, std::function<std::wstring(const char*)> getReplacement);

	static std::wstring AddLineBreaks(std::wstring& str);
};
