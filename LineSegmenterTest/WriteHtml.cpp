#include "stdafx.h"
#include "writehtml.h"
#include <locale>
#include <codecvt>
#include <regex>
#include <sstream>
//#include "misc_utils.h"

static std::wstring s2ws(const std::string& str)
{
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

/*static*/const char* CResultAsHtmlOutput::szHTML =
R"literal(<!DOCTYPE html>
<html>
<meta charset="UTF-8">
<body>

<svg id="svgdisp" width="%[widthSvg]px" height="%[heightSvg]px" version="1.1" 	xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
<defs>
	<filter id = "f1">
	<!-- <feColorMatrix type = "matrix" values = "1 0 0 0 , 0 1 0 0 0, 0 0 1 0 0.0, 0 0 0 1 0" /> -->
	<feColorMatrix id = "imageSaturation" type = "saturate" values = "0.4" />
	<feColorMatrix id = "imageColorMatrix" type = "matrix" values = "1 0 0 0 0, 0 1 0 0 0, 0 0 1 0 0, 0 0 0 1 0" />
	</filter>
</defs>

%[beginimage]
<image x="0" y="0" width="%[widthImage]" height="%[heightImage]" filter="url(#f1)" %[imageTransformMatrix] xlink:href="%[imagefilename]" />
%[endimage]

%[linesegmentssvg]

%[ellipsessvg]

%[linessvg]

%[pointssvg]

%[polygonsvg]

</svg>

<style>
	.table1 {
        border: 1px solid black;
    }
	.table1th {
        border: 1px solid black;
    }
	.table1td {
        border: 1px solid black;
    }
    .inputautosize {
        width: calc(100% - 12px); /* IE 9,10 , Chrome, Firefox */
        width: -webkit-calc(100% - 12px); /*For safari 6.0*/
    }
  .bordercustomtext
    {
        border-style:solid;
        border-color:#287EC7;
    } 

	%[modifystyles]
</style>

<table class="table1">
  <tr>
	<th class="table1th">Image Filter </th>
	<th class="table1th" style="width:450px">Value</th>
  </tr>
  <tr>
	<td class="table1td">Saturation</td>
	<td class="table1td"><input id="saturationSlider" class="inputautosize" type="range"  min="0" max="100" onchange="setImageSaturationValue(this.value)" oninput="setImageSaturationValue(this.value)"/></td>
  </tr>
  <tr>
	<td class="table1td">Transparency</td>
	<td class="table1td"><input id="transparencySlider" class="inputautosize" type="range"  min="0" max="100" value="100" onchange="setImageTransparencyValue(this.value)" oninput="setImageTransparencyValue(this.value)"/></td>
  </tr>
</table>

<table class="table1">
  <tr>
	<th class="table1th">Change Display </th>
	<th class="table1th" style="width:450px">
	<table>
		<tr class="table1td" >
			<td style="width:250px;text-align:center">Value</td>
			<td><input type="button" value="Select all" onclick="selectDeselectAll(true)"></td>
			<td><input type="button" value="Unselect all" onclick="selectDeselectAll(false)"></td>
		</tr>
	</table>
    </th>
  </tr>
  %[modifier]
</table>

%[customtext]

<script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-3.2.1.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/svg-pan-zoom@3.5.1/dist/svg-pan-zoom.min.js"></script>
<script type = "text/javascript">
window.onload = function() {
	var s = document.getElementById("imageSaturation").getAttribute("values");
	var f = parseFloat(s) * 100;
	document.getElementById("saturationSlider").setAttribute("value", f);

	var panZoomTiger = svgPanZoom('#svgdisp',{controlIconsEnabled:true});
}

function setImageSaturationValue(newValue)
{
	var f = newValue / 100.0;
	document.getElementById("imageSaturation").setAttribute("values", f.toString());
}

function setImageTransparencyValue(newValue)
{
	var f = newValue / 100.0;
	var s = "1 0 0 0 0, 0 1 0 0 0, 0 0 1 0 0, 0 0 0 " + f.toString() + " 0";
	document.getElementById("imageColorMatrix").setAttribute("values", s);
}

function selectDeselectAll(b)
{
	$("input:checkbox.showhidecheckbox").prop('checked',b).trigger('change');
}

%[modifierHandlers]

</script>

</body>
</html> )literal";

