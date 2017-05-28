#pragma once
#include <vector>
#include <functional>

class CChainFuncs
{
private:
	std::vector<std::function<bool(int, int, CResultAsHtmlOutput::SegmentData& segmentData, CResultAsHtmlOutput::Attributes& attribs)>> funcs;
	int curFunc;
	int offsetCnt;
public:
	CChainFuncs() :curFunc(0), offsetCnt(0) {};
	void AddFunc(std::function<bool(int, int, CResultAsHtmlOutput::SegmentData& segmentData, CResultAsHtmlOutput::Attributes& attribs)> func)
	{
		this->funcs.push_back(func);
	}

	bool Func(int c, CResultAsHtmlOutput::SegmentData& segmentData, CResultAsHtmlOutput::Attributes& attribs)
	{
		if (this->curFunc >= this->funcs.size())
		{
			return false;
		}

		bool b = this->funcs[this->curFunc](c, c - this->offsetCnt, segmentData, attribs);
		if (b == false)
		{
			this->curFunc++;
			this->offsetCnt = c;
			return this->Func(c, segmentData, attribs);
		}

		return true;
	}
};
