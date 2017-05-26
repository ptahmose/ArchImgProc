#pragma once
#include <vector>
#include <functional>

class CChainFuncs
{
private:
	std::vector<std::function<bool(int, int, float& x1, float& y1, float& x2, float& y2, float& width, std::string& color)>> funcs;
	int curFunc;
	int offsetCnt;
public:
	CChainFuncs() :curFunc(0), offsetCnt(0) {};
	void AddFunc(std::function<bool(int, int, float& x1, float& y1, float& x2, float& y2, float& width, std::string& color)> func)
	{
		this->funcs.push_back(func);
	}

	bool Func(int c, float& x1, float& y1, float& x2, float& y2, float& width, std::string& color)
	{
		if (this->curFunc >= this->funcs.size())
		{
			return false;
		}

		bool b = this->funcs[this->curFunc](c, c - this->offsetCnt, x1, y1, x2, y2, width, color);
		if (b == false)
		{
			this->curFunc++;
			this->offsetCnt = c;
			return this->Func(c, x1, y1, x2, y2, width, color);
		}

		return true;
	}
};