CResultAsHtmlOutput::CResultAsHtmlOutput(const wchar_t* filename)
{
	this->a = this->b = this->c = this->d = this->e = this->f = std::numeric_limits<float>::quiet_NaN();
	_wfopen_s(&this->fp, filename, L"wb");
	/*std::wstring_convert<std::codecvt_utf16<wchar_t>> conv;
	this->filenameImage = conv.from_bytes(reinterpret_cast<const char*>(filename));*/
}

CResultAsHtmlOutput::~CResultAsHtmlOutput()
{
	fclose(this->fp);
}

void CResultAsHtmlOutput::AddCustomTextLine(const char* sz)
{
	this->AddCustomTextLine(s2ws(sz).c_str());
}

void CResultAsHtmlOutput::AddCustomTextLine(const wchar_t* sz)
{
	this->customText += sz;
	this->customText += L'\n';
}

void CResultAsHtmlOutput::Generate()
{
	std::string strSegments = this->GenerateSegmentsSvg();
	std::string strEllipses = this->GenerateEllipsesSvg();
	std::string strPoints = this->GeneratePointsSvg();
	std::string strLines = this->GenerateLinesSvg();
	std::string strPolygon = this->GeneratePolygonSvg();

	std::istringstream f(CResultAsHtmlOutput::szHTML);
	std::string line;
	while (std::getline(f, line))
	{
		std::wstring l = this->Process(line,
			[&](const char* szKey) -> std::wstring
		{
			if (strcmp(szKey, "beginimage") == 0)
			{
				if (this->filenameImage.empty())
					return std::wstring(L"<!--");
				return std::wstring();
			}
			else if (strcmp(szKey, "endimage") == 0)
			{
				if (this->filenameImage.empty())
					return std::wstring(L"-->");
				return std::wstring();
			}
			else if (strcmp(szKey, "imagefilename") == 0)
			{
				std::wstring str(L"file:///");
				str += this->filenameImage;
				return str;
			}
			else if (strcmp(szKey, "linesegmentssvg") == 0)
			{
				return s2ws(strSegments);
			}
			else if (strcmp(szKey, "ellipsessvg") == 0)
			{
				return s2ws(strEllipses);
			}
			else if (strcmp(szKey, "linessvg") == 0)
			{
				return s2ws(strLines);
			}
			else if (strcmp(szKey, "polygonsvg") == 0)
			{
				return s2ws(strPolygon);
			}
			else if (strcmp(szKey, "widthSvg") == 0)
			{
				return std::to_wstring(this->widthsvg > 0 ? this->widthsvg : 1456);
			}
			else if (strcmp(szKey, "heightSvg") == 0)
			{
				return std::to_wstring(this->heightsvg > 0 ? this->heightsvg : 2592);
			}
			else if (strcmp(szKey, "widthImage") == 0)
			{
				return std::to_wstring(this->widthImage > 0 ? this->widthImage : 1456);
			}
			else if (strcmp(szKey, "heightImage") == 0)
			{
				return std::to_wstring(this->heightImage > 0 ? this->heightImage : 2592);
			}
			else if (strcmp(szKey, "customtext") == 0)
			{
				if (this->customText.empty())
				{
					return this->customText;
				}

				std::wstring s;
				s = L"<p class=\"bordercustomtext\">\n";
				s += CResultAsHtmlOutput::AddLineBreaks(this->customText);
				s += L"</p>";
				return s;
			}
			else if (strcmp(szKey, "imageTransformMatrix") == 0)
			{
				if (std::isnan(this->a)) { return std::wstring(); }
				std::wostringstream s;
				s << std::endl << L"    transform=\"matrix(" << this->a << L"," << this->b << L"," << this->c << L"," << this->d << L"," << this->e << L"," << this->f << L")\"" << std::endl << L"    ";
				return s.str();
			}
			else if (strcmp(szKey, "pointssvg") == 0)
			{
				return s2ws(strPoints);
			}
			else if (strcmp(szKey, "modifier") == 0)
			{
				std::wostringstream s;
				for (auto i : this->hideShowModifierInfo)
				{
					s << L"<tr>" << std::endl <<
						L"<td class=\"table1td\">" << i.text.c_str() << "</td>" << std::endl <<
						L"<td class=\"table1td\"><input type=\"checkbox\" checked=\"checked\" class=\"showhidecheckbox\" onchange=\"handlerFunc" << i.className.c_str() << L"(this.checked)\"/></td>" << std::endl <<
						L"</tr>" << std::endl;
				}

				return s.str();
			}
			else if (strcmp(szKey, "modifierHandlers") == 0)
			{
				std::wostringstream s;
				for (auto i : this->hideShowModifierInfo)
				{
					s << L"function " << "handlerFunc" << i.className.c_str() << L"(newValue)" << std::endl <<
						L"{" << std::endl <<
						L"$('." << i.className.c_str() << "').css({\"display\":newValue?\"\":\"none\"});" << std::endl <<
						L"}" << std::endl;
				}

				return s.str();
			}
			else if (strcmp(szKey, "modifystyles") == 0)
			{
				std::wostringstream s;
				for (auto i : this->hideShowModifierInfo)
				{
					s << L". " <<  i.className.c_str() << L" {" << std::endl <<
						L"display:" << std::endl <<
						L"}" << std::endl;
				}

				return s.str();
			}

			return std::wstring(L"XXX");
		});

		this->WriteUTF16(l.c_str());
	}
}

