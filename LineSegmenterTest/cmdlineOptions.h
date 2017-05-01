#pragma once

class CCmdlineOptions
{
private:
	std::string sourceFilename;
	std::string outputFolder;
	std::string outputFilenamePrefix;
public:
	CCmdlineOptions();

	int Parse(char** argv, int argc);


	const std::string& GetSourceFilename() const { return this->sourceFilename; }
	const std::string& GetOutputFolder() const{ return this->outputFolder; }
	const std::string& GetOutputFilenamePrefix() const { return this->outputFilenamePrefix; }
};
