#pragma once

class CCmdlineOptions
{
private:
	std::string sourceFilename;
public:
	CCmdlineOptions();

	int Parse(char** argv, int argc);


	const std::string& GetSourceFilename() const { return this->sourceFilename; }
};