/*static*/std::wstring CResultAsHtmlOutput::AddLineBreaks(std::wstring& str)
{
	std::wregex rx(L"\n");
	return std::regex_replace(str, rx, L"<br/>");
}

void CResultAsHtmlOutput::WriteUTF16(const wchar_t* str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	auto s = utf8_conv.to_bytes(str);
	fwrite(s.c_str(), 1, s.size(), this->fp);
	fputs("\n", this->fp);
}

std::string CResultAsHtmlOutput::GenerateSegmentsSvg()
{
	std::ostringstream s;
	this->GenerateSegmentsSvg(s);
	return s.str();
}

void CResultAsHtmlOutput::GenerateSegmentsSvg(std::ostream& stream)
{
	if (!this->getSegments)
	{
		return;
	}

	CResultAsHtmlOutput::Attributes attribs;
	for (int i = 0;; ++i)
	{
		//LSDLineSegment item;
		//float x0, y0, x1, y1;
		//float width = 1;
		attribs.Clear();
		SegmentData sd;
		sd.SetDefault();
		//std::string color;
		if (!this->getSegments(i, sd, attribs))
		{
			break;
		}

		if (attribs.color.empty())
		{
			attribs.color = "black";
		}

		stream << "<line x1 = \"" << sd.x0 << "\" y1=\"" << sd.y0 << "\" x2=\"" << sd.x1 << "\" y2=\"" << sd.y1 << "\" "
			<< "stroke-width=\"" << sd.width << "\" stroke=\"" << attribs.color << "\"";
		if (!attribs.className.empty())
		{
			stream << " class=\"" << attribs.className << "\"";
		}

		stream << "/>" << std::endl;
	}
}

std::string CResultAsHtmlOutput::GenerateEllipsesSvg()
{
	std::ostringstream s;
	this->GenerateEllipsesSvg(s);
	return s.str();
}

void CResultAsHtmlOutput::GenerateEllipsesSvg(std::ostream& stream)
{
	if (!this->getEllipse)
	{
		return;
	}

	for (int i = 0;; ++i)
	{
		std::string color;
		EllipseOptions options;
		EllipseParams ellParams;
		if (!this->getEllipse(i, ellParams, options))
		{
			break;
		}

		if (options.GetColor().empty())
		{
			color = "purple";
		}
		else
		{
			color = options.GetColor();
		}

		stream << "<g transform = \"translate(" << ellParams.x0 << " " << ellParams.y0 << ") rotate(" << (ellParams.theta / 3.141592653589793238463) * 180 << ")\">" << std::endl
			<< "<ellipse cx=\"0\" cy=\"0\" rx=\"" << ellParams.a << "\" ry=\"" << ellParams.b << "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"1\" />" << std::endl;
		if (options.GetDrawLineMajorAxis() == true)
		{
			stream << "<line x1=\"" << -ellParams.a << "\" y1=\"0\" x2=\"" << ellParams.a << "\" y2=\"0\" stroke-width=\"2\" stroke=\"purple\" />" << std::endl;
		}

		if (options.GetDrawLineMinorAxis() == true)
		{
			stream << "<line x1=\"0\" y1=\"" << -ellParams.b << "\" x2=\"0\" y2=\"" << ellParams.b << "\" stroke-width=\"2\" stroke=\"DarkGoldenRod\" />" << std::endl;
		}

		stream << "</g>" << std::endl;
	}
}

