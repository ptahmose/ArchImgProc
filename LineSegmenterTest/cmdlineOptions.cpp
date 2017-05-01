#include "stdafx.h"
#include "cmdlineOptions.h"
#include <dimcli/cli.h>
#include <iostream>


CCmdlineOptions::CCmdlineOptions()
{
	
}


int CCmdlineOptions::Parse(char** argv, int argc)
{
	Dim::Cli cli;
	auto & srcFilename = cli.opt<std::string>("s source").desc("source filename");
	auto & outputFolder = cli.opt<std::string>("o output-folder").desc("output folder");
	auto & prefixOutput = cli.opt<std::string>("p output-prefix").desc("output file prefix");

	bool b = cli.parse(std::cerr, argc, argv);
	if (!b)
	{
		return cli.exitCode();
	}


	this->sourceFilename = *srcFilename;
	this->outputFolder = *outputFolder;
	this->outputFilenamePrefix = *prefixOutput;
	return 0;
}