std::string CResultAsHtmlOutput::GeneratePointsSvg()
{
	std::ostringstream s;
	this->GeneratePointsSvg(s);
	return s.str();
}

std::string CResultAsHtmlOutput::GenerateLinesSvg()
{
	std::ostringstream s;
	this->GenerateLinesSvg(s);
	return s.str();
}

std::string CResultAsHtmlOutput::GeneratePolygonSvg()
{
	std::ostringstream s;
	this->GeneratePolygonSvg(s);
	return s.str();
}

void CResultAsHtmlOutput::GenerateLinesSvg(std::ostream& stream)
{
	if (!this->getLine)
	{
		return;
	}

	stream << "<!-- BEGIN LINES -->" << std::endl;
	const float LineWidth = 1;

	Attributes attribs;

	for (int i = 0;; ++i)
	{
		LineData ld;
		attribs.Clear();
		if (!this->getLine(i, ld, attribs))
		{
			break;
		}
		//float x0, y0, x1, y1; std::string strColor;
		/*if (!this->getLine(i, x0, y0, x1, y1, strColor))
		{
			break;
		}*/

		if (attribs.color.empty())
		{
			attribs.color = "red";
		}

		stream << "<line x1=\"" << ld.x0 << "\" y1=\"" << ld.y0 << "\" x2=\"" << ld.x1 << "\" y2=\"" << ld.y1 << "\" stroke-width=\"" << LineWidth << "\" stroke=\"" << attribs.color << "\"";

		if (!attribs.className.empty())
		{
			stream << " class=\"" << attribs.className << "\"";
		}

		stream << "/>" << std::endl;
	}

	stream << "<!-- END LINES -->" << std::endl;
}

void CResultAsHtmlOutput::GeneratePolygonSvg(std::ostream& stream)
{
	if (!this->getPolygonPoints) { return; }

	stream << "<!-- BEGIN POLYGON -->" << std::endl;
	stream << "<polyline points = \"";
	for (int i = 0;; ++i)
	{
		float x, y;
		if (!this->getPolygonPoints(i, x, y)) { break; }
		stream << " " << x << "," << y;
	}

	stream << "\" style=\"fill:none;stroke:black;stroke-width:3\" />" << std::endl;

	stream << "<!-- END POLYGON -->" << std::endl;
}

void CResultAsHtmlOutput::GeneratePointsSvg(std::ostream& stream)
{
	if (!this->getPoint)
	{
		return;
	}

	stream << "<!-- BEGIN POINTS -->" << std::endl;
	const float WidthHeight = 8;
	const float LineWidth = 2;

	for (int i = 0;; ++i)
	{
		float x, y; std::string strColor;
		if (!this->getPoint(i, x, y, strColor))
		{
			break;
		}

		if (strColor.empty())
		{
			strColor = "red";
		}

		stream << "<!-- " << x << ", " << y << " -->" << std::endl;
		stream << "<line x1=\"" << x - WidthHeight << "\" y1=\"" << y - WidthHeight << "\" x2=\"" << x + WidthHeight << "\" y2=\"" << y + WidthHeight << "\" stroke-width=\"" << LineWidth << "\" stroke=\"" << strColor << "\"/>" << std::endl;
		stream << "<line x1=\"" << x + WidthHeight << "\" y1=\"" << y - WidthHeight << "\" x2=\"" << x - WidthHeight << "\" y2=\"" << y + WidthHeight << "\" stroke-width=\"" << LineWidth << "\" stroke=\"" << strColor << "\"/>" << std::endl;
	}

	stream << "<!-- END POINTS -->" << std::endl;
}

std::wstring CResultAsHtmlOutput::Process(const std::string& str, std::function<std::wstring(const char*)> getReplacement)
{
	std::regex regex("%\\[[[:alnum:]]*\\]", std::regex_constants::ECMAScript);

	auto words_begin = std::sregex_iterator(str.begin(), str.end(), regex);
	auto words_end = std::sregex_iterator{};
	if (words_begin == words_end)
	{
		std::wstring ws = s2ws(str);
		return ws;
	}

	std::wstring wstr;
	int lastPos = 0;
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		int position = i->position();
		std::string match_str = match.str().substr(2, match.str().size() - 3);
		wstr += s2ws(str.substr(lastPos, position - lastPos));
		wstr += getReplacement(match_str.c_str());
		lastPos = position + match.str().size();
	}

	wstr += s2ws(str.substr(lastPos, str.size() - lastPos));

	return wstr;
